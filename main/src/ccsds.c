#include "ccsds.h"

// modified from https://github.com/altillimity/libccsds/blob/master/src/ccsds.cpp
ccsds_header_t parse_ccsds_header(uint8_t *header_buf) {
    ccsds_header_t header;
    // copy raw bytes
    memcpy(header.raw, header_buf, CCSDS_HEADER_SIZE);
    // set struct fields using bitwise operations
    header.version = (header_buf[0] >> 5) & 0x07;
    header.type = (header_buf[0] >> 4) & 0x01;
    header.secondary_header_flag = (header_buf[0] >> 3) & 0x01;
    header.apid = ((header_buf[0] & 0x07) << 8) | header_buf[1];
    header.sequence_flag = (header_buf[2] >> 6) & 0x01;
    header.packet_sequence_count = ((header_buf[2] & 0x3F) << 8) | header_buf[3];
    header.packet_length = (header_buf[4] << 8) | header_buf[5];
    return header;
}

// chatgpts attempt at an encoder
bool encode_ccsds_header(ccsds_header_t header, uint8_t* out_buf) {
    // make sure out_buf is at least 6 bytes
    if (out_buf == 0) {
        return false;
    }
    
    out_buf[0] = (header.version << 5) | (header.type << 4) | (header.secondary_header_flag << 3) | ((header.apid >> 8) & 0x07);
    out_buf[1] = header.apid & 0xFF;
    out_buf[2] = (header.sequence_flag << 6) | ((header.packet_sequence_count >> 8) & 0x3F);
    out_buf[3] = header.packet_sequence_count & 0xFF;
    out_buf[4] = (header.packet_length >> 8) & 0xFF;
    out_buf[5] = header.packet_length & 0xFF;
    
    return true;
}