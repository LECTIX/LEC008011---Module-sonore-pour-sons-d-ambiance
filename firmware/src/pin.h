#ifndef PIN_H
#define PIN_H

#define NB_SWITCHES     14

#define PIN_LED         13    // PB5
#define PIN_DFP_TX      0     // PD0
#define PIN_DFP_RX      1     // PD1
#define PIN_DFP_BUSY    2     // PD2
const uint8_t PIN_SWITCHES[NB_SWITCHES] = {15, 16, 17, 18, 19, 3, 4, 5, 6, 7, 8, 9, 10, 14};
#define PIN_SOUND_VOL   A7    // PE3

#endif
