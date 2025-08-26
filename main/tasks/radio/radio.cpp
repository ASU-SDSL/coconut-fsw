#include <RadioLib.h>
#include "radio.h"
#include <FreeRTOS.h>
#include "PicoHal.h"
#include <stdlib.h>
#include "log.h"
#include "gse.h"
#include "spacepacket.h"
#include "command.h"
#include "timing.h"

#define ERR_NONE 0
#define NULL_QUEUE_WAIT_TIME 100

#define RADIO_LOGGING 0
#define RADIO_LOGGING_CAD 0
#define TEMP_ON 0

/**
 * one thread
 * queue polling to check on the size of queue each iteration
 * radio receive polling
 * On receive:
 * Send an interrupt when packet receive https://jgromes.github.io/RadioLib/class_s_x127x.html#ad63322c9c58dd82e4b9982f10e546f33
 * In interrupt set a flag -- set the flag to be volatile
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
#define RADIO_NO_CONTACT_DEADMAN_MS (1000UL * 60 * 60 * 24 * 7) // 8 days in ms
#define RADIO_SAVE_INTERVAL_MS (1000UL * 60 * 5) // 5 minutes

PicoHal *picoHal = new PicoHal(spi0, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_SCK_PIN);
// Add interupt pin
RFM98 radioRFM = new Module(picoHal, RADIO_RFM_NSS_PIN, RADIO_RFM_DIO0_PIN, RADIO_RFM_NRST_PIN, RADIO_RFM_DIO1_PIN); //RADIOLIB_NC); // RFM98 is an alias for SX1278
SX1268 radioSX = new Module(picoHal, RADIO_SX_NSS_PIN, RADIO_SX_DIO1_PIN, RADIO_SX_NRST_PIN, RADIO_SX_BUSY_PIN); 
PhysicalLayer* radio = &radioSX;
uint64_t radio_last_received_time = 0; 

int radio_state_RFM = RADIO_STATE_NO_ATTEMPT; 
int radio_state_SX = RADIO_STATE_NO_ATTEMPT;

uint8_t radio_transmit_power = 21;

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
    void radio_queue_stat_response(){
        radio_queue_operations_t buf; 
        buf.operation_type = radio_operation_type_t::RETURN_STATS; 
        while(!radio_queue) 
        {
            vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS); 
        }
        xQueueSendToBack(radio_queue, &buf, portMAX_DELAY); 
    }
    void radio_queue_lora_mode_change(uint8_t new_mode){
        radio_queue_operations_t buf; 
        buf.operation_type = radio_operation_type_t::SET_LORA_MODE; 
        uint8_t* heap_buf = static_cast<uint8_t *>(pvPortMalloc(1));
        *(buf.data_buffer) = new_mode; 
        buf.data_size = 1; 
        while(!radio_queue) 
        {
            vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS); 
        }
        xQueueSendToBack(radio_queue, &buf, portMAX_DELAY); 
    }
    uint8_t radio_which(){
        return (radio == &radioRFM); 
    }
    int16_t radio_get_RFM_state(){
        return radio_state_RFM; 
    }
    int16_t radio_get_SX_state(){
        return radio_state_SX; 
    }
    void radio_flag_valid_packet(){ 
        static uint64_t last_saved_received_time = 0; 
        radio_last_received_time = timing_now();  

        if(radio_last_received_time - last_saved_received_time > RADIO_SAVE_INTERVAL_MS){
            logln_info("Saving last recieved time as %ull", radio_last_received_time);
            char buffer[sizeof(uint64_t)]; 
            memcpy(buffer, &radio_last_received_time, sizeof(uint64_t)); 
            write_file(RADIO_STATE_FILE_NAME, buffer, sizeof(uint64_t), false); 
            last_saved_received_time = radio_last_received_time; 
        }
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

static uint8_t radio_mode = RADIO_SAFE_MODE; 
static uint32_t last_fast_mode_start = 0; 
static bool radio_auto_safe_queued = false; 

int radio_set_mode(uint8_t mode){
    // safe can now be auto queued again 
    if(mode == RADIO_SAFE_MODE) radio_auto_safe_queued = false; 
    // check no change
    if(radio_mode == mode) return 0; 

    radio_mode = mode; 

    // if we're switching to fast mode mark the time, auto switch back to safe mode eventually
    if(mode == RADIO_FAST_MODE){  
        last_fast_mode_start = to_ms_since_boot(get_absolute_time());
    }

    if(radio == &radioRFM){
        radio_begin_rfm(); 
    } else {
        radio_begin_sx(); 
    }
    
    return 0; 
}

// radio initializers 
void radio_begin_rfm(){
    if(radio_mode == RADIO_FAST_MODE){
        radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW_FAST, RADIO_SF_FAST, RADIO_CR_FAST, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
    } else {
        radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW_SAFE, RADIO_SF_SAFE, RADIO_CR_SAFE, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
    }
}

void radio_begin_sx(){
    if(radio_mode == RADIO_FAST_MODE){
        radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW_FAST, RADIO_SF_FAST, RADIO_CR_FAST, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
    } else {
        radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW_SAFE, RADIO_SF_SAFE, RADIO_CR_SAFE, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
    }
}

/**
 * @brief Handle hardware switches between radios 
 * 
 * @param new_radio PhysicalLayer pointer to radio module object
 * @return int currently unused 
 */
