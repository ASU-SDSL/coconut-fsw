// source file for one wire
void setup() {
  int64_t ROM_CODE = ; //get rom code 
}
float getTempC() {
    sendCommand(MATCH_ROM, CONVERT_T, !selectedPowerMode);
    delayForConversion(selectedResolution, selectedPowerMode);
    readScratchpad();
    uint8_t lsb = selectedScratchpad[TEMP_LSB];
    uint8_t msb = selectedScratchpad[TEMP_MSB];

    switch (selectedResolution) {
        case 9:
            lsb &= 0xF8;
            break;
        case 10:
            lsb &= 0xFC;
            break;
        case 11:
            lsb &= 0xFE;
            break;
    }

    uint8_t sign = msb & 0x80;
    int16_t temp = (msb << 8) + lsb;

    if (sign) {
        temp = ((temp ^ 0xffff) + 1) * -1;
    }

    return temp / 16.0;
}
int alarm() {
  // add code later
  return 0;
}
