#include <RadioLib.h>
#include "radio.h"
#include <FreeRTOS.h>
#include "command.h"
#include "PicoHal.h"
#include <stdlib.h>

#define ERR_NONE 0
#define NULL_QUEUE_WAIT_TIME 100

#define RADIO_LOGGING 1
#define RADIO_LOGGING_CAD 1 

// USER FUNCTIONS

// void set_power_output_request(int new_db) {

//     radio_queue_operations_t new_operation;
//     new_operation.operation_type = SET_OUTPUT_POWER;
//     new_operation.data_buffer = (uint8_t*)pvPortMalloc(sizeof(int));
//     new_operation.data_size = sizeof(int);
//     memcpy(new_operation.data_buffer, &new_db, sizeof(int)); // does this work?

//     while(radio_queue == NULL) {
//         vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS);
//     }
//     xQueueSendToBack(radio_queue, &new_operation, portMAX_DELAY); 
// }

// void transmit_request(char* buffer, size_t size) {

//     radio_queue_operations_t new_operation;
    
//     new_operation.operation_type = TRANSMIT;
//     new_operation.data_size = size;
//     // Allocate chunk on heap to copy buffer contents
//     uint8_t* heap_buf = (uint8_t*)pvPortMalloc(size);
//     memcpy(heap_buf, buffer, size);
//     new_operation.data_buffer = heap_buf;
    
//     // Wait for queue to become available
//     while (radio_queue == NULL) {
//         vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS);
//     }
//     // xQueueSendToBack(uart0_queue, &new_buffer, portMAX_DELAY);
//     xQueueSendToBack(radio_queue, &new_operation, portMAX_DELAY);
// }



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
#define RADIO_RECEIVE_TIMEOUT_MS 1000

PicoHal *picoHal = new PicoHal(spi0, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_SCK_PIN);
// Add interupt pin
RFM98 radioRFM = new Module(picoHal, RADIO_RFM_NSS_PIN, RADIO_RFM_DIO0_PIN, RADIO_RFM_NRST_PIN, RADIO_RFM_DIO1_PIN); //RADIOLIB_NC); // RFM98 is an alias for SX1278
SX1268 radioSX = new Module(picoHal, RADIO_SX_NSS_PIN, RADIO_SX_DIO1_PIN, RADIO_SX_NRST_PIN, RADIO_SX_BUSY_PIN); 
PhysicalLayer* radio = &radioSX;
int radio_state_RFM = RADIO_STATE_NO_ATTEMPT; 
int radio_state_SX = RADIO_STATE_NO_ATTEMPT; 

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
        new_buffer.operation_type = operation_type_t::TRANSMIT;
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
        new_buffer.operation_type = operation_type_t::SET_OUTPUT_POWER;
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
    void radio_set_module(operation_type_t op)
    {
        if(op == operation_type_t::SET_OUTPUT_POWER || op == operation_type_t::TRANSMIT){
            return;
        }

        radio_queue_operations_t new_buffer;
        if(op == operation_type_t::ENABLE_RFM98){ 
            new_buffer.operation_type = operation_type_t::ENABLE_RFM98;
        }
        else{
            new_buffer.operation_type = operation_type_t::ENABLE_SX1268;
        }

        while(!radio_queue)
        {
            vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS); 
        }
        xQueueSendToBack(radio_queue, &new_buffer, portMAX_DELAY); 
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

void init_radio()
{
    sleep_ms(1000); // for debugging

    // If the RFM is physically wired into the board it needs to call begin() before the SX1268
    // my current theory as to why is that it before begin() it is polluting the SPI line
    radio_state_RFM = radioRFM.begin();  

    if(radio_state_RFM == 0){
        radioRFM.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
        radioRFM.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE); 
        radio = &radioRFM; 
        radioSX.sleep(); 
    }
    else {
        radio_state_SX = radioSX.begin(434.0, 125.0, 9, 7, 18, 2, 8, 0.0, false);
        if(radio_state_SX == 0){
            radioSX.setDio1Action(radio_general_flag_SX);
            radio = &radioSX;
            radioRFM.sleep(); 
        }
        else {
            #if RADIO_LOGGING
            printf("Errors RFM: %d SX: %d\n", radio_state_RFM, radio_state_SX);
            #endif 
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
        while (true){
            #if RADIO_LOGGING
            printf("receive setup failed with code %d\n", channel_scan_state);
            #endif
        }
    }
}

