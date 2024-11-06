#include "command.h"

void receive_command_byte_from_isr(char ch) {
    // ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING CMD BYTES FROM TASKS
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBackFromISR(command_byte_queue, &ch, NULL);
    }
}

void receive_command_byte(char ch) {
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBack(command_byte_queue, &ch, NULL);
    }
}

void receive_command_bytes(uint8_t* packet, size_t packet_size) {
    if (command_byte_queue) {
        for (int i = 0; i < packet_size; i++ ){
            xQueueSendToBack(command_byte_queue, &packet[i], portMAX_DELAY);
        }
    }
}

void parse_command_packet(ccsds_header_t header, uint8_t* payload_buf, uint32_t payload_size) {
    logln_info("Received command with APID: %hu", header.apid);
    switch (header.apid) {
        case STORE_GROUNDNODE_DATA:
            // TODO: Implement
            logln_error("STORE_GROUNDNODE_DATA unimplemented");
            break;
        case CHANGE_HEARTBEAT_TELEM_RATE:
            if (payload_size < sizeof(change_heartbeat_telem_rate_t)) break;
            change_heartbeat_telem_rate_t* heartbeat_tr_args = (change_heartbeat_telem_rate_t*)payload_buf;
            if (check_password(heartbeat_tr_args->password) == false) break;
            edit_steve_job_recur_time(HEARTBEAT_JOB_NAME, heartbeat_tr_args->ms);
            break;
        case REQUEST_DOWNLINK_GROUNDNODE_DATA:
            // TODO: Implement
            logln_error("REQUEST_DOWNLINK_GROUNDNODE_DATA unimplemented");
            break;
        case REQUEST_DOWNLINK_TELEMETRY_DATA:
            // TODO: Implement
            logln_error("REQUEST_DOWNLINK_TELEMETRY_DATA unimplemented");
            break;
        case LIST_STEVE_TASKS:
            print_debug_exec_times();
            break;
        case FILE_LS:
            if (payload_size < sizeof(file_ls_t)) break;
            file_ls_t* ls_args = (file_ls_t*)payload_buf;
            if (check_password(ls_args->password) == false) break;
            list_directory(ls_args->path);
            break;
        case FILE_MKDIR:
            if (payload_size < sizeof(file_mkdir_t)) break;
            file_mkdir_t* mkdir_args = (file_mkdir_t*)payload_buf;
            if (check_password(mkdir_args->password) == false) break;
            make_directory(mkdir_args->path);
            break;
        case FILE_CAT:
            if (payload_size < sizeof(file_cat_t)) break;
            file_cat_t* cat_args = (file_cat_t*)payload_buf;
            if (check_password(cat_args->password) == false) break;
            cat(cat_args->path);
            break;
        case FILE_DELETE: 
            if (payload_size < sizeof(file_delete_t)) break;
            file_delete_t* delete_args = (file_delete_t*)payload_buf;
            if (check_password(delete_args->password) == false) break;
            delete_file(delete_args->path);
            break;
        case FILE_APPEND:
            if (payload_size < sizeof(file_append_t)) break;
            file_append_t* append_args = (file_append_t*)payload_buf;
            if (check_password(append_args->password) == false) break;
            if (append_args->data_len > (payload_size - sizeof(file_append_t))) { break; }
            write_file(append_args->path, append_args->data, append_args->data_len, true);
            break;
        case FILE_TOUCH:
            if (payload_size < sizeof(file_touch_t)) break;
            file_touch_t* touch_args = (file_touch_t*)payload_buf;
            if (check_password(touch_args->password) == false) break;
            touch(touch_args->path);
            break;
        case FILE_MKFS:
            if (payload_size < sizeof(file_mkfs_t)) break;
            file_mkfs_t* mkfs_args = (file_mkfs_t*)payload_buf;
            if (check_password(mkfs_args->password) == false) break;
            if (mkfs_args->confirm != 1) break;
            make_filesystem();
            break;
        default:
            logln_error("Received command with unknown APID: %hu", header.apid);
    }
}

bool check_password(const char* password_buf) {
    // Returns true if password_buf and COMMAND_PASSWORD match
    // COMMAND_PASSWORD is defined in base dir CMakeLists.txt
    bool password_match = memcmp(password_buf, COMMAND_PASSWORD, PASSWORD_LENGTH) == 0;
    if (!password_match) {
        logln_error("Failed to authenticate admin password!");
    }
    return password_match;
}

void command_task(void* unused_arg) {
    // Initialize byte queue
    command_byte_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(command_byte_t));
    while (true) {
        // Keep gathering bytes until we get the sync bytes
        uint32_t sync_index = 0;
        while (true) {
            // stall thread until we get a byte
            command_byte_t command_byte = 0;
            xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
            
            logln_info("Byte Received: 0x%hhx", command_byte);
            logln_info("Checking Sync Byte: 0x%hhx", COMMAND_SYNC_BYTES[sync_index]);

            // check if current sync index byte matches
            if (command_byte != COMMAND_SYNC_BYTES[sync_index]) {
                // match unsuccessful
                sync_index = 0;
                continue;
            }
            // match successful
            // see if we've hit all sync bytes and can start parsing the packet
            if (sync_index >= (sizeof(COMMAND_SYNC_BYTES) - 2)) {
                break;
            }
            // otherwise keep checking bytes
            sync_index += 1;
        }
        // We've succesfully received all sync bytes if we've reached here
        // TODO: Add better error checks and handling here
        // Gather spacepacket header bytes
        uint8_t spacepacket_header_bytes[6];
        for (int i = 0; i < CCSDS_ENCODED_HEADER_SIZE; i++) {
            xQueueReceive(command_byte_queue, &spacepacket_header_bytes[i], portMAX_DELAY);
        }
        // Parse spacepacket header
        ccsds_header_t header = parse_ccsds_header(spacepacket_header_bytes);
        // Allocate correct size buffer
        size_t payload_size = header.packet_length + 1; // 4.1.3.5.3 transmits data size field as payload_length - 1
        uint8_t* payload_buf = pvPortMalloc(payload_size);
        if (payload_buf == 0) {
            logln_error("Failed to allocate payload buf of size 0x%x!", payload_size);
            continue;
        }
        // Read payload
        for (int i = 0; i < payload_size; i++) {
            xQueueReceive(command_byte_queue, &payload_buf[i], portMAX_DELAY);
        }
        // Parse packet payload
        parse_command_packet(header, payload_buf, payload_size);
        // Free payload buffer
        vPortFree(payload_buf);
    }
}