int radio_hardware_switch_to(PhysicalLayer* new_radio){
    if(new_radio == &radioRFM){
        // turn off SX
        gpio_put(RADIO_SX_POWER_PIN, 0);
        // turn on RFM 
        gpio_put(RADIO_RFM_POWER_PIN, 1); 
        // rf switch to RFM 
        gpio_put(RADIO_RF_SWITCH_PIN, RADIO_RF_SWITCH_RFM);  

    } else {
        // turn off RFM 
        gpio_put(RADIO_RFM_POWER_PIN, 0);
        // turn on SX
        gpio_put(RADIO_SX_POWER_PIN, 1); 
        // rf switch to SX
        gpio_put(RADIO_RF_SWITCH_PIN, RADIO_RF_SWITCH_SX);  
    }

    // wait for 100 ms (arbitrary) for power on
    vTaskDelay(pdMS_TO_TICKS(100)); 
    return 0; 
}

// toggle the radio until one of them works
void radio_panic(){
    #if TEMP_ON || RADIO_LOGGING
    printf("Critical Radio fail, panicking...\n");
    #endif

    do {
        // short the conditional so that it doesn't switch to the sx as I don't think it is currently not working on the stack 
        if(radio == &radioSX){
            radio_state_SX = RADIO_ERROR_CUSTOM; 
            #if RADIO_LOGGING
            printf("Attempting to switch to RFM98...\n");
            #endif
            // try to clean up if possible
            radioSX.clearDio1Action(); 

            // initialize RFM98
            radio_hardware_switch_to(&radioRFM); 
            //radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
            radio_begin_rfm(); 
            #if RADIO_LOGGING
            printf("Res: %d\n", radio_state_RFM); 
            #endif
            radio = &radioRFM;
            if(radio_state_RFM == 0){
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

            radio_hardware_switch_to(&radioSX); 
            // initialize SX1268
            // radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
            radio_begin_sx(); 
            #if RADIO_LOGGING
            printf("Res: %d\n", radio_state_SX); 
            #endif
            radio = &radioSX; 
            if(radio_state_SX == 0){
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
        #if TEMP_ON || RADIO_LOGGING
        printf("SX: %d RFM: %d\n", radio_state_SX, radio_state_RFM); 
        #endif
        vTaskDelay(pdMS_TO_TICKS(10)); // wait 10 milliseconds between tries
    } while(radio_state_RFM != 0 && radio_state_SX != 0); 

    #if TEMP_ON || RADIO_LOGGING
    printf("Final SX: %d RFM: %d\n", radio_state_SX, radio_state_RFM); 
    if(radio == &radioSX) printf("Resolved to SX1268.\n");
    else printf("Resolved to RFM98.\n");
    #endif
}

void init_radio()
{
    vTaskDelay(pdMS_TO_TICKS(1000)); // for debugging

    // initialize rf switch and power switch gpio
    gpio_init(RADIO_RF_SWITCH_PIN);
    gpio_set_dir(RADIO_RF_SWITCH_PIN, 1); 
    gpio_put(RADIO_RF_SWITCH_PIN, RADIO_RF_SWITCH_RFM); // 0 for RFM 

    gpio_init(RADIO_SX_POWER_PIN); 
    gpio_set_dir(RADIO_SX_POWER_PIN, 1); 
    gpio_put(RADIO_SX_POWER_PIN, 0);

    gpio_init(RADIO_RFM_POWER_PIN);
    gpio_set_dir(RADIO_RFM_POWER_PIN, 1); 
    gpio_put(RADIO_RFM_POWER_PIN, 0); 

    // switch hardware to RFM
    radio_hardware_switch_to(&radioRFM); 

    // If the RFM is physically wired into the board it needs to call begin() before the SX1268
    // my current theory as to why is that it before begin() it is polluting the SPI line
    // radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
    radio_begin_rfm(); 

    if(radio_state_RFM == 0){
        radioRFM.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
        radioRFM.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE); 
        radio = &radioRFM; 
    }
    else {
        radio_hardware_switch_to(&radioSX); 
        // radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
        radio_begin_sx(); 
        if(radio_state_SX == 0){
            radioSX.setDio1Action(radio_general_flag_SX);
            radio = &radioSX;
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

    // load last received time from persistent storage: TODO 
    // check if the persistent log file already exists
    if(file_exists(RADIO_STATE_FILE_NAME)){
        // if it does load the last received time from it
        char result_buffer[sizeof(uint64_t)]; 

        if(read_file(RADIO_STATE_FILE_NAME, result_buffer, sizeof(uint64_t)) == sizeof(uint64_t)) {
            memcpy(&radio_last_received_time, result_buffer, sizeof(uint64_t)); 
            logln_info("Last received time loaded as %ull", radio_last_received_time); 
        } else {
            logln_error("Error on persistent radio time load"); 
        }

    } else { 
        // if it doesn't create it and populate it 
        logln_info("Creating last received time persistent storage...");

        char buf[sizeof(uint64_t)] = {0, 0, 0, 0, 0, 0, 0, 0}; 
        memcpy(buf, &radio_last_received_time, sizeof(uint64_t));
        write_file(RADIO_STATE_FILE_NAME, buf, sizeof(uint64_t), false); 
    }

}

void set_power_output(PhysicalLayer* radio_module, int8_t new_dbm){
    radio_transmit_power = (new_dbm); 
    radio_module->setOutputPower(radio_transmit_power); 
}



/**
 * Monitor radio and send stuff when needed
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
    int transmission_size = 0;  

    while(true){
        // save now time since boot 
        uint32_t radio_now = to_ms_since_boot(get_absolute_time());

        // if there's been no contact for a long time, try to switch radios 
        if(time_since_ms(radio_last_received_time) > RADIO_NO_CONTACT_PANIC_TIME_MS){
            radio_panic(); 
        }

        // if the radio has been in fast mode for too long 
        if(radio_mode == RADIO_FAST_MODE && (radio_auto_safe_queued == false) && time_between(radio_now, last_fast_mode_start) > RADIO_FAST_MODE_MAX_DURATION_MS){
            // queue a switch back to safe more 
            radio_queue_lora_mode_change(RADIO_SAFE_MODE); 
            radio_auto_safe_queued = true; 
        }

        // check operation duration to avoid hanging in an operation mode 
        if(receiving && time_between(radio_now, operation_start_time) > RADIO_RECEIVE_TIMEOUT_MS){
            #if RADIO_LOGGING
            printf("receive timeout\n"); 
            #endif
            receiving = false;
            radio->startChannelScan();
        }
        else if(transmitting && time_between(radio_now, operation_start_time) > RADIO_TRANSMIT_TIMEOUT_MS){
            #if TEMP_ON || RADIO_LOGGING
            printf("transmit timeout\n"); 
            #endif
            transmitting = false;
            radio->finishTransmit(); 
            radio->startChannelScan();
        }

        // handle interrupt flags 
        if(cad_detected_RFM || operation_done_RFM || general_flag_SX){
            // handle finished transmission
            if(transmitting && (operation_done_RFM || general_flag_SX)){
                transmitting = false; 
                radio->finishTransmit();
                #if RADIO_LOGGING
                float duration_s = ((double)radio_now - operation_start_time) / 1000.0; 
                printf("Duration (s): %f\n", duration_s);
                printf("Bit rate: %f\n", transmission_size / duration_s);
                printf("Done.\n"); 
                vTaskDelay(pdMS_TO_TICKS(200)); 
                #endif 
            }

            // handle finished receive 
            else if(receiving){
                
                size_t packet_size = radio->getPacketLength();
                uint8_t packet[packet_size];

                int packet_state = radio->readData(packet, packet_size);

                if (packet_state == RADIOLIB_ERR_NONE)
                {
                    // parse out sync bytes and grab packet with header
                    // create command.c function to read packet
                    #if TEMP_ON || RADIO_LOGGING
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
                    // if the radio should be silenced from deadman switch, don't transmit, but still receive from queue 
                    // check if radio_last_received_time is 0 because if it is that means that we're on since boot time and 
                    // should wait for contact
                    if(radio_last_received_time != 0 && time_since_ms(radio_last_received_time) > RADIO_NO_CONTACT_DEADMAN_MS){
                        // free buffer 
                        vPortFree(rec.data_buffer);
                        break;
                    }
                    #if TEMP_ON || RADIO_LOGGING
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
                    while(state != 0){
                        #if TEMP_ON || RADIO_LOGGING
                        printf("Transmit failed with code: %d\n", state); 
                        #endif
                        radio_panic(); 
                        state = radio->startTransmit(rec.data_buffer, rec.data_size);
                    }
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
                    radio_hardware_switch_to(&radioRFM); 
                    // radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
                    radio_begin_rfm(); 
                    if(radio_state_RFM == 0){
                        radioSX.clearDio1Action(); 
                        radio = &radioRFM;
                        radioRFM.setDio0Action(radio_operation_done_RFM, GPIO_IRQ_EDGE_RISE);
                        radioRFM.setDio1Action(radio_cad_detected_RFM, GPIO_IRQ_EDGE_RISE); 
                        radio->startChannelScan(); 
                        #if RADIO_LOGGING
                        printf("success\n"); 
                        #endif
                        logln_info("Switched to RFM98");
                        // vPortFree(rec.data_buffer); 
                    }
                    else {
                        logln_info("Switch to RFM98 failed with code: %d", radio_state_RFM); 
                        #if RADIO_LOGGING
                        printf("switch to RFM failed with code: %d\n", radio_state_RFM); 
                        #endif
                        radio_hardware_switch_to(&radioSX); 
                        // radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
                        radio_begin_sx(); 
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
                    radio_hardware_switch_to(&radioSX); 
                    // radio_state_SX = radioSX.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_SX_TXCO_VOLT, RADIO_SX_USE_REG_LDO);
                    radio_begin_sx(); 
                    if(radio_state_SX == 0){
                        radioRFM.clearDio0Action();
                        radioRFM.clearDio1Action(); 
                        radio = &radioSX; 
                        radioSX.setDio1Action(radio_general_flag_SX);
                        radio->startChannelScan(); 
                        #if RADIO_LOGGING
                        printf("success\n"); 
                        #endif
                        logln_info("Switched to SX1268"); 
                        // vPortFree(rec.data_buffer); 
                    }
                    else{
                        logln_info("Switch to SX1268 failed with code: %d", radio_state_SX); 
                        #if RADIO_LOGGING
                        printf("switch to SX failed with code: %d\n", radio_state_SX); 
                        #endif
                        radio_hardware_switch_to(&radioRFM); 
                        // radio_state_RFM = radioRFM.begin(RADIO_FREQ, RADIO_BW, RADIO_SF, RADIO_CR, RADIO_SYNC_WORD, radio_transmit_power, RADIO_PREAMBLE_LEN, RADIO_RFM_GAIN);
                        radio_begin_rfm(); 
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

                case RETURN_STATS: 
                {
                    // get radio stats from last transmission
                    size_t payload_size = 3 * sizeof(float); 
                    char payload_buffer[payload_size]; 
                    float temp = radio->getRSSI(); 
                    memcpy(payload_buffer, &temp, sizeof(float));
                    temp = radio->getSNR(); 
                    memcpy(payload_buffer + sizeof(float), &temp, sizeof(float));
                    temp = (radio == &radioSX)? radioSX.getFrequencyError(): radioRFM.getFrequencyError(); 
                    memcpy(payload_buffer + (2*sizeof(float)), &temp, sizeof(float));

                    logln_info("RADIO_STAT_RES queued"); 
                    // send the data to telemetry 
                    send_telemetry(RADIO_STAT_RES, payload_buffer, payload_size);
                    
                }
                    break;

                case SET_LORA_MODE: 
                    radio_set_mode(*(rec.data_buffer)); 
                    vPortFree(rec.data_buffer);
                    break;

                default:
                    logln_error("Bad radio operation: %d", rec.operation_type); 
                    break;
            }
        }
    }

}
