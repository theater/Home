
#include <SPI.h>
#include <RF24.h>
#include "printf.h"

#define RF_CS 9
#define RF_CSN 10
RF24 radio(RF_CS, RF_CSN);
const uint64_t pipes[2] = { 0xe7e7e7e7e7LL, 0xc2c2c2c2c2LL };
unsigned long counter=0;


struct sensor_struct{
  int sensor_id;
  float temp;
  float soil_temp;
  float humid;
  float pres;
};

void setup() {
  Serial.begin(115200);
  printf_begin();
  radio.begin();
  radio.setChannel(1);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);  
//  radio.setCRCLength(RF24_CRC_8);
//  radio.write_register(DYNPD,0);
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);    // note that our pipes are the same above, but that
  radio.openReadingPipe(1, pipes[0]); // they are flipped between rx and tx sides.
  radio.printDetails();
}

void loop() {
  radio.startListening();
  if (radio.available()) {

//    Serial.println("--------------------------------------------------------------------------------");
    long rx_data[8];  // we'll receive a 32 byte packet
    
    bool done = false;
    while (!done) {
      done = radio.read( &rx_data, sizeof(rx_data) );
      printf("Got payload @ %lu...\r ", millis());
    }
    
    // echo it back real fast
    radio.stopListening();
    delay(100);
    radio.write( &rx_data, sizeof(rx_data) );
    Serial.print("Sent response.\t");
    radio.startListening();

    // do stuff with the data we got.
    Serial.print(" Value: ");
    Serial.print(rx_data[0]);
    // print count of received
    Serial.print("\tCount received:\t");
    Serial.println(++counter);
    
  }  
  radio.stopListening();
  delay(50);
}


// https://gist.github.com/bryanthompson/ef4ecf24ad36410f077b 
