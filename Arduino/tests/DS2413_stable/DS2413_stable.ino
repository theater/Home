#include <OneWire.h>

#define DS2413_ONEWIRE_PIN  (2)

#define DS2413_FAMILY_ID    0x85
#define DS2413_ACCESS_READ  0xF5
#define DS2413_ACCESS_WRITE 0x5A
#define DS2413_ACK_SUCCESS  0xAA
#define DS2413_ACK_ERROR    0xFF

OneWire oneWire(DS2413_ONEWIRE_PIN);
uint8_t address[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void printBytes(uint8_t* addr, uint8_t count, bool newline=0) 
{
  for (uint8_t i = 0; i < count; i++) 
  {
    Serial.print(addr[i]>>4, HEX);
    Serial.print(addr[i]&0x0f, HEX);
    Serial.print(" ");
  }
  if (newline)
  {
    Serial.println();
  }
}

byte read(void)
{		
  bool ok = false;
  uint8_t results;
  byte IOA = 0x1;
  byte IOB = 0x2;
  byte IOA_result;
  byte IOB_result;
  oneWire.reset();
  oneWire.select(address);
  oneWire.write(DS2413_ACCESS_READ);

  results = oneWire.read();                 /* Get the register results   */
  IOA_result=IOA&results;
  IOB_result=IOB&results;
  oneWire.reset();


  return IOA_result+IOB_result;
}

bool write(uint8_t state)
{
  uint8_t ack = 0;
  
  /* Top six bits must '1' */
  state |= 0xFC;
  
  oneWire.reset();
  oneWire.select(address);
  oneWire.write(DS2413_ACCESS_WRITE);
  oneWire.write(state);
  oneWire.write(~state);                    /* Invert data and resend     */    
  ack = oneWire.read();                     /* 0xAA=success, 0xFF=failure */  
  if (ack == DS2413_ACK_SUCCESS)
  {
    oneWire.read();                          /* Read the status byte      */
  }
  oneWire.reset();
    
  return (ack == DS2413_ACK_SUCCESS ? true : false);
}

void setup(void) 
{
  Serial.begin(9600);  
  
  Serial.println(F("Looking for a DS2413 on the bus"));
  
  /* Try to find a device on the bus */
  oneWire.reset_search();
  delay(250);
  if (!oneWire.search(address)) 
  {
    printBytes(address, 8);
    Serial.println(F("No device found on the bus!"));
    oneWire.reset_search();
    while(1);
  }
  
  /* Check the CRC in the device address */
  if (OneWire::crc8(address, 7) != address[7]) 
  {
    Serial.println(F("Invalid CRC!"));
    while(1);
  }
  
  /* Make sure we have a DS2413 */
  if (address[0] != DS2413_FAMILY_ID) 
  {
    printBytes(address, 8);
    Serial.println(F(" is not a DS2413!"));
    while(1);
  }
  
  Serial.print(F("Found a DS2413: "));
  printBytes(address, 8);
  Serial.println(F(""));
}

void loop(void) 
{
  /* Read */

byte state = read();
  if (state == -1)
    Serial.println(F("Failed reading the DS2413"));
switch (state) {
  case 0: Serial.println(F("IOA:OFF, IOB:OFF"));
  case 1: Serial.println(F("IOA:ON, IOB:OFF"));
  case 2: Serial.println(F("IOA:OFF, IOB:ON"));
  case 3: Serial.println(F("IOA:ON, IOB:ON"));
}  
  /* Write 
  bool ok = false;
  ok = write(0x3);
  if (!ok) Serial.println(F("Wire failed"));
  delay(1000);
  ok = write(0x0);
  if (!ok) Serial.println(F("Wire failed")); */
  delay(1000);
}
