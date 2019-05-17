#include <string.h>
#include <Arduino.h>
#include <bluefruit.h>


#define PACKET_BUTTON_LEN               (5)
#define PACKET_SEEKBAR_LEN              (5)

//    READ_BUFSIZE            Size of the read buffer for incoming packets
#define READ_BUFSIZE                    (20)


/* Buffer to hold incoming characters */
uint8_t packetbuffer[READ_BUFSIZE+1];

/**************************************************************************/
/*!
    @brief  Casts the four bytes at the specified address to a float
*/
/**************************************************************************/
float parsefloat(uint8_t *buffer)
{
  float f;
  memcpy(&f, buffer, 4);
  return f;
}

/**************************************************************************/
/*!
    @brief  Casts the four bytes at the specified address to an int
*/
/**************************************************************************/
int parseint32(uint8_t *buffer)
{
  int i;
  memset(&i, 0, sizeof(int));
  memcpy(&i, buffer, 4);
  return i;
}

/**************************************************************************/
/*!
    @brief  Casts the four bytes at the specified address to an int
*/
/**************************************************************************/
int parseint16(uint8_t *buffer)
{
  int i;
  memset(&i, 0, sizeof(int));
  memcpy(&i, buffer, 2);
  return i;
}

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters
    @param  data      Pointer to the byte data
    @param  numBytes  Data length in bytes
*/
/**************************************************************************/
void printHex(const uint8_t * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
    {
      Serial.print(F("0"));
      Serial.print(data[szPos] & 0xf, HEX);
    }
    else
    {
      Serial.print(data[szPos] & 0xff, HEX);
    }
    // Add a trailing space if appropriate
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(F(" "));
    }
  }
  Serial.println();
}

/**************************************************************************/
/*!
    @brief  Waits for incoming data and parses it
*/
/**************************************************************************/
uint16_t origtimeout, replyidx;

uint8_t readPacket(BLEUart *ble_uart, uint16_t timeout)
{
  origtimeout = timeout, replyidx = 0;
  memset(packetbuffer, 0, READ_BUFSIZE);

  while (ble_uart->available()  && replyidx < PACKET_SEEKBAR_LEN) {
    char c =  ble_uart->read();
    if (c == '!') {
      replyidx = 0;
      Serial.println(millis());
    }
    packetbuffer[replyidx] = c;
    replyidx++;
    timeout = origtimeout;
  }

  while (timeout--) {
    if (replyidx >= 20) break;
    if ((packetbuffer[1] == 'B') && (replyidx == PACKET_BUTTON_LEN)) break;
    if ((packetbuffer[1] == 'S') && (replyidx == PACKET_SEEKBAR_LEN))break;
    if (timeout == 0) break;
  }

  packetbuffer[replyidx] = 0;  // null term

  if (!replyidx)  // no data or timeout
    return 0;
  if (packetbuffer[0] != '!')  // doesn't start with '!' packet beginning
    return 0;

  // check checksum!
  uint8_t xsum = 0;
  uint8_t checksum = packetbuffer[replyidx-1];

  for (uint8_t i=0; i<replyidx-1; i++) {
    xsum += packetbuffer[i];
  }
  xsum = ~xsum;

  // Throw an error message if the checksum's don't match
  if (xsum != checksum)
  {
    Serial.print("Checksum mismatch in packet : ");
    printHex(packetbuffer, replyidx);
    Serial.print("Expected : ");
    Serial.println(xsum, HEX);
    return 0;
  }

  // checksum passed!
  return replyidx;
}
