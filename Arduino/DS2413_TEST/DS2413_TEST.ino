#include <OneWire.h>
#include <PubSubClient.h>
#include <Ethernet.h>

#define ONE_WIRE_PIN 4  // Data wire is plugged into pin 4 on the Arduino
#define REPORTING_INTERVAL 60000 // each minute post data to MQTT

unsigned long now;
unsigned long sentTime;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

// Network settings
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte mask[] = {
  255,255,255,0 };
byte dns_server[] = {
  10,17,220,72 };
byte gateway[] = {
  10,55,65,1 };

IPAddress ip(10,55,65,222);

void gpio(int dPin,boolean state) {
  if (state==0) {  //(inverse logic due to relays
    digitalWrite(dPin, HIGH);   // sets the pin ON
     Serial.print("Digital Pin"); Serial.print(dPin);Serial.print(" set ON");
    } else {
    digitalWrite(dPin, LOW);    // sets the pin OFF
     Serial.print("Digital Pin"); Serial.print(dPin);Serial.print(" set OFF");
  }
} 

int MQTT_Connect () {
  if (!MQTT_Client.connected()) {
    if (MQTT_Client.connect("ArduinoNANO-AQ", "mqttuser", "MqTtUser")) {
      MQTT_Client.publish("Arduino","Arduino-AQ is UP");
      MQTT_Client.subscribe("AQ_Light1");
      MQTT_Client.subscribe("AQ_Heater1");
      MQTT_Client.subscribe("AQ_Cooler1");
      Serial.write("Connected to MQTT\n");
      return 1;
    } else {
       Serial.write("Error connecting to MQTT\n");
       return 0;
     }
  } else return 1;
}

void OneWireGetAndReport() {
  now = millis();
  if ((now - sentTime)>OW_REPORTING_INTERVAL) {  
    sentTime = now;
    //  Start reading sensors...
    sensors.requestTemperatures(); // Send the command to get temperatures from 1-wire
    DHT.read11(DHT11_PIN);  //read info from DHT11
    //do transformations and post to MQTT
    char strConvert[10];
    // 1-wire HERE
    dtostrf(sensors.getTempCByIndex(0),6,3,strConvert);
      Serial.print("Temperature for AQ temp Sensor is: ");   // print AQ temperature
      Serial.print(sensors.getTempCByIndex(0));                  // to serial
      MQTT_Client.publish("AQ_Temp_Sensor1",strConvert);  // send it to MQTT broker
    dtostrf(sensors.getTempCByIndex(1),6,3,strConvert);  
      Serial.print("\nTemperature for LR-1wire is: "); // print living room temperature
      Serial.print(sensors.getTempCByIndex(1));        //to serial
      MQTT_Client.publish("LR_Temp_Sensor1",strConvert); // send to MQTT
  }
}/**/

void LedLightUp () {
  if 
}

void callback(char* topic, byte* payload, unsigned int length) {
}

void setup()
{
  Serial.begin(9600);
  pinMode(PIN7, OUTPUT);      // sets the digital pin as output
  pinMode(PIN8, OUTPUT);      // sets the digital pin as output    
  pinMode(PIN9, OUTPUT);      // sets the digital pin as output
  gpio(PIN7,0);   // Initialize digital pin 7 with OFF
  gpio(PIN8,0);   // Initialize digital pin 8 with OFF
  gpio(PIN9,0);   // Initialize digital pin 9 with OF
  Ethernet.begin(mac, ip, dns_server, gateway, mask);   
  sensors.begin();  //start the one-wire library
}

void loop()
{
  if (!MQTT_Connect()){
    OneWireGetAndReport();  
    delay(30000); // if not connected wait 30sec before next attempt
  } else {
    OneWireGetAndReport();  
    MQTT_Client.loop();
  }
}
