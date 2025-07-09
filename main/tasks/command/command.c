#include "pico/stdlib.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "queue.h"
#include "gse.h"
#include "log.h"
#include "heartbeat_job.h"
#include "steve.h"
#include "filesystem.h"
#include "radio.h"
#include "command.h"
#include "set_rtc_job.h"
#include "watchdog.h"
#include "hb_tlm_log.h"
#include "main.h"

void receive_command_byte_from_isr(char ch) {
    // ONLY USE FROM INTERRUPTS, CREATE NEW METHOD FOR QUEUEING CMD BYTES FROM TASKS
    // Send to command queue
    if (command_byte_queue) {
        xQueueSendToBackFromISR(command_byte_queue, &ch, 0);
    }
}

void receive_command_byte(char ch) {
    // Wait for command queue
    while (!command_byte_queue) {
        vTaskDelay(1);
    }
    // Send to command queue
    xQueueSendToBack(command_byte_queue, &ch, 0);
}

void receive_command_bytes(uint8_t* packet, size_t packet_size) {
    for (int i = 0; i < packet_size; i++ ){
        receive_command_byte(packet[i]);
    }
}


uint32_t command_count = 0;

uint32_t get_command_count(void){
    uint32_t temp_commandCount;

    xSemaphoreTake(commandCountMutex, portMAX_DELAY);
    temp_commandCount = command_count;
    xSemaphoreGive(commandCountMutex);
    
    return temp_commandCount;
}

void parse_command_packet(spacepacket_header_t header, uint8_t* payload_buf, uint32_t payload_size) {
    
    xSemaphoreTake(commandCountMutex, portMAX_DELAY);
    command_count++;
    xSemaphoreGive(commandCountMutex);

    logln_info("Received command with APID: %hu", header.apid);
    
    
    // Used for the ack struct
    uint8_t command_status = 1; // 1 for success/true, 0 for failure/false
    // Data may or may not be returned, this data should be allocated and freed if needed depending on the packet
    uint8_t *return_data = NULL;
    int return_data_len = 0;

    switch (header.apid) {
        case UPLOAD_USER_DATA:
            if (payload_size < sizeof(upload_user_data_t)) break;
            upload_user_data_t* upload_user_data_args = (upload_user_data_t*)payload_buf;
            if (upload_user_data_args->data_len > (payload_size - sizeof(upload_user_data_t))) break;
            upload_user_data(upload_user_data_args->user_token, upload_user_data_args->data, upload_user_data_args->data_len);
            break;
        case CHANGE_HEARTBEAT_TELEM_RATE:
            if (payload_size < sizeof(change_heartbeat_telem_rate_t)) break;
            change_heartbeat_telem_rate_t* heartbeat_tr_args = (change_heartbeat_telem_rate_t*)payload_buf;
            if (!is_admin(heartbeat_tr_args->admin_token)) break;
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
            if (!is_admin(ls_args->admin_token)) break;
            list_dir(ls_args->path);
            break;
        case FILE_MKDIR:
            if (payload_size < sizeof(file_mkdir_t)) break;
            file_mkdir_t* mkdir_args = (file_mkdir_t*)payload_buf;
            if (!is_admin(mkdir_args->admin_token)) break;
            make_dir(mkdir_args->path);
            break;
        case FILE_CAT:
            if (payload_size < sizeof(file_cat_t)) break;
            file_cat_t* cat_args = (file_cat_t*)payload_buf;
            if (!is_admin(cat_args->admin_token)) break;
            cat(cat_args->path);
            break;
        case FILE_DELETE: 
            if (payload_size < sizeof(file_delete_t)) break;
            file_delete_t* delete_args = (file_delete_t*)payload_buf;
            if (!is_admin(delete_args->admin_token)) break;
            delete_file(delete_args->path);
            break;
        case FILE_APPEND:
            if (payload_size < sizeof(file_append_t)) break;
            file_append_t* append_args = (file_append_t*)payload_buf;
            if (!is_admin(append_args->admin_token)) break;
            if (append_args->data_len > (payload_size - sizeof(file_append_t))) break;
            write_file(append_args->path, append_args->data, append_args->data_len, true);
            break;
        case FILE_TOUCH:
            if (payload_size < sizeof(file_touch_t)) break;
            file_touch_t* touch_args = (file_touch_t*)payload_buf;
            if (!is_admin(touch_args->admin_token)) break;
            touch(touch_args->path);
            break;
        case FILE_MKFS:
            if (payload_size < sizeof(file_mkfs_t)) break;
            file_mkfs_t* mkfs_args = (file_mkfs_t*)payload_buf;
            if (!is_admin(mkfs_args->admin_token)) break;
            if (mkfs_args->confirm != 1) break;
            make_filesystem();
            break;
        case ADD_USER:
            if (payload_size < sizeof(add_user_t)) break;
            add_user_t* add_user_args = (add_user_t*)payload_buf;
            if (!is_admin(add_user_args->admin_token)) break;
            add_user(add_user_args->new_user_name, add_user_args->new_user_token);
            break;
        case DELETE_USER:
            if (payload_size < sizeof(delete_user_t)) break;
            delete_user_t* delete_user_args = (delete_user_t*)payload_buf;
            if (!is_admin(delete_user_args->admin_token)) break;
            if (mkfs_args->confirm != 1) break;
            delete_user(delete_user_args->user_name);
            break;

        case MCU_POWER_CYCLE:
            if(payload_size < sizeof(mcu_power_cycle_t)) break;
            mcu_power_cycle_t* mcu_power_cycle_args = (mcu_power_cycle_t*)payload_buf; 
            if (!is_admin(mcu_power_cycle_args->admin_token)) break; // check admin

            watchdog_freeze(); // Freezing the watchdog will cause a reboot within a few seconds
            break;

        case PLAYBACK_HEARTBEAT_PACKETS:
            if (payload_size < sizeof(playback_hb_tlm_payload_t)) break; // Should probably return an error to the ground
            playback_hb_tlm_payload_t* playback_hb_payload = (playback_hb_tlm_payload_t*)payload_buf;
            if (!is_admin(playback_hb_payload->admin_token)) break; // check admin

            int status = hb_tlm_playback(playback_hb_payload);
            if (status != 0) command_status = 0;
            break;
        
        case RADIO_CONFIG: 
            if(payload_size < sizeof(radio_config_t)) break; 
            radio_config_t* radio_config_args = (radio_config_t*)payload_buf; 
            // check admin
            if(!is_admin(radio_config_args->admin_token)) break;
            logln_info("Attempting to switch to %d with power %d", radio_config_args->selected_radio, radio_config_args->updated_power);
            // queue module update if arg isn't 2 
            if(radio_config_args->selected_radio == 1) radio_set_module(ENABLE_RFM98); 
            else if(radio_config_args->selected_radio == 0) radio_set_module(ENABLE_SX1268); 

            // queue power update if arg isn't 0
            if(radio_config_args->updated_power != 0) radio_set_transmit_power(radio_config_args->updated_power); 
            break;

        case RADIO_STAT:
            if(payload_size < sizeof(radio_stat_t)) break;
            radio_stat_t* radio_stat_args = (radio_stat_t*)payload_buf; 
            if(!is_admin(radio_stat_args->admin_token)) break;

            logln("Queuing stat response"); 
            radio_queue_stat_response(); 
            break;

        case SET_RTC_TIME:
            if (payload_size < sizeof(set_rtc_time_t)) break;
            set_rtc_time_t* set_rtc_time_args = (set_rtc_time_t*)payload_buf; 
            if(!is_admin(set_rtc_time_args->admin_token)) break;

            void* args = pvPortMalloc(payload_size); 
            memcpy(args, payload_buf + sizeof(set_rtc_time_args->admin_token), payload_size - sizeof(set_rtc_time_args->admin_token));
            // logln_info("args[0]: %d payload_buf: %d len: %d", (((uint8_t*)args)[0]), *payload_buf, payload_size); 

            const char* job_name = "SET_RTC";
            schedule_delayed_job_ms(job_name, &set_rtc_job, 10); 
            steve_job_t* job = find_steve_job(job_name); 
            job->arg_data = args; 
            logln_info("RTC job created"); 
            break;
            
        case FSW_ACK:
            break; // This will just send the ack
        default:
            logln_error("Received command with unknown APID: %hu", header.apid);
    }

    // Send ack

    int ack_size = sizeof(ack_telemetry_t) + return_data_len;
    ack_telemetry_t *ack = pvPortMalloc(ack_size);
    ack->command_status = command_status;
    ack->data = return_data;

    // Get last error
    char last_error[MAX_ERROR_LOG_STR_SIZE];
    get_most_recent_logged_error(last_error, MAX_ERROR_LOG_STR_SIZE);
    strncpy(ack->last_logged_error, last_error, sizeof(ack->last_logged_error)); // Copy only first 24 chars (or however many the ack struct says)

    send_telemetry(ACK, (char*) ack, ack_size); // Send ack

    vPortFree(ack);
}


