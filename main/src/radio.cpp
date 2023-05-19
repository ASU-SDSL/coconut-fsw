#include <RadioLib.h>
#include "RadioLibPiHal.h"



// NOTE: see cmakelists for NonArduino Raspberry example on RadioLib github for some explaination.
// I think the Radiolib library is not being built properly






// The Pysquared board has an RFM98PW radio

PiHal* hal = new PiHal(); // can specify the speed here as an argument if desired

RFM98 radio = new Module(hal, 7, 17, 22, RADIOLIB_NC);

/**
 * @brief Monitor radio, write to SD card, and send stuff when needed
 */
void radio_task(void* unused_arg) {
  // Enable USB serial so we can print
  //stdio_init_all();


}