void set_power_output(PhysicalLayer* radio_module, int8_t new_dbm){
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

int parse_ccsds_num(uint8_t* packet, uint8_t packet_size){
    packet += sizeof(ccsds_header_t);

    return parse_num(packet, packet_size - sizeof(ccsds_header_t));
}

/**
 * 0 for stat echo
 * 1 for "ECHO" echo
 * 2 for switch to RFM98
 * 3 for switch to SX1268
 */
void parseRebound(PhysicalLayer* radio, uint8_t* packet, uint8_t packet_size){
    int code = -1;
    
    if(packet_size > sizeof(ccsds_header_t)){
        code = parse_ccsds_num(packet, packet_size); 
    }
    else {
        code = parse_num(packet, packet_size);
    }
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
            radio_set_module(operation_type_t::ENABLE_RFM98);
            break;
        case 3:
            radio_set_module(operation_type_t::ENABLE_SX1268);
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
    unsigned long receive_start_time = to_ms_since_boot(get_absolute_time());

    while(true){
        // stop radio from getting stuck in receive mode
        if(receiving && abs((long long)(to_ms_since_boot(get_absolute_time()) - receive_start_time)) > RADIO_RECEIVE_TIMEOUT_MS){
            #if RADIO_LOGGING
            printf("receive timeout\n"); 
            #endif
            receiving = false;
            radio->startChannelScan();
        }

        // receiving split into 2 different if statements, they can probably be combined 
        // but for now, split is better for testing

        // receiving with RFM98
        if(radio == &radioRFM && (cad_detected_RFM || operation_done_RFM)){
            if(transmitting){ // radio was transmitting (interrupt signals finish)
                transmitting = false; 
                #if RADIO_LOGGING
                printf("Done.\n");
                #endif
            }
            if(receiving){ // radio was receiving (interrupt signals finish)
                cad_detected_RFM = false;
                operation_done_RFM = false; 
                
                size_t packet_size = radio->getPacketLength();
                uint8_t packet[packet_size];

                int packet_state = radio->readData(packet, packet_size);

                if (packet_state == RADIOLIB_ERR_NONE)
                {
                    // parse out sync bytes and grab packet with header
                    // create command.c function to read packet
                    #if RADIO_LOGGING
                    printf("Received packet: ");
                    for(int i = 0; i < packet_size; i++){
                        printf("%c", packet[i]);
                    }
                    printf("\n"); 
                    #endif

                    parseRebound(radio, packet, packet_size);                     
                    
                    //parse_radio_packet(packet, packet_size);
                    // Check if command is to set output power of the radio
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

                receiving = false; 
            }

            if(!receiving && !transmitting && cad_detected_RFM) { // radio was scanning (interrupt signals CAD detected)
                #if RADIO_LOGGING_CAD
                printf("CAD Detected, starting receive... "); 
                #endif
                state = radio->startReceive(); 
                receive_start_time = to_ms_since_boot(get_absolute_time()); 
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
            
            // restart channelScan - this definitely feels like it should be a continuous mode, but it's not
            if(!receiving && !transmitting){
                state = radio->startChannelScan();
            }

            // reset flags
            cad_detected_RFM = false; 
            operation_done_RFM = false; 
        }

        // receiving with SX1268
        if(radio == &radioSX && general_flag_SX){
            general_flag_SX = false;

            if(transmitting){
                transmitting = false; 
                #if RADIO_LOGGING
                printf("Done.\n"); 
                #endif
            }
            else if(receiving){

                size_t packet_size = radio->getPacketLength();
                uint8_t packet[packet_size];

                int packet_state = radio->readData(packet, packet_size);

                if (packet_state == RADIOLIB_ERR_NONE)
                {
                    // parse out sync bytes and grab packet with header
                    // create command.c function to read packet
                    #if RADIO_LOGGING
                    printf("Received packet: ");
                    for(int i = 0; i < packet_size; i++){
                        printf("%c", packet[i]);
                    }
                    printf("\n"); 
                    #endif
                    //parse_radio_packet(packet, packet_size);
                    // Check if command is to set output power of the radio

                    parseRebound(radio, packet, packet_size); 
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

                receiving = false; 
            } else {
                state = radioSX.getChannelScanResult(); 

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
                        printf("SX scan failed with code: %d\n", state);
                    }
                    #endif
                    receive_start_time = to_ms_since_boot(get_absolute_time());
                    receiving = true; 
                }
                #if RADIO_LOGGING
                else if (state == RADIOLIB_CHANNEL_FREE) {
                    //printf("channel free\n"); 
                }
                else {
                    printf("failed with code: %d\n", state); 
                }
                #endif

            }

            if(!receiving && !transmitting){
                state = radio->startChannelScan(); 
            }
        }

        if(!transmitting && !receiving && xQueueReceive(radio_queue, &rec, 0))
        {
            switch (rec.operation_type) {
                case TRANSMIT:
                    #if RADIO_LOGGING
                    printf("transmitting...\n");
                    #endif
                    radio->startTransmit(rec.data_buffer, rec.data_size);
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
                    radio_state_RFM = radioRFM.begin(); 
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
                        printf("RFM is not connected\n"); 
                        #endif
                        radio_state_SX = radioSX.begin(434.0, 125.0, 9, 7, 18, 2, 8, 0.0, false);
                        if(radio_state_SX != 0){
                            #if RADIO_LOGGING
                            printf("switch back to SX1268 failed with code: %d\n", radio_state_SX);
                            #endif
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
                    radio_state_SX = radioSX.begin(434.0, 125.0, 9, 7, 18, 2, 8, 0.0, false);
                    if(radio_state_SX == 0){
                        radioRFM.clearDio0Action();
                        radioRFM.clearDio1Action(); 
                        radioRFM.sleep(); 
                        radio = &radioSX; 
                        radioSX.standby(); 
                        radioSX.setDio1Action(radio_general_flag_SX);
                        radioSX.startChannelScan(); 
                        #if RADIO_LOGGING
                        printf("success\n"); 
                        #endif
                        // vPortFree(rec.data_buffer); 
                    }
                    else{
                        #if RADIO_LOGGING
                        printf("SX is not connected\n"); 
                        #endif
                        radio_state_RFM = radioRFM.begin(); 
                        if(radio_state_RFM != 0){
                            #if RADIO_LOGGING
                            printf("switch back failed with code: %d\n", radio_state_RFM);
                            #endif
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