void command_task(void* unused_arg) {
    // Initialize byte queue
    command_count = 0; // Reset command count
    

    command_byte_queue = xQueueCreate(COMMAND_MAX_QUEUE_ITEMS, sizeof(command_byte_t));
    while (true) {
        // Keep gathering bytes until we get the sync bytes
        uint32_t sync_index = 0;
        while (true) {
            // stall thread until we get a byte
            command_byte_t command_byte = 0;
            xQueueReceive(command_byte_queue, &command_byte, portMAX_DELAY);
            
            logln_info("Byte Received: 0x%hhx", command_byte);

            // check if current sync index byte matches
            uint8_t check_byte = COMMAND_SYNC_BYTES[sync_index];
            // logln_info("Comparing with Sync Byte %d: 0x%hhx", sync_index, check_byte);
            if (command_byte != check_byte) {
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
        logln_info("Received sync bytes, moving to decode"); 
        // We've succesfully received all sync bytes if we've reached here
        // logln_info("Received all sync bytes!");
        // TODO: Add better error checks and handling here
        // Gather spacepacket header bytes
        uint8_t spacepacket_header_bytes[SPACEPACKET_ENCODED_HEADER_SIZE];
        for (int i = 0; i < sizeof(spacepacket_header_bytes); i++) {
            xQueueReceive(command_byte_queue, &spacepacket_header_bytes[i], portMAX_DELAY);
        }
        // Parse spacepacket header
        spacepacket_header_t header;
        if (decode_spacepacket_header(spacepacket_header_bytes, sizeof(spacepacket_header_bytes), &header) == -1) {
            logln_error("Failed to decode SpacePacket header!");
            continue;
        }
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
