#include <fstream>

#include <google/protobuf/text_format.h>

#include "command_proto_mutator/command.pb.h"

int main(int argc, char** argv) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    // Make protobuf obj
    FuzzInputs *inputs = new FuzzInputs();
    inputs->add_inputs();

    FuzzInput *input = inputs->mutable_inputs(0);
    SpacePacketHeader *header = input->mutable_command()->mutable_header();
    // header->set_version(0);
    // header->set_type(0);
    // header->set_secondary_header_flag(0);
    header->set_apid(CHANGE_HEARTBEAT_TELEM_RATE);
    // header->set_sequence_flag(0);
    // header->set_packet_sequence_count(0);
    const char *payload = "\xFF\x00\x00\x00";
    header->set_packet_length(4-1);
    
    input->set_ignore_packet_length(0);
    
    input->mutable_command()->set_payload(payload, 4);

    // // Write binary output to file
    // std::ofstream ofs(argv[1], std::ios_base::out | std::ios_base::binary);
    // inputs->SerializeToOstream(&ofs);
    // std::cout << "Writing to file " << argv[1] << std::endl; 

    // Write text output to stdout 
    std::string buffer; 
    google::protobuf::TextFormat::PrintToString(*inputs, &buffer);
    std::cout << buffer;  
    // inputs->PrintDebugString(); 


    ////// Convert binary input to string
    // std::ifstream ifs(argv[1], std::ios_base::in | std::ios_base::binary);

    // // Convert protobuf
    // FuzzInputs inputs;
    // inputs.ParseFromIstream(&ifs);

    // // Get text rep
    // inputs.PrintDebugString();

    return 0;
}