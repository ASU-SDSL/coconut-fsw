#include <RadioLib.h>
#include "RadioLibPiHal.h"
#define ERR_NONE 0
// #include <SX1278.h>
#include <radio.h>
#include <FreeRTOS.h>
#include "command.h"

// USER FUNCTIONS

void set_power_output_request(int new_db) {

    radio_queue_operations_t new_operation;
    new_operation.operation_type = SET_OUTPUT_POWER;
    new_operation.data_buffer = (char*)pvPortMalloc(sizeof(int));
    new_operation.data_size = sizeof(int);
    memcpy(new_operation.data_buffer, &new_db, sizeof(int)); // does this work?

    while(radio_queue == NULL) {
        vTaskDelay(NULL_QUEUE_WAIT_TIME / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(radio_queue, &new_read_operation, portMAX_DELAY);
}

void transmit_request(char* buffer, size_t size) {

    radio_queue_operations_t new_operation;
    
    new_operation.data_size = size;
    // Allocate chunk on heap to copy buffer contents
    char* heap_buf = pvPortMalloc(size);
    memcpy(heap_buf, buffer, size);
    new_operation.data_buffer = heap_buf;
    
    // Wait for queue to become available
    while (uart0_queue == NULL) {
        vTaskDelay(GSE_CHECK_DELAY_MS / portTICK_PERIOD_MS);
    }
    xQueueSendToBack(uart0_queue, &new_buffer, portMAX_DELAY);
}



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

PiPicoHal *hal = new PiPicoHal(spi0); // can specify the speed here as an argument if desired
// Add interupt pin
RFM98 radio = new Module(hal, RADIO_NSS_PIN, RADIO_IRQ_PIN, RADIO_NRST_PIN, RADIOLIB_NC);
volatile bool packet_recieved = false;

#ifdef __cplusplus
extern "C"
{
#endif
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

void radio_packet_recieve()
{
    packet_recieved = true;
}

void init_radio()
{
    hal->init();
    int radio_state = radio.begin();
    if (radio_state == RADIOLIB_ERR_NONE)
    {
        printf("Success, radio initialized");
    }
    else
    {
        printf("failed ");
        while (true)
            ;
    }

    radio.setPacketReceivedAction(radio_packet_recieve);
    int recieve_state = radio.startReceive();
    if (recieve_state == RADIOLIB_ERR_NONE)
    {
        printf("Success, recieving...");
    }
    else
    {
        printf("failed");
        while (true)
            ;
    }
}

void set_power_output(RFM98 radio_module, int8_t new_dbm) {
    radio_module.setOutputPower(new_dbm);
}

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task(void *unused_arg)
{
    init_radio();
    radio_queue = xQueueCreate(RADIO_MAX_QUEUE_ITEMS, sizeof(telemetry_queue_transmission_t));
    telemetry_queue_transmission_t rec;

    while (true)
    {
        if (packet_recieved)
        {
            uint8_t *packet;
            size_t packet_size = radio.getPacketLength();

            int packet_state = radio.readData(packet, packet_size);

            if (packet_state == RADIOLIB_ERR_NONE)
            {
                // parse out sync bytes and grab packet with header
                // create command.c function to read packet
                printf("Recieved packet");
                parse_radio_packet(packet, packet_size);
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

        // should maybe move to interrupt based transmit but may cause UB when combined with recieve interrupts
        if (xQueueReceive(radio_queue, &rec, 0))
        {
            switch (rec.function_id) {
                case TRANSMIT:
                    radio.transmit(rec.data_buffer, rec.data_size);
                    vPortFree(rec.data_buffer);
                    break;

                case SET_OUTPUT_POWER:
                    set_power_output(radio, rec.data_buffer[0]);
                    vPortFree(rec.data_buffer);
                    break;
            }
            
        }

        radio.startReceive();
    }
}
