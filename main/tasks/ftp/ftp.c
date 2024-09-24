#include "ftp.h"

static inline uint8_t get8(uint8_t* ptr) {
    return ptr[0];
}

static inline uint16_t get16(uint8_t* ptr) {
    return (ptr[0] << 8) + ptr[1];
}

static inline uint32_t get32(uint8_t* ptr) {
    return (ptr[0] << 24) + (ptr[1] << 16) + (ptr[2] << 8) + ptr[3];
}

/**
 * Enqueue CCSDS payload for FTP processing
 */
void ftp_queue_message(char *buffer, size_t size) {
    ftp_queue_operations_t state;
    state.buffer = buffer;
    state.size = size;
    xQueueSendToBack(ftp_queue, &state, portMAX_DELAY);
}

/**
 * Check if provided credentials are valid
 */
bool valid_creds(char* creds, uint16_t len) {
    return true;
}

/**
 * Check if user can modify a certain path
 * ie CRUD
 */
uint8_t get_perms(char* path, uint16_t path_len, char* creds, uint16_t cred_len) {
    return 0b1111;
}

/**
 * Check if path exists
 */
bool path_exists(char* path, uint16_t len) {
    return true;
}

/**
 * Delete a file / directory
 * Returns success
 */
bool delete_path(char* path, uint16_t len) {
    return true;
}

/**
 * Create a file / directory
 * Returns success
 */
bool create_path(char* path, uint16_t len) {
    return true;
}

/**
 * Get sequence number associated with a path
 */
uint16_t get_seq_id(char* path, uint16_t len) {
    return 0;
}

/**
 * Store sequence number associated with a path
 */
bool save_seq_id(uint16_t seq_id, char* path, uint16_t len) {
    return true;
}

/**
 * Main task
 */
void ftp_task(void* unused_arg) {
    ftp_queue = xQueueCreate(FTP_MAX_QUEUE_ITEMS, sizeof(ftp_queue_operations_t));

    while (true) {
        ftp_queue_operations_t data;
        ftp_state state;
        xQueueReceive(ftp_queue, &data, portMAX_DELAY);

        // Extract important FTP info
        state.buffer = data.buffer;
        state.size = data.size;

        uint8_t* curr = state.buffer;
        size_t size = state.size;

        if (size < 5) goto end;

        state.op = get8(curr); curr++;
        state.creds_length = get16(curr); curr += 2;
        state.path_length = get16(curr); curr += 2;

        if (size < 5 + state.creds_length + state.path_length) goto end;

        state.creds = curr; curr += state.creds_length;
        state.path = curr; curr += state.path_length;

        // Check valid credentials
        if (!valid_creds(state.creds, state.creds_length)) {
            // reply = create_reply(invalid_creds)
            goto reply;
        }

        // Check user permissions for path
        uint8_t perms = get_perms(state.path, state.path_length, state.creds, state.creds_length);
        bool exists = path_exists(state.path, state.path_length);

        // TODO: Do whatever the operation told you to
        // Replies should also contain file path and/or user so nodes know who each reply is for
        switch (state.op) {
            case FTP_REMOVE:
                if (exists && (perms & PERM_DELETE)) {
                    if (delete_path(state.path, state.path_length)) {
                        // reply = create_reply(rm_ack)
                    } else {
                        // reply = create_reply(rm_fail)
                    }
                } else {
                    if (!exists) {
                        // reply = create_reply(does_not_exist)
                    } else {
                        // reply = create_reply(rm_not_allowed)
                    }
                }
                break;
            case FTP_LIST:
                if (exists && (perms & PERM_READ)) {
                    // Should also include file sizes
                    // reply = create_reply(ls(state.path, state.path_length))
                } else {
                    if (!exists) {
                        // reply = create_reply(does_not_exist)
                    } else {
                        // reply = create_reply(ls_not_allowed)
                    }
                }
                break;
            case FTP_CREATE:
                if (!exists && (perms & PERM_CREATE)) {
                    if (create_path(state.path, state.path_length)) {
                        // reply = create_reply(mk_ack)
                    } else {
                        // reply = create_reply(mk_fail)
                    }
                } else {
                    if (exists) {
                        // reply = create_reply(already_exists)
                    } else {
                        // reply = create_reply(mk_not_allowed)
                    }
                }
                break;
            case FTP_APPEND:
                uint16_t seq_id = get_seq_id(state.path, state.path_length);

                if (exists && (perms & PERM_UPDATE)) {
                    // f_handle = create_handle(state.path, state.path_length, APPEND)
                    // if (write(f_handle, curr, size - 5 - state.creds_length - staet.path_length)) {
                    //     reply = create_reply(app_ack)
                    // }
                    // else {
                    //     reply = creaet_reply(app_fail)
                    // }
                } else {
                    if (!exists) {
                        // reply = create_reply(does_not_exist)
                    } else {
                        // reply = create_reply(app_not_allowed)
                    }
                }

                // store state
                bool saved = save_seq_id(seq_id, state.path, state.path_length);

                if (!saved) {
                    // oh no
                }

                break;
            case FTP_COPY:
                if (exists && (perms & PERM_READ)) {
                    // f_handle = create_handle(state.path, state.path_length, READ_ONLY)
                    // offset = get32(curr) // COPY should contain file offset as the additional payload, 32 bits
                    // results = read(f_handle, offset, MAX_SIZE ie 1024)
                    // if (results.valid) {
                    //     reply = create_reply(read_ack, results.buffer, results.size)
                    // }
                    // else {
                    //     reply = create_reply(read_fail)
                    // }
                } else {
                    if (!exists) {
                        // reply = create_reply(does_not_exist)
                    } else {
                        // reply = create_reply(read_not_allowed)
                    }
                }
                break;
            default:
                break;
        }

reply:
        // TODO: Reply with proper packet, enqueue for transmission
        uint8_t* payload_buffer; // = encode_reply(reply)
        size_t total_payload_size; // = get_reply_size(reply)

        // Send reply through UART
        gse_queue_message(payload_buffer, total_payload_size);

        // Send reply through radio
        radio_queue_message(payload_buffer, total_payload_size);
end:
        vPortFree(state.buffer);
    }
}
