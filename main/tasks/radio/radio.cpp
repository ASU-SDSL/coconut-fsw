#include <RadioLib.h>
#include "radio.h"
#include <FreeRTOS.h>
#include "PicoHal.h"
#include <stdlib.h>
#include "log.h"
#include "gse.h"
#include "spacepacket.h"
#include "command.h"

#define ERR_NONE 0
#define NULL_QUEUE_WAIT_TIME 100

#define RADIO_LOGGING 0
#define RADIO_LOGGING_CAD 0

/**
 * one thread
 * queue polling to check on the size of queue each iteration
 * radio recieve polling
 * On recieve:
 * Send an interrupt when packet recieve https://jgromes.github.io/RadioLib/class_s_x127x.html#ad63322c9c58dd82e4b9982f10e546f33
 * In interupt set a flag -- set the flag to be volitile
 * send to command.c to parse
 * On send:
 * poll the queue
 * if smt in queue send on radio
 */
#define RADIO_STATE_NO_ATTEMPT 1 
#define RADIO_ERROR_CUSTOM 2
#define RADIO_RECEIVE_TIMEOUT_MS 1000
#define RADIO_TRANSMIT_TIMEOUT_MS 10000
#define RADIO_NO_CONTACT_PANIC_TIME_MS (1000UL * 60 * 60 * 24 * 7) //  7 days in ms

PicoHal *picoHal = new PicoHal(spi0, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_SCK_PIN);
// Add interupt pin
RFM98 radioRFM = new Module(picoHal, RADIO_RFM_NSS_PIN, RADIO_RFM_DIO0_PIN, RADIO_RFM_NRST_PIN, RADIO_RFM_DIO1_PIN); //RADIOLIB_NC); // RFM98 is an alias for SX1278
SX1268 radioSX = new Module(picoHal, RADIO_SX_NSS_PIN, RADIO_SX_DIO1_PIN, RADIO_SX_NRST_PIN, RADIO_SX_BUSY_PIN); 
PhysicalLayer* radio = &radioSX;

int radio_state_RFM = RADIO_STATE_NO_ATTEMPT; 
int radio_state_SX = RADIO_STATE_NO_ATTEMPT;

uint8_t radio_transmit_power = 2;

#ifdef __cplusplus
extern "C"
{
#endif
    void radio_task(void *unused_arg){
        radio_task_cpp();
    }
    void radio_queue_message(char *buffer, size_t size)
    {
        // Create new transmission structure
        // telemetry_queue_transmission_t new_buffer;
        radio_queue_operations_t new_buffer; 
        new_buffer.operation_type = radio_operation_type_t::TRANSMIT;
        new_buffer.data_size = size;
        // Allocate chunk on heap to copy buffer contents
        // auto heap_buf = static_cast<char *>(pvPortMalloc(size));
        auto heap_buf = static_cast<uint8_t *>(pvPortMalloc(size));
        memcpy(heap_buf, buffer, size);
        new_buffer.data_buffer = heap_buf;
        // Wait for queue to become available
        if (radio_queue) {
            xQueueSendToBack(radio_queue, &new_buffer, portMAX_DELAY);
        }
    }
    void radio_set_transmit_power(uint8_t output_power)
    {
        radio_queue_operations_t new_buffer;
        new_buffer.operation_type = radio_operation_type_t::SET_OUTPUT_POWER;
        new_buffer.data_size = 1; 
        auto heap_buf = static_cast<uint8_t *>(pvPortMalloc(1));
        memcpy(heap_buf, &output_power, 1); 
        new_buffer.data_buffer = heap_buf; 
        while(!radio_queue)
        {
            vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS); 
        }
        xQueueSendToBack(radio_queue, &new_buffer, portMAX_DELAY); 
    }
    void radio_set_module(radio_operation_type_t op)
    {
        if(op == radio_operation_type_t::SET_OUTPUT_POWER || op == radio_operation_type_t::TRANSMIT){
            return;
        }

        radio_queue_operations_t new_buffer;
        if(op == radio_operation_type_t::ENABLE_RFM98){ 
            new_buffer.operation_type = radio_operation_type_t::ENABLE_RFM98;
        }
        else{
            new_buffer.operation_type = radio_operation_type_t::ENABLE_SX1268;
        }

        while(!radio_queue)
        {
            vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS); 
        }
        xQueueSendToBack(radio_queue, &new_buffer, portMAX_DELAY); 
    }
    uint8_t radio_which(){
        return (radio == &radioRFM); 
    }
    uint16_t radio_get_RFM_state(){
        return radio_state_RFM; 
    }
    uint16_t radio_get_SX_state(){
        return radio_state_SX; 
    }
