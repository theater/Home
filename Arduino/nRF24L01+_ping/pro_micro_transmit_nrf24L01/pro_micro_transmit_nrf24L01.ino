#include <SPI.h>
#include <RF24.h>
#include "printf.h" 

#define LED 2
#define RF_CS 9
#define RF_CSN 10
RF24 radio(RF_CS, RF_CSN);
const uint64_t pipes[2] = { 0xe7e7e7e7e7LL, 0xc2c2c2c2c2LL };
long packets_lost=0;
long packets_sent=0;

void setup() {
  Serial.begin(115200);
  printf_begin();
  pinMode(LED, OUTPUT);
  radio.begin();
  radio.setChannel(1);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);  
//  radio.setCRCLength(RF24_CRC_8);
//  radio.write_register(DYNPD,0);
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  radio.startListening();
  radio.printDetails();
}

void loop() {
  unsigned long time = millis();

  long data[8];  // we'll transmit a 32 byte packet
  data[0] = millis();      // our first byte in the pcaket will just be the number 99.
  
  Serial.print("Sending the following data to radio:");Serial.print(data[0]);
  // transmit the data
  radio.stopListening();
  radio.write( &data, sizeof(data) );
  radio.startListening();
  packets_sent++;   // count sent packets
  // listen for acknowledgement from the receiver
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while (!radio.available() && ! timeout)
    if (millis() - started_waiting_at > 1000 )
      timeout = true;

  if (timeout){
    Serial.print("=====> Failed, response timed out. Packets lost: ");
    Serial.print(++packets_lost);  //count packets lost
    Serial.print("/");
    Serial.println(packets_sent);  
  } else { 
    // the receiver is just going to spit the data back
    radio.read( &data, sizeof(data) );
    digitalWrite(LED, HIGH);
    delay(100);  // light up the LED for 100ms if it worked.
    digitalWrite(LED, LOW);
    Serial.print("=====> Got response, round trip delay: ");
    Serial.println(millis() - started_waiting_at);
  }

  delay(1000); // wait a second and do it again. 
}

// https://gist.github.com/bryanthompson/ef4ecf24ad36410f077b 
