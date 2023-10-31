#ifndef CCSDS_FILE_DEFINED
#define CCSDS_FILE_DEFINED

#include <stdint.h>
#include <stdbool.h>

#define CCSDS_ENCODED_HEADER_SIZE 6

struct ccsds_header {
    uint8_t version;
    bool type;
    bool secondary_header_flag;
    uint16_t apid;
    uint8_t sequence_flag;
    uint16_t packet_sequence_count;
    uint16_t packet_length;
} typedef ccsds_header_t;

ccsds_header_t parse_ccsds_header(uint8_t* header_buf);

// TODO: Needs implementation
bool encode_ccsds_header(ccsds_header_t header, uint8_t* out_buf);

#endif /* !CCSDS_FILE_DEFINED */