#ifdef __cplusplus
}
#endif

// isrs
volatile bool operation_done_RFM = false;
volatile bool cad_detected_RFM = false; 
volatile bool general_flag_SX = false; 

void radio_operation_done_RFM()
{
    operation_done_RFM = true;
}

void radio_cad_detected_RFM(){
    cad_detected_RFM = true; 
}

void radio_general_flag_SX(){
    general_flag_SX = true; 
}
// end isrs

// toggle the radio until one of them works
void radio_panic(){
    #if RADIO_LOGGING
    printf("Critical Radio fail, panicking...\n");
    #endif

    do {
        if(radio == &radioSX){
            radio_state_SX = RADIO_ERROR_CUSTOM; 
            #if RADIO_LOGGING
            printf("Attempting to switch to RFM98...\n");
            #endif
            // try to clean up if possible
            radioSX.clearDio1Action(); 

            // initialize RFM98
            radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
            #if RADIO_LOGGING
            printf("Res: %d\n", radio_state_RFM); 
            #endif
            radio = &radioRFM;
            if(radio_state_RFM == 0){
                radioRFM.standby(); 
                radioRFM.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
                radioRFM.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE); 
                // final check 
                if(radio->startChannelScan() == 0) break; // avoid sleep 
                else radio_state_RFM = RADIO_ERROR_CUSTOM; 
            }
            #if RADIO_LOGGING
            else {
                printf("Switch to RFM failed with code: %d\n", radio_state_RFM);
            }
            #endif
        } else {
            radio_state_RFM = RADIO_ERROR_CUSTOM; 
            #if RADIO_LOGGING
            printf("Attempting to switch to SX1268...\n");
            #endif
            // try to clean up if possible 
            radioRFM.clearDio0Action();
            radioRFM.clearDio1Action(); 

            // initialize SX1268
            radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
            #if RADIO_LOGGING
            printf("Res: %d\n", radio_state_SX); 
            #endif
            radio = &radioSX; 
            if(radio_state_SX == 0){
                radioSX.standby(); 
                radioSX.setDio1Action(radio_general_flag_SX);
                // final check 
                if(radio->startChannelScan() == 0) break; // avoid sleep 
                else radio_state_SX = RADIO_ERROR_CUSTOM; 
            } 
            #if RADIO_LOGGING
            else {
                printf("Switch to SX failed with code: %d\n", radio_state_SX);
            }
            #endif
        }
        printf("SX: %d RFM: %d\n", radio_state_SX, radio_state_RFM); 
        sleep_ms(15000); // wait 15 seconds between tries
    } while(radio_state_RFM != 0 && radio_state_SX != 0); 

    #if RADIO_LOGGING
    printf("Final SX: %d RFM: %d\n", radio_state_SX, radio_state_RFM); 
    if(radio == &radioSX) printf("Resolved to SX1268.\n");
    else printf("Resolved to RFM98.\n");
    #endif
}

