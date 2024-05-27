#include <RadioLib.h>
#include "RadioLibPiHal.h"
#include "radio.h"
#include <FreeRTOS.h>
#include "command.h"
#include "PicoHal.h"

#define ERR_NONE 0
#define NULL_QUEUE_WAIT_TIME 100

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



PicoHal *picoHal = new PicoHal(spi0, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_SCK_PIN);
// Add interupt pin
RFM98 radioRFM = new Module(picoHal, RADIO_RFM_NSS_PIN, RADIO_RFM_IRQ_PIN, RADIO_RFM_NRST_PIN, RADIOLIB_NC); //RADIOLIB_NC); // RFM98 is an alias for SX1278
SX1268 radioSX = new Module(picoHal, RADIO_SX_NSS_PIN, RADIO_SX_IRQ_PIN, RADIO_SX_NRST_PIN, RADIO_SX_BUSY_PIN); 
PhysicalLayer* radio = &radioSX;

volatile bool operation_done = false;

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
        telemetry_queue_transmission_t new_buffer;
        new_buffer.payload_size = size;
        // Allocate chunk on heap to copy buffer contents
        auto heap_buf = static_cast<char *>(pvPortMalloc(size));
        memcpy(heap_buf, buffer, size);
        new_buffer.payload_buffer = heap_buf;
        // Wait for queue to become available
        while (!radio_queue)
        {
            vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS);
        }
        xQueueSendToBack(radio_queue, &new_buffer, portMAX_DELAY);
    }
#ifdef __cplusplus
}
#endif

void radio_operation_done()
{
    printf("radio_operation_done\n");
    operation_done = true;
}

void init_radio()
{
    // trying to blow the stack 
    // RFM98 bomb = new Module(picoHal, RADIO_RFM_NSS_PIN, RADIO_RFM_IRQ_PIN, RADIO_RFM_NRST_PIN, RADIOLIB_NC); //RADIOLIB_NC); // RFM98 is an alias for SX1278
    // printf("bomb"); 
    // init_radio(); 

    sleep_ms(1000); // for debugging
    // printf("Resetting Radios...\n");
    // radioRFM.reset(); 
    // printf("RFM Reset.\n");
    // radioSX.reset(false);
    // printf("SX Reset.\n");
    // sleep_ms(50);

    printf("Size of rfm98: %d sx1268: %d hal: %d queueItem: %d\n", sizeof(RFM98)/4, sizeof(SX1268)/4, sizeof(PicoHal)/4, sizeof(radio_queue_operations_t)/4 * 64);

    UBaseType_t wm = uxTaskGetStackHighWaterMark(NULL); 
    printf("Task Watermark: %d (in words)\n", wm);

    printf("1\n");
    int radio_state_RFM = radioRFM.begin();
    printf("2\n");
    int radio_state = radioSX.begin(434.0, 125.0, 9, 7, 18, 2, 8, 0.0, false);
    printf("3: RFM: %d SX: %d\n", radio_state_RFM, radio_state);
    radio = &radioSX; 

    // printf("3.5\n");
    // radioSX.setDio1Action(radio_operation_done); 
    // printf("after setDio1Action\n"); 
    // radioRFM.setPacketReceivedAction(radio_operation_done); 
    // printf("after rfm setPacketReceivedAction\n");
    printf("3.5\n");
    
    wm = uxTaskGetStackHighWaterMark(NULL); 
    printf("Task Watermark: %d (in words)\n", wm);
    //radio->setPacketReceivedAction(radio_operation_done); 

    printf("4\n");
    int receive_state = radio->startReceive();
    printf("5\n");
    if (receive_state == RADIOLIB_ERR_NONE)
    {
        printf("Success, recieving...\n");
    }
    else
    {
        printf("failed");
        while (true){
            printf("recieve setup failed with code %d\n", receive_state);
        }
    }
}

void set_power_output(PhysicalLayer* radio_module, int8_t new_dbm){
    radio_module->setOutputPower(new_dbm); 
}

/**
 * Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task_cpp()
{
    sleep_ms(1000); 
    printf("Starting Radio Task\n");
    init_radio();
    radio_queue = xQueueCreate(RADIO_MAX_QUEUE_ITEMS, sizeof(radio_queue_operations_t)); // telemetry_queue_transmission_t));
    printf("Immediate queue status: %d\n", !radio_queue);
    radio_queue_operations_t rec;
    bool transmitting = false;

    while (true)
    {
        if (operation_done)
        {
            if(transmitting){
                printf("done transmitting\n"); 
                transmitting = false;
                radio->startReceive(); 
                operation_done = false; 
            }
            else {
                printf("package received\n");
                //uint8_t *packet; needs memeory allocated to ti
                size_t packet_size = radio->getPacketLength();
                uint8_t packet[packet_size];

                int packet_state = radio->readData(packet, packet_size);
                // acknowledge packet has been recieved 
                operation_done = false;

                if (packet_state == RADIOLIB_ERR_NONE)
                {
                    // parse out sync bytes and grab packet with header
                    // create command.c function to read packet
                    printf("Recieved packet: ");
                    for(int i = 0; i < packet_size; i++){
                        printf("%c", packet[i]);
                    }
                    printf("\n"); 
                    //parse_radio_packet(packet, packet_size);
                    // Check if command is to set output power of the radio
                }
                else if (packet_state == RADIOLIB_ERR_CRC_MISMATCH)
                {
                    printf("CRC Error!!");
                }
                else
                {
                    printf("Packet Reading failed");
                }
            }
        }

        // should maybe move to interrupt based transmit but may cause UB when combined with recieve interrupts
        if (xQueueReceive(radio_queue, &rec, 0))
        {
            switch (rec.operation_type) {
                case TRANSMIT:
                    printf("transmitting...\n");
                    radio->startTransmit(rec.data_buffer, rec.data_size);
                    transmitting = true; 
                    vPortFree(rec.data_buffer);
                    break;

                case SET_OUTPUT_POWER:
                    set_power_output(radio, rec.data_buffer[0]);
                    vPortFree(rec.data_buffer);
                    break;
                
                // case ENABLE_RFM98:
                //     if(radio == &radioRFM) break;
                //     radio->clearPacketReceivedAction();
                //     radio = &radioRFM;
                //     radio->setPacketReceivedAction(radio_operation_done);
                //     vPortFree(rec.data_buffer); 
                //     break;
                
                // case ENABLE_SX1268:
                //     if(radio == &radioSX) break;
                //     radio->clearPacketReceivedAction();
                //     radio = &radioSX;
                //     radio->setPacketReceivedAction(radio_operation_done);
                //     vPortFree(rec.data_buffer); 
                //     break;

            }
            
        }

    }
}
