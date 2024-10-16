#include <stdint.h>

#include "command.pb.h"
#include "afl_mutator.h"

#include "google/protobuf/text_format.h"

#define SPACE_PACKET_HEADER_SIZE 6U
#define COMMAND_SYNC_BYTES "\x35\x2E\xF8\x53"
#define COMMAND_SYNC_BYTES_SIZE 4U

size_t BuildSpacepackets(const FuzzInputs& inputs, unsigned char **out_buf){
    ///// Calculate total size
    size_t total_size = COMMAND_SYNC_BYTES_SIZE * inputs.inputs().size();
    total_size += SPACE_PACKET_HEADER_SIZE * inputs.inputs().size(); 
    for (const FuzzInput &input : inputs.inputs()) {
        total_size += input.command().payload().size();
    }
    ///// Allocate packet
    uint8_t *buf = new uint8_t[total_size];
    uint8_t *current_buf = buf;
    ////// Build packets from protobufs
    for (const FuzzInput &input : inputs.inputs()) {     
        ////// Add sync bytes
        memcpy(current_buf, COMMAND_SYNC_BYTES, COMMAND_SYNC_BYTES_SIZE);
        current_buf += COMMAND_SYNC_BYTES_SIZE;
        ////// Build spacepacket header 
        const SpacePacketHeader &header = input.command().header();
        uint32_t version = 0; // header.version()
        bool type = false; // header.type()
        bool secondary_header_flag = false; // header.secondary_header_flag()
        APID apid = header.apid();
        uint32_t sequence_flag = 0; // header.sequence_flag()
        uint32_t packet_sequence_count = 0; // header.packet_sequence_count()
        uint32_t packet_length = input.ignore_packet_length() ? total_size : header.packet_length();
        current_buf[0] = (version << 5) | (type << 4) | (secondary_header_flag << 3) | ((apid >> 8) & 0x07);
        current_buf[1] = apid & 0xFF;
        current_buf[2] = (sequence_flag << 6) | ((packet_sequence_count >> 8) & 0x3F);
        current_buf[3] = packet_sequence_count & 0xFF;
        // Deal w variable length option
        current_buf[4] = (packet_length >> 8) & 0xFF;
        current_buf[5] = packet_length & 0xFF;
        current_buf += SPACE_PACKET_HEADER_SIZE;
        ////// Put payload into allocation
        memcpy(current_buf, input.command().payload().c_str(), input.command().payload().size());
        current_buf += input.command().payload().size();
    }
    ////// Return buffer and size
    *out_buf = buf;
    return total_size;
}

DEFINE_AFL_TEXT_PROTO_FUZZER(const FuzzInputs& inputs, unsigned char **out_buf){
    // transfer the input to some interesting DATA
    // and output the DATA to *out_buf
    
    /*
     * @param[in] Protobuf Buffer containing the test case
     * @param[out] out_buf Pointer to the buffer containing the test case after tranferance. 
     * @return Size of the output buffer after processing or the needed amount.
    */
    std::cerr << inputs.DebugString();
    return BuildSpacepackets(inputs, out_buf); 
}