void init_radio()
{
    sleep_ms(1000); // for debugging

    // If the RFM is physically wired into the board it needs to call begin() before the SX1268
    // my current theory as to why is that it before begin() it is polluting the SPI line
    radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);

    if(radio_state_RFM == 0){
        radioRFM.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
        radioRFM.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE); 
        radio = &radioRFM; 
        radioSX.sleep(); 
    }
    else {
        radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
        if(radio_state_SX == 0){
            radioSX.setDio1Action(radio_general_flag_SX);
            radio = &radioSX;
            radioRFM.sleep(); 
        }
        else {
            #if RADIO_LOGGING
            printf("Errors RFM: %d SX: %d\n", radio_state_RFM, radio_state_SX);
            #endif 
            radio_panic(); 
        }
    }
    #if RADIO_LOGGING
    printf("States --> RFM: %d SX: %d\n", radio_state_RFM, radio_state_SX);
    #endif

    int channel_scan_state = radio->startChannelScan();

    if (channel_scan_state == RADIOLIB_ERR_NONE)
    {
        #if RADIO_LOGGING
        printf("Success, scanning...\n");
        #endif
    }
    else
    {
        // to do fix
        #if RADIO_LOGGING
        printf("receive setup failed with code %d\n", channel_scan_state);
        sleep_ms(1000); 
        #endif
        radio_panic(); 
    }
}

void set_power_output(PhysicalLayer* radio_module, int8_t new_dbm){
    radio_transmit_power = new_dbm;
    radio_module->setOutputPower(new_dbm); 
}

int parse_num(uint8_t * packet, size_t packet_size){
    int num = 0; 
    for(int i = 0; i < packet_size; i++){
        if(packet[i] >= '0' && packet[i] <= '9'){
            num = (num * 10) + (packet[i] - '0');
        }
    }

    return num; 
}

/**
 * 0 for stat echo
 * 1 for "ECHO" echo
 * 2 for switch to RFM98
 * 3 for switch to SX1268
 */
void parseRebound(PhysicalLayer* radio, uint8_t* packet, uint8_t packet_size){
    int code = -1;
    
    code = parse_num(packet, packet_size);

    sleep_ms(1000); 

    switch(code){
        case 0:{
            #if RADIO_LOGGING
            printf("Returning stats\n"); 
            #endif
            float rssi = radio->getRSSI();
            float snr = radio->getSNR(); 
            float fe = 0;
            if(radio == &radioSX){
                fe = radioSX.getFrequencyError();
            } 
            else {
                fe = radioRFM.getFrequencyError(); 
            }

            // 6 (header) + 64 (msg);
            char buff[6+64];
            buff[0] = 'A';
            buff[1] = 'A';
            buff[2] = 'A';
            buff[3] = 'A';
            buff[4] = 'A';
            buff[5] = 64; // packet size

            sprintf(buff+6, "%.2fdBm %.2fdB %.2fHz", rssi, snr, fe);
            radio_queue_message(buff, sizeof(buff)); 

            break; 
        }
        case 1:{
            char msg[] = {'E', 'C', 'H', 'O', '!'};
            radio_queue_message(msg, 5);
            break; 
        }
        case 2:
            #if RADIO_LOGGING
                printf("attempting to switch to RFM98\n"); 
            #endif
            radio_set_module(radio_operation_type_t::ENABLE_RFM98);
            break;
        case 3:
            #if RADIO_LOGGING 
                printf("attempting to switch to SX1268\n"); 
            #endif 
            radio_set_module(radio_operation_type_t::ENABLE_SX1268);
            break;
        
    }    
}



