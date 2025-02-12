
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <fstream>

#include "FreeRTOS.h"
#include "command.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "pico/stdlib.h"
#include "queue.h"
#include "task.h"
#include "spacepacket.h"

extern "C" {
BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char *const pcName,
                       const configSTACK_DEPTH_TYPE uxStackDepth,
                       void *const pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t *const pxCreatedTask);

void gse_task(void *pvParameters);

void steve_task(void *pvParameters);

void command_task(void *pvParameters);

void telemetry_task(void *pvParameters);

void filesystem_task(void *pvParameters);

void watchdog_task(void *pvParameters);

void receive_command_bytes(uint8_t *packet, size_t packet_size);
}

#include "command.h"
#include "filesystem.h"
#include "gse.h"
#include "log.h"
#include "steve.h"
#include "telemetry.h"
#include "watchdog.h"

typedef struct fuzzer_input {
    uint8_t *data;
    size_t size;
} fuzzer_input_t;

extern "C" void fuzzer_task(void *input) {
    fuzzer_input_t *fuzzinput = (fuzzer_input_t *)input;
    // Send commands
    receive_command_bytes(fuzzinput->data, fuzzinput->size);
    // Wait
    vTaskDelay(10);
    // End process
    // vTaskEndScheduler();
    exit(0);
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t *Data, uint32_t Size) {
    // Start tasks
    TaskHandle_t scheduler_task_handle;
    BaseType_t scheduler_task_status = xTaskCreate(steve_task,
                                                   "STEVE",
                                                   512,
                                                   NULL,
                                                   1,
                                                   &scheduler_task_handle);

    TaskHandle_t command_task_handle;
    BaseType_t command_task_status = xTaskCreate(command_task,
                                                 "COMMAND",
                                                 512,
                                                 NULL,
                                                 1,
                                                 &command_task_handle);

    TaskHandle_t telemetry_task_handle;
    BaseType_t telemetry_task_status = xTaskCreate(telemetry_task,
                                                   "TELEMETRY",
                                                   256,
                                                   NULL,
                                                   1,
                                                   &telemetry_task_handle);

    TaskHandle_t filesystem_task_handle;
    BaseType_t filesystem_task_status = xTaskCreate(filesystem_task,
                                                    "FILESYSTEM",
                                                    512,
                                                    NULL,
                                                    1,
                                                    &filesystem_task_handle);

    // Setup fuzzer thread
    fuzzer_input_t input;
    input.data = Data;
    input.size = Size;
    TaskHandle_t fuzzer_task_handle;
    BaseType_t fuzzer_task_status = xTaskCreate(fuzzer_task,
                                                "FUZZER",
                                                256,
                                                &input,
                                                1,
                                                &fuzzer_task_handle);

    BaseType_t watchdog_task_status = xTaskCreate(watchdog_task,
                                        "WATCHDOG",
                                        256,
                                        NULL,
                                        1,
                                        &xWatchdogTaskHandler);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();
    return 0;
}

// Put APID's that need the admin token prepended in here
static APID admin_token_apids[] = {
    APID_FILE_LS,
    APID_FILE_MKDIR,
    APID_FILE_CAT,
    APID_FILE_DELETE,
    APID_FILE_APPEND,
    APID_FILE_TOUCH,
    APID_FILE_MKFS
};

static bool needs_admin_token(APID apid) {
    for (int i = 0; i < (sizeof(admin_token_apids) / sizeof(APID)); i++) {
        if (apid == admin_token_apids[i]) {
            return true;
        }
    }
    return false;
}

static size_t proto_to_bytes(char *protobuf_path, uint8_t **out_buf) {
    // Parse protobuf
    // std::ifstream ifs(protobuf_path, std::ios_base::in | std::ios_base::binary);
    // std::cerr << ifs.rdbuf();
    int fd = open(protobuf_path, O_RDONLY);
    google::protobuf::io::FileInputStream file_input(fd);
    file_input.SetCloseOnDelete(true);
    FuzzInputs inputs;
    google::protobuf::TextFormat::Parse(&file_input, &inputs);  // text
    // inputs.ParseFromIstream(&ifs); // binary
    // std::cerr << inputs.DebugString();
    // Calculate total size
    size_t total_size = (sizeof(COMMAND_SYNC_BYTES) - 1) * inputs.inputs().size();
    total_size += SPACEPACKET_ENCODED_HEADER_SIZE * inputs.inputs().size();
    for (const FuzzInput &input : inputs.inputs()) {
        total_size += input.command().payload().size();
        if (needs_admin_token(input.command().header().apid())) {
            total_size += sizeof(ADMIN_TOKEN);
        }
    }
    // Allocate packet
    uint8_t *buf = new uint8_t[total_size];
    uint8_t *current_buf = buf;
    // Build packets from protobufs
    for (const FuzzInput &input : inputs.inputs()) {
        // Add sync bytes
        memcpy(current_buf, COMMAND_SYNC_BYTES, (sizeof(COMMAND_SYNC_BYTES) - 1));
        current_buf += sizeof(COMMAND_SYNC_BYTES) - 1;
        // Build spacepacket header
        const SpacePacketHeader &header = input.command().header();
        uint32_t version = 0;                // header.version()
        bool type = false;                   // header.type()
        bool secondary_header_flag = false;  // header.secondary_header_flag()
        APID apid = header.apid();
        uint32_t sequence_flag = 0;          // header.sequence_flag()
        uint32_t packet_sequence_count = 0;  // header.packet_sequence_count()
        uint32_t packet_length = input.ignore_packet_length() ? total_size : header.packet_length();
        current_buf[0] = (version << 5) | (type << 4) | (secondary_header_flag << 3) | ((apid >> 8) & 0x07);
        current_buf[1] = apid & 0xFF;
        current_buf[2] = (sequence_flag << 6) | ((packet_sequence_count >> 8) & 0x3F);
        current_buf[3] = packet_sequence_count & 0xFF;
        // Deal w variable length option
        current_buf[4] = (packet_length >> 8) & 0xFF;
        current_buf[5] = packet_length & 0xFF;
        current_buf += SPACEPACKET_ENCODED_HEADER_SIZE;
        // Put in admin token if necessary
        if (needs_admin_token(apid)) {
            memcpy(current_buf, ADMIN_TOKEN, sizeof(ADMIN_TOKEN));
            current_buf += sizeof(ADMIN_TOKEN);
        }
        // Put payload into allocation
        memcpy(current_buf, input.command().payload().c_str(), input.command().payload().size());
        current_buf += input.command().payload().size();
    }
    // Return buffer and size
    *out_buf = buf;
    return total_size;
}

int main(int argc, char **argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    // Convert protobuf to bytes
    uint8_t *bytebuf;
    size_t bytesize = proto_to_bytes(argv[1], &bytebuf);
    // Fuzz :)
    LLVMFuzzerTestOneInput(bytebuf, bytesize);
    // free(protobuf);
    free(bytebuf);
    return 0;
}
