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

void process_ftp(uint8_t* payload, uint32_t payload_size) {
    ftp_state state;
    uint8_t* curr = payload; // payload cursor

    uint8_t tx_id = get8(curr); curr++;
    ftp_operation_t op = get8(curr); curr++;

    uint16_t creds_length = get16(curr); curr += 2;
    char* creds = curr; curr += creds_length;

    // If new transaction
    if (tx_id == 0xFF && op != CONTINUE) {
        // TODO: Create new tx_id
        // state.tx_id = new_tx_id;

        state.op = op;
        state.creds_length = creds_length;
        state.creds = creds;

        // TODO: Check creds
        // else reply = appropriate error
        // jump to end

        state.path_length = get16(curr); curr += 2;
        state.path = curr; curr += state.path_length;
        state.file_size = get32(curr); curr += 4;

        // TODO: Do whatever the operation told you to
        switch (state.op) {
            case REMOVE:
                // if (exists(state.path))
                // delete(state.path)
                // reply = rm_ack(state.path)
                // else reply = appropriate error
                break;
            case LIST:
                // if (exists(state.path))
                // files = ls(state.path)
                // reply = ls_ack(files)
                // else reply = appropriate error
                break;
            case CREATE:
                // if (not_exists(state.path))
                // f_handle = create(state.path, state.file_size)
                // reply = success(state.path)
                // else reply = appropriate error
                break;
            case APPEND:
                // f_handle = append(state.path, state.file_size)
                // reply = success(state.path)
                // else reply = appropriate error
                break;
            case COPY:
                // if (exists(state.path))
                // f_handle = open(state.path)
                // reply, read_size = copy(f_handle)
                // state.offset += read_size
                // else reply = appropriate error
                break;
            default:
                break;
        }
        
    // Should manually note that this is a continuation of a prev. transaction
    // If FSW is sending packets, consider these as ACKs as well
    // Can also use CCSDS header for this logic, but could get messy with non-unique APIDs and
    // potential simultaneous transactions
    } else if (op == CONTINUE) {
        // TODO: Check creds
        // TODO: Hydrate state from tx_id (including op)
        // TODO: If tx_id doesn't exist, reply error
        // TODO: Do whatever the operation told you to

        switch (state.op) {
            default:
                break;
        }
    }

    // TODO: If no error and should store, store state and start state expiration timer
    if (state.op == APPEND || state.op == CREATE || state.op == COPY) {
        if (state.offset < state.file_size) {
            // Store state
            // Start state expiration timer, may be hard to keep track of and cancel
            // Timers should echo previous packet or delete transaction state
        }
    }

    // TODO: Reply with proper packet, enqueue for transmission
    uint8_t* payload_buffer; // = encode(reply)
    size_t total_payload_size; // = size(reply)

    // Send reply through UART
    gse_queue_message(payload_buffer, total_payload_size);

    // Send reply through radio
    radio_queue_message(payload_buffer, total_payload_size);
}