/**
 * Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task_cpp(){
    #if RADIO_LOGGING
    printf("Starting Radio Task\n"); 
    #endif
    init_radio(); 

    radio_queue = xQueueCreate(RADIO_MAX_QUEUE_ITEMS, sizeof(radio_queue_operations_t));
    radio_queue_operations_t rec; 
    
    bool receiving = false; 
    bool transmitting = false; 
    int state = 0; 
    uint32_t operation_start_time = to_ms_since_boot(get_absolute_time());
    uint32_t last_receive_time = to_ms_since_boot(get_absolute_time());
    int transmission_size = 0;  

    while(true){
        // save now time since boot 
        uint32_t radio_now = to_ms_since_boot(get_absolute_time());
        // if there's been no contect for a long time, try to switch radios 
        if(abs((long long)(radio_now - last_receive_time)) > RADIO_NO_CONTACT_PANIC_TIME_MS){
            radio_panic(); 
        }

        // check operation duration to avoid hanging in an operation mode 
        if(receiving && abs((long long)(radio_now - operation_start_time)) > RADIO_RECEIVE_TIMEOUT_MS){
            #if RADIO_LOGGING
            printf("receive timeout\n"); 
            #endif
            receiving = false;
            radio->startChannelScan();
        }
        else if(transmitting && abs((long long)(radio_now - operation_start_time)) > RADIO_TRANSMIT_TIMEOUT_MS){
            #if RADIO_LOGGING
            printf("transmit timeout\n"); 
            #endif
            transmitting = false;
            radio->startChannelScan();
        }

        // handle interrupt flags 
        if(cad_detected_RFM || operation_done_RFM || general_flag_SX){
            // handle finished transmission
            if(transmitting){
                transmitting = false; 
                #if RADIO_LOGGING
                float duration_s = ((double)radio_now - operation_start_time) / 1000.0; 
                printf("Duration (s): %f\n", duration_s);
                printf("Bit rate: %f\n", transmission_size / duration_s);
                printf("Done.\n"); 
                sleep_ms(200); 
                #endif 
            }

            // handle finished receive 
            else if(receiving){
                
                size_t packet_size = radio->getPacketLength();
                uint8_t packet[packet_size];

                int packet_state = radio->readData(packet, packet_size);

                if (packet_state == RADIOLIB_ERR_NONE)
                {
                    last_receive_time = to_ms_since_boot(get_absolute_time()); 
                    // parse out sync bytes and grab packet with header
                    // create command.c function to read packet
                    #if RADIO_LOGGING
                    printf("Received packet: ");
                    for(int i = 0; i < packet_size; i++){
                        printf("%c", packet[i]);
                    }
                    printf("\n"); 
                    #endif
                    receive_command_bytes(packet, packet_size);
                    // Check if command is to set output power of the radio

                    // parseRebound(radio, packet, packet_size); 
                }
                #if RADIO_LOGGING
                else if (packet_state == RADIOLIB_ERR_CRC_MISMATCH)
                {
                    printf("CRC Error!!\n");
                }
                else
                {
                    printf("Packet Reading failed\n");
                }
                #endif

                // clear receiving flag 
                receiving = false; 
            }
            // handle other interrupts (CAD done)
            // this is the main difference between the 2 modules 
            else {
                if(radio == &radioRFM && cad_detected_RFM){
                    #if RADIO_LOGGING_CAD
                    printf("CAD Detected, starting receive... "); 
                    #endif
                    state = radio->startReceive(); 
                    operation_start_time = to_ms_since_boot(get_absolute_time()); 
                    #if RADIO_LOGGING_CAD
                    if(state == 0){
                        printf("success\n"); 
                    }
                    else{
                        printf("receive failed with code: %d\n", state); 
                    }
                    #endif

                    receiving = true;
                }
                else if(radio == &radioSX) { // radio is radioSX 
                    state = radioSX.getChannelScanResult(); // not a PhysicalLayer function

                    if(state == RADIOLIB_LORA_DETECTED){
                        #if RADIO_LOGGING_CAD
                        printf("CAD Detected, starting receive... ");
                        #endif
                        state = radio->startReceive();
                        #if RADIO_LOGGING_CAD
                        if(state == 0){
                            printf("success\n");
                        }
                        else {
                            printf("SX start receive failed with code: %d\n", state);
                        }
                        #endif
                        operation_start_time = to_ms_since_boot(get_absolute_time());
                        receiving = true; 
                    }
                    #if RADIO_LOGGING
                    else if (state == RADIOLIB_CHANNEL_FREE) {
                        //printf("channel free\n"); 
                    }
                    else {
                        printf("SX scan failed with code: %d\n", state); 
                    }
                    #endif
                }
            }

            if(!receiving && !transmitting) {
                state = radio->startChannelScan(); 
            }

            // clear flags 
            general_flag_SX = false;
            cad_detected_RFM = false;
            operation_done_RFM = false; 
        }

        if(!transmitting && !receiving && xQueueReceive(radio_queue, &rec, 0))
        {
            switch (rec.operation_type) {
                case TRANSMIT:
                    #if RADIO_LOGGING
                    {
                    char message[rec.data_size+1];
                    for(int i = 0; i < rec.data_size; i++){
                        if(rec.data_buffer[i] == '\0') message[i] = '|';
                        else message[i] = rec.data_buffer[i]; 
                    }
                    message[rec.data_size] = '\0';
                    printf("transmitting %s...\n", message);
                    }
                    #endif
                    transmission_size = rec.data_size; 
                    operation_start_time = to_ms_since_boot(get_absolute_time()); 
                    state = radio->startTransmit(rec.data_buffer, rec.data_size);
                    #if RADIO_LOGGING
                    while(state != 0){
                        printf("Transmit failed with code: %d\n", state); 
                        radio_panic(); 
                        state = radio->startTransmit(rec.data_buffer, rec.data_size);
                    }
                    #endif
                    transmitting = true;
                    vPortFree(rec.data_buffer);
                    break;

                case SET_OUTPUT_POWER:
                    set_power_output(radio, rec.data_buffer[0]);
                    vPortFree(rec.data_buffer);
                    break;
                
                case ENABLE_RFM98:
                    if(radio == &radioRFM) break;
                    #if RADIO_LOGGING
                    printf("attempting to swap to RFM98...\n");
                    #endif
                    radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
                    if(radio_state_RFM == 0){
                        radioSX.clearDio1Action(); 
                        radioSX.sleep(); 
                        radio = &radioRFM;
                        radioRFM.standby(); 
                        radioRFM.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
                        radioRFM.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE); 
                        radio->startChannelScan(); 
                        #if RADIO_LOGGING
                        printf("success\n"); 
                        #endif
                        // vPortFree(rec.data_buffer); 
                    }
                    else {
                        #if RADIO_LOGGING
                        printf("switch to RFM failed with code: %d\n", radio_state_RFM); 
                        #endif
                        radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
                        if(radio_state_SX != 0){
                            #if RADIO_LOGGING
                            printf("switch back to SX1268 failed with code: %d\n", radio_state_SX);
                            #endif
                            radio_panic(); 
                        }
                        else {
                            radio->startChannelScan(); 
                        }
                        // transmit error code?
                    }
                    
                    break;
                
                case ENABLE_SX1268:
                    if(radio == &radioSX) break;
                    #if RADIO_LOGGING
                    printf("attempting swap to SX1268...\n"); 
                    #endif
                    radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
                    if(radio_state_SX == 0){
                        radioRFM.clearDio0Action();
                        radioRFM.clearDio1Action(); 
                        radioRFM.sleep(); 
                        radio = &radioSX; 
                        radioSX.standby(); 
                        radioSX.setDio1Action(radio_general_flag_SX);
                        radio->startChannelScan(); 
                        #if RADIO_LOGGING
                        printf("success\n"); 
                        #endif
                        // vPortFree(rec.data_buffer); 
                    }
                    else{
                        #if RADIO_LOGGING
                        printf("switch to SX failed with code: %d\n", radio_state_SX); 
                        #endif
                        radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
                        if(radio_state_RFM != 0){
                            #if RADIO_LOGGING
                            printf("switch back failed with code: %d\n", radio_state_RFM);
                            #endif
                            radio_panic(); 
                        }
                        else {
                            radio->startChannelScan(); 
                        }
                    }

                    break;
            }
        }
    }

}
