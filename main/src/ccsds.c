#include "ccsds.h"

// modified from https://github.com/altillimity/libccsds/blob/master/src/ccsds.cpp
ccsds_header_t parse_ccsds_header(uint8_t *header_buf) {
    ccsds_header_t header;
    // copy raw bytes
    memcpy(header.raw, header_buf, CCSDS_HEADER_SIZE);
    // set struct fields
    header.version = header_buf[0] >> 5;
    header.type = (header_buf[0] >> 4) % 2;
    header.secondary_header_flag = (header_buf[0] >> 3) % 2;
    header.apid = (header_buf[0] % (int)pow(2, 3)) << 8 | header_buf[1];
    header.sequence_flag = header_buf[2] >> 6;
    header.packet_sequence_count = (header_buf[2] % (int)pow(2, 6)) << 8 | header_buf[3];
    header.packet_length = header_buf[4] << 8 | header_buf[5];
    return header;
}