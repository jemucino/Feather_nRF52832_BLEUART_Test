# Test Code for Nordic UART Service on Adafruit Feather nRF52 Bluefruit

This repository contains test code for the Nordic UART Service on the Adafruit Feather NRF52 Bluefruit. It is based on the BLE Uart example from the [Adafruit_nRF52_Arduino repo](https://github.com/adafruit/Adafruit_nRF52_Arduino).

## Setup

Follow [this tutorial from Adafruit](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/arduino-bsp-setup) to set up the Arduino IDE to build and deploy this code.

## Functionality

Data sent to the device over Nordic UART Service is parsed and written out to serial (at 115200 baud). Everytime a new packet is received, the millis() timestamp is written to serial as well.

## Packet structure

The data packets can be of two types: Button and SeekBar.

### Button

The Button data is an Int16 value sent as a byte array (little-endian) prefixed by 0x2142 (i.e., "!B") and postfixed by a CRC byte.

### SeekBar

The SeekBar data is an Int32 value sent as a byte array (little-endian) prefixed by 0x2153 (i.e., "!S") and postfixed by a CRC byte.

## CRC computation

The CRC byte is computed simply by taking the sum of all bytes in the payload data (i.e., prefix and Button/SeekBar value) then doing a bitwise negation on the least significant byte of the result. The following code shows an example implementation for checking the CRC byte.

```cpp
  // check checksum!
  uint8_t xsum = 0;
  uint8_t checksum = packetbuffer[replyidx-1];

  for (uint8_t i=0; i<PACKET_LEN; i++) {
    xsum += packetbuffer[i];
  }
  xsum = ~xsum;

  // Throw an error message if the checksum's don't match
  if (xsum != checksum)
  {
    Serial.print("Checksum mismatch in packet : ");
    printHex(packetbuffer, replyidx+1);
    return 0;
  }
```

Where _PACKET___LEN_ is in bytes and does not include the CRC.
