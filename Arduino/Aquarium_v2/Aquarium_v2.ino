#include <PubSubClient.h>
#include <DallasTemperature.h>
#include <UIPEthernet.h>
#include <OneWire.h>
#include <Timer.h>

//Digital pins 10,11,12,13 are used for ethernet shield ! ! !
#define ONE_WIRE_PIN 9  // Data wire is plugged into pin 4 on the Arduino
#define AQ_LIGHTS1_PIN 7  //AQ_Lights1
#define AQ_HEATER1_PIN 6 //AQ_Heater1
#define AQ_COOLER1_PIN 5  //AQ_Cooler1
#define RESET 3  // Reset button - pin 3 goes to GND
#define OW_REPORTING_INTERVAL 60000  // report 1-wire sensors data each minute to MQTT broker
#define AQ_MANUAL_INTERVAL 30000  // Aquarium manual function check interval will be on each 30 sec reset of ethernet on 5x30sec=150sec
#define AQ_Set_Temperature_manual 25 // Set temperature in manual mode (if no MQTT connection available)
#define RESET_INTERVAL 30000  // Reset counter will be changed each 1 mins, 3 times if not connected by then - reset the duino
#define MQTT_Client_ID "NANO-AQ"


byte server[] = { 192,168,254,30 };
//unsigned long now;
float AQ_Temperature;
float LR_Temperature;

Timer OW_GET_Timer;
Timer Reset_Timer;

int resetCounter=0;

EthernetClient ethClient;
PubSubClient MQTT_Client(server, 1883, callback, ethClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x35};
IPAddress myIP(192,168,254,35);  //IP address of Arduino

void gpio(int dPin,boolean state) {
  if (state==1) {  // regular logic due to SSR
    digitalWrite(dPin, HIGH);   // sets the pin ON
//     Serial.print("Digital Pin"); Serial.print(dPin);Serial.print(" set OFF\n");
    } else {
    digitalWrite(dPin, LOW);    // sets the pin OFF
//     Serial.print("Digital Pin"); Serial.print(dPin);Serial.print(" set ON\n");
  }
} 

int MQTT_Connect () {
 if (!MQTT_Client.connected()) {
    if (MQTT_Client.connect("ArduinoNANO-AQ", "mqttuser", "MqTtUser")) {
      MQTT_Client.publish("Arduino","Arduino-AQ is UP");
      MQTT_Client.subscribe("AQ_Light1");
      MQTT_Client.subscribe("AQ_Heater1");
      MQTT_Client.subscribe("AQ_Cooler1");
//      Serial.write("Connected to MQTT\n");
      return 1;
    } else {
//       Serial.write("Error connecting to MQTT\n");
       return 0;
     }
  } else return 1;
}

void OneWireGetAndReport() {
    //  Start reading sensors...
    sensors.requestTemperatures(); // Send the command to get temperatures from 1-wire
    char strConvert[10];
    // 1-wire HERE
    delay(10);
    AQ_Temperature=sensors.getTempCByIndex(0);
    LR_Temperature=sensors.getTempCByIndex(1);
    if(AQ_Temperature>-20) { 
      dtostrf(AQ_Temperature,6,3,strConvert);
      MQTT_Client.publish("AQ_Temp_Sensor1",strConvert);  // send it to MQTT broker
    } 
    if(LR_Temperature>-20) { 
    dtostrf(LR_Temperature,6,3,strConvert);  
      MQTT_Client.publish("LR_Temp_Sensor1",strConvert); // send to MQTT
  }
}/**/
  
void ManualAQLogics()  {
    AQ_Temperature=sensors.getTempCByIndex(0);
//    Serial.write("Manual logics started...\n");
    if (AQ_Temperature>AQ_Set_Temperature_manual+0.5) {
        gpio(AQ_COOLER1_PIN,1);
        gpio(AQ_HEATER1_PIN,0);
    }  
    if (AQ_Temperature<AQ_Set_Temperature_manual-0.5) { 
        gpio(AQ_HEATER1_PIN,1);
        gpio(AQ_COOLER1_PIN,0);
    }
    delay(10);
}
void callback(char* topic, byte* payload, unsigned int length) {
// ##### this function is the main part that triggers event based on the MQTT received messages. All the important input stuff is here... 

// ####### byte* to String transformation of payload starts here
  char cPayload[10];
  for (int i=0; i<=length; i++) {
    cPayload[i]=(char)payload[i];
  }
  cPayload[length]='\0';
  String strPayload = String(cPayload);
// ####### byte* to String transformation ends here  

// #### Real work/Logics start here. Using IFs to destinguish for the different MQTT subscriptions/relays (unfortunalte string not allowed in switch operator) :(    
  if (String (topic) == "AQ_Light1" ) {
//       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(AQ_LIGHTS1_PIN,1); } else { gpio(AQ_LIGHTS1_PIN,0); }
  }
  if (String (topic) == "AQ_Heater1" ) {
//       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(AQ_HEATER1_PIN,1); } else { gpio(AQ_HEATER1_PIN,0);; }
  }
  if (String (topic) == "AQ_Cooler1" ) {
//       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(AQ_COOLER1_PIN,1); } else { gpio(AQ_COOLER1_PIN,0); }
  }/**/
}

void ResetFunction() {
  if (!MQTT_Connect()){
    ManualAQLogics();
    resetCounter++;
  } else {
    resetCounter=0;
  }
  if(resetCounter>=3){
       Enc28J60.init(mac);
  }
  if(resetCounter>5) {
    gpio(RESET,LOW);
  }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}



void setup()
{
  digitalWrite(RESET, HIGH);
  Serial.begin(115200);
  pinMode(AQ_LIGHTS1_PIN, OUTPUT);      // sets the digital pin as output
  pinMode(AQ_HEATER1_PIN, OUTPUT);      // sets the digital pin as output    
  pinMode(AQ_COOLER1_PIN, OUTPUT);      // sets the digital pin as output
  gpio(AQ_LIGHTS1_PIN,LOW);   // Initialize digital AQ_LIGHTS1_PIN with OFF
  gpio(AQ_HEATER1_PIN,LOW);   // Initialize digital AQ_HEATER1_PIN with OFF
  gpio(AQ_COOLER1_PIN,LOW);   // Initialize digital AQ_COOLER1_PIN with OFF


  Ethernet.begin(mac,myIP);
  sensors.begin();  //start the one-wire library
  delay(15);
  MQTT_Connect();
  delay(15);
// Timer callbacks creation
  OW_GET_Timer.every(OW_REPORTING_INTERVAL,OneWireGetAndReport);
  Reset_Timer.every(RESET_INTERVAL,ResetFunction);
  OneWireGetAndReport();
}



void loop()
{
  OW_GET_Timer.update();
  Reset_Timer.update();
  if (MQTT_Connect()){  
    MQTT_Client.loop(); 
  } /**/
}
