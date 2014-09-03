#include <DallasTemperature.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <dht.h>

#define ONE_WIRE_PIN 9  // Data wire is plugged into pin 4 on the Arduino
#define DHT11_PIN 8  //DHT11
#define PRESENCE_PIN 7  // PIR
#define AQ_COOLER1_PIN 6  //AQ_Cooler1
#define AQ_HEATER1_PIN 5 //AQ_Heater1
#define AQ_LIGHTS1_PIN 4  //AQ_Lights1
#define AQ_SPARE_PIN 3  // FUTURE Digital use
#define OW_REPORTING_INTERVAL 60000  // report 1-wire sensors data each minute to MQTT broker
#define AQ_MANUAL_INTERVAL 900000  // Aquarium manual function check interval will be on each 15 min
#define AQ_Set_Temperature_manual 25 // Set temperature in manual mode (if no MQTT connection available)
#define PRESENCE_COUNTER_RESET 120000  // reset presence counter each 10 mins to be implemented

byte server[] = { 192,168,254,30 };
unsigned long now;
unsigned long sentTime;
float AQ_Temperature;
unsigned long aq_checkagain;
unsigned long presence_counter=0;
unsigned long presence_sentTime=0;

EthernetClient ethClient;
PubSubClient MQTT_Client(server, 1883, callback, ethClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
dht DHT;

void gpio(int dPin,boolean state) {
  if (state==0) {  //(inverse logic due to relays
    digitalWrite(dPin, HIGH);   // sets the pin ON
     Serial.print("Digital Pin"); Serial.print(dPin);Serial.print(" set OFF\n");
    } else {
    digitalWrite(dPin, LOW);    // sets the pin OFF
     Serial.print("Digital Pin"); Serial.print(dPin);Serial.print(" set ON\n");
  }
} 

int MQTT_Connect () {
  if (!MQTT_Client.connected()) {
    if (MQTT_Client.connect("ArduinoNANO-AQ", "mqttuser", "MqTtUser")) {
      MQTT_Client.publish("Arduino","Arduino-AQ is UP");
      MQTT_Client.subscribe("AQ_Light1");
      MQTT_Client.subscribe("AQ_Heater1");
      MQTT_Client.subscribe("AQ_Cooler1");
      Serial.print("Connected to MQTT\n");
      return 1;
    } else {
       Serial.print("Error connecting to MQTT\n");
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

    // DHT11 here...
    dtostrf(DHT.humidity,2,0,strConvert);
      Serial.print("\nHumidity DHT11 is: "); // print DHT11 humidity
      Serial.print(DHT.humidity);        //to serial
      MQTT_Client.publish("LR_Humidity_sensor1",strConvert); // send to MQTT
       dtostrf(DHT.temperature,4,2,strConvert);
      Serial.print("\nTemperature DHT11 is: "); // print DHT11 living room temperature
      Serial.print(DHT.temperature);        //to serial
      MQTT_Client.publish("LR_Temp_Sensor2",strConvert); // send to MQTT
// logics about presence here...
 /*     if ((now - presence_sentTime)>PRESENCE_COUNTER_RESET) {
        presence_sentTime=now;
        if (presence_counter>0) { 
            MQTT_Client.publish("LR_Presense_1","ON");
            Serial.print("\nPresence detected "); Serial.print(presence_counter); Serial.print(" times\n");
            presence_counter=0;
          } else { 
            MQTT_Client.publish("LR_Presense_1","OFF");
            Serial.print("\nNo presence detected in the last minute... "); // print presence detector state
          }
      } /**/
  }
}/**/
/*
int PresenceCheck() {
  if (digitalRead(PRESENCE_PIN)){
    presence_counter++;
    return 1;
  } else {
    return 0;
  }
} /**/ 
  
void ManualAQLogics()  {
  now = millis();
  Serial.write("Starting manual AQ logics\n"); 
  sensors.requestTemperatures(); // Send the command to get temperatures from 1-wire
  if ((now - aq_checkagain)>AQ_MANUAL_INTERVAL){
    aq_checkagain=now;    
    AQ_Temperature=sensors.getTempCByIndex(0);
    if (AQ_Temperature>AQ_Set_Temperature_manual) {
        Serial.print("Starting AQ Cooler");
        gpio(AQ_COOLER1_PIN,1);
    }  
    if (AQ_Temperature<AQ_Set_Temperature_manual-1) { 
        Serial.print("Starting AQ Heater");
        gpio(AQ_HEATER1_PIN,1);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
// ##### this function is the main part that triggers event based on the MQTT received messages. All the important input stuff is here... 

// ####### byte* to String transformation of payload starts here
  char cPayload[30];
  for (int i=0; i<=length; i++) {
    cPayload[i]=(char)payload[i];
  }
  cPayload[length]='\0';
  String strPayload = String(cPayload);
// ####### byte* to String transformation ends here  

// #### Real work/Logics start here. Using IFs to destinguish for the different MQTT subscriptions/relays (unfortunalte string not allowed in switch operator) :(    
// #### Logic is inverted cause relays expect 0 in order to connect, i.e. pindown is ON
  if (String (topic) == "AQ_Light1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(AQ_LIGHTS1_PIN,1); } else { gpio(AQ_LIGHTS1_PIN,0); }
  }
  if (String (topic) == "AQ_Heater1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(AQ_HEATER1_PIN,1); } else { gpio(AQ_HEATER1_PIN,0);; }
  }
  if (String (topic) == "AQ_Cooler1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(AQ_COOLER1_PIN,1); } else { gpio(AQ_COOLER1_PIN,0); }
  }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
  uint8_t mac[6] = {0x21,0x21,0x02,0x03,0x04,0x05};
  IPAddress myIP(192,168,254,35);  //IP address of Arduino
  Serial.begin(9600);
  pinMode(PRESENCE_PIN,INPUT);   // Presence PIN initialized as input for PIR
  digitalWrite(PRESENCE_PIN, LOW);
  pinMode(AQ_SPARE_PIN, OUTPUT);      // sets the digital pin as output
  pinMode(AQ_LIGHTS1_PIN, OUTPUT);      // sets the digital pin as output
  pinMode(AQ_HEATER1_PIN, OUTPUT);      // sets the digital pin as output    
  pinMode(AQ_COOLER1_PIN, OUTPUT);      // sets the digital pin as output
  gpio(AQ_SPARE_PIN,0);   // Initialize digital SPARE PING with OFF
  gpio(AQ_LIGHTS1_PIN,0);   // Initialize digital AQ_LIGHTS1_PIN with OFF
  gpio(AQ_HEATER1_PIN,0);   // Initialize digital AQ_HEATER1_PIN with OFF
  gpio(AQ_COOLER1_PIN,0);   // Initialize digital AQ_COOLER1_PIN with OFF
  Ethernet.begin(mac,myIP);
  sensors.begin();  //start the one-wire library
}

void loop()
{
//  Serial.println(freeRam ());
  if (MQTT_Connect()){
    MQTT_Client.loop();
    OneWireGetAndReport();  
//    PresenceCheck();
  } else { 
    ManualAQLogics();
  }
} 
