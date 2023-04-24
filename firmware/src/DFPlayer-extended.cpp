#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "pin.h"
#include <Bounce2.h>

/**
 * LED errors codes : 
 *  1 flash (250 ms) = No SD card detected
 *  2 flashes (250 ms) = Cannot Find File
 *  3 flashes (250 ms) = File Index Out of Bound
 *  4 flashes (250 ms) = Check Sum Not Match
 *  5 flashes (250 ms) = Get Wrong Stack
 *  6 flashes (250 ms) = Sleeping
 *  7 flashes (250 ms) = In Advertise
 *  10 flashes (250 ms) = Time Out!
 *  11 flashes (250 ms) = Stack Wrong!
 *  14 flashes (250 ms) = Card Online!
 *  15 flashes (250 ms) = USB Inserted!
 *  16 flashes (250 ms) = USB Removed!
 * 
 */

void (*resetFunc)(void) = 0;
long mapWithHysteresis(long x, long in_min, long in_max, long out_min, long out_max, byte threshold = 0);
void updateVolume();
void handleErrors(uint8_t type, int value);
void flashLED(int nbFlashs);

DFRobotDFPlayerMini myDFPlayer;
Bounce *switches = new Bounce[NB_SWITCHES];
void printDetail(uint8_t type, int value);

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_DFP_BUSY, INPUT);
  pinMode(PIN_SOUND_VOL, INPUT);

  for (int i = 0; i < NB_SWITCHES; i++)
  {
    switches[i].attach(PIN_SWITCHES[i], INPUT_PULLUP); //setup the bounce instance for the current button
    switches[i].interval(25);                          // interval in ms
  }

  delay(1000);

  Serial.begin(9600);

  if (!myDFPlayer.begin(Serial))
  {
    handleErrors(myDFPlayer.readType(), myDFPlayer.read());
    resetFunc();
  }

  digitalWrite(PIN_LED, HIGH);

  // Player init
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  updateVolume();
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
}

void loop()
{
  static unsigned long timer = millis();

  for (int i = 0; i < NB_SWITCHES; i++)
  {
    switches[i].update();
    if (switches[i].fell())
    {
      myDFPlayer.playMp3Folder(i + 1);
    }
  }

  if (millis() - timer > 100)
  {
    timer = millis();
    updateVolume();
  }

  if (myDFPlayer.available())
  {
    handleErrors(myDFPlayer.readType(), myDFPlayer.read());
  }
}

long mapWithHysteresis(long x, long in_min, long in_max, long out_min, long out_max, byte threshold)
{
  static long lastX = x;

  if (abs(lastX - x) > threshold)
  {
    lastX = x;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
  else
    return (lastX - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void updateVolume()
{
  static int volume = -1;
  int vol = analogRead(PIN_SOUND_VOL);
  vol = mapWithHysteresis(vol, 0, 1023, 0, 30, 15);
  if (vol != volume)
  {
    // digitalWrite(PIN_LED, vol & 0x01);
    volume = vol;
    myDFPlayer.volume(vol);
  }
}

void handleErrors(uint8_t type, int value)
{
  switch (type)
  {
  case TimeOut: // Time Out!
    flashLED(10);
    break;
  case WrongStack: // Stack Wrong!
    flashLED(11);
    break;
  case DFPlayerCardInserted: // Card Inserted!
    // flashLED(12);
    break;
  case DFPlayerCardRemoved: // Card Removed!
    flashLED(1);
    resetFunc();
    // flashLED(13);
    break;
  case DFPlayerCardOnline: // Card Online!
    // flashLED(14);
    break;
  case DFPlayerUSBInserted: // USB Inserted!
    flashLED(15);
    break;
  case DFPlayerUSBRemoved: // USB Removed!
    flashLED(16);
    break;
  case DFPlayerPlayFinished: // Play Finished!
    digitalWrite(PIN_LED, HIGH);
    // flashLED(17);
    break;
  case DFPlayerError: // DFPlayerError
    switch (value)
    {
    case Busy: // card not found
      flashLED(1);
      resetFunc();
      break;
    case Sleeping: // Sleeping
      flashLED(6);
      break;
    case SerialWrongStack: // Get Wrong Stack
      flashLED(5);
      break;
    case CheckSumNotMatch: // Check Sum Not Match
      flashLED(4);
      break;
    case FileIndexOut: // File Index Out of Bound
      flashLED(3);
      break;
    case FileMismatch: // Cannot Find File
      flashLED(2);
      break;
    case Advertise: // In Advertise
      flashLED(7);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void flashLED(int nbFlashs)
{
  digitalWrite(PIN_LED, LOW);
  delay(1000);
  for (int i = 0; i < nbFlashs; ++i)
  {
    digitalWrite(PIN_LED, HIGH);
    delay(250);
    digitalWrite(PIN_LED, LOW);
    delay(250);
  }
}