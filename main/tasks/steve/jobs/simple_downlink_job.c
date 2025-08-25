#include "simple_downlink_job.h"

void simple_downlink_job(void * args){
  char *file_name = (char*) args; 

  // check if the file exists 
  if(!file_exists(file_name)){
    logln_error("File %s does not exist", file_name); 
    vPortFree(args); 
    return; 
  }

  // get the file size
  FILINFO f_info; 

  if(stat(file_name, &f_info) != 0){
    logln_error("File %s stat failed", file_name); 
    vPortFree(args); 
    return; 
  }

  // read the file 
  char buf[f_info.fsize];
  read_file(file_name, buf, f_info.fsize); 

  // break up the file and queue 
  uint16_t chunk_num = 0; 
  uint16_t chunk_total = f_info.fsize / SDJ_CHUNK_SIZE; 

  while(chunk_num * SDJ_CHUNK_SIZE + SDJ_CHUNK_SIZE < f_info.fsize){
    // to be freed in telemetry? 

    simple_downlink_telemetry_t packet; 
    packet.count = chunk_num; 
    packet.total = chunk_total; 

    memcpy(packet.data, buf + chunk_num * SDJ_CHUNK_SIZE, SDJ_CHUNK_SIZE); 

    send_telemetry(SIMPLE_DOWNLINK_GROUNDNODE_DATA, (char*) &packet, sizeof(simple_downlink_telemetry_t));

    chunk_num++; 
  }

  simple_downlink_telemetry_t end_packet; 
  end_packet.count = chunk_num; 
  end_packet.total = chunk_total; 

  memcpy(end_packet.data, buf + chunk_num * SDJ_CHUNK_SIZE, f_info.fsize - (chunk_num * SDJ_CHUNK_SIZE)); 

  send_telemetry(SIMPLE_DOWNLINK_GROUNDNODE_DATA, (char*) &end_packet, sizeof(simple_downlink_telemetry_t));

  // free memory 
  vPortFree(args); 
}