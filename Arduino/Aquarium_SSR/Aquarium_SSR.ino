#include <DallasTemperature.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <dht.h>

//Digital pins 10,11,12,13 are used for ethernet shield ! ! !
#define ONE_WIRE_PIN 9  // Data wire is plugged into pin 4 on the Arduino
#define DHT11_PIN 8  //DHT11
#define AQ_LIGHTS1_PIN 7  //AQ_Lights1
#define AQ_HEATER1_PIN 6 //AQ_Heater1
#define AQ_COOLER1_PIN 5  //AQ_Cooler1
#define OW_REPORTING_INTERVAL 60000  // report 1-wire sensors data each minute to MQTT broker
#define AQ_MANUAL_INTERVAL 900000  // Aquarium manual function check interval will be on each 15 min
#define AQ_Set_Temperature_manual 25 // Set temperature in manual mode (if no MQTT connection available)

byte server[] = { 192,168,254,30 };
unsigned long now;
unsigned long sentTime;
float AQ_Temperature;
unsigned long aq_checkagain;
unsigned long presence_counter=0;
unsigned long presence_sentTime=0;
unsigned long keepalivetime=0; 

uint8_t mac[6] = {0x21,0x21,0x02,0x03,0x04,0x05};
IPAddress myIP(192,168,254,35);  //IP address of Arduino

EthernetClient ethClient;
PubSubClient MQTT_Client(server, 1883, callback, ethClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
dht DHT;

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
    MQTT_Client.disconnect();
    if (MQTT_Client.connect("ArduinoNANO-AQ")) {
      MQTT_Client.publish("Arduino","Arduino-AQ is UP");
      MQTT_Client.subscribe("AQ_Light1");
      MQTT_Client.subscribe("AQ_Heater1");
      MQTT_Client.subscribe("AQ_Cooler1");
//      Serial.println("Connected to MQTT\n");
      return 1;
    } else {
//       Serial.println("Error connecting to MQTT\n");
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
    if(sensors.getTempCByIndex(0)>-20) { 
        dtostrf(sensors.getTempCByIndex(0),6,3,strConvert);
//      Serial.println("Temperature for AQ temp Sensor is: ");   // print AQ temperature
//      Serial.println(sensors.getTempCByIndex(0));                  // to serial
        MQTT_Client.publish("AQ_Temp_Sensor1",strConvert);  // send it to MQTT broker
    }
    if(sensors.getTempCByIndex(1)>-20) { 
    dtostrf(sensors.getTempCByIndex(1),6,3,strConvert);  
//      Serial.println("\nTemperature for LR-1wire is: "); // print living room temperature
//      Serial.println(sensors.getTempCByIndex(1));        //to serial
      MQTT_Client.publish("LR_Temp_Sensor1",strConvert); // send to MQTT
    }
    // DHT11 here...
    if(DHT.humidity>0) {
    dtostrf(DHT.humidity,2,0,strConvert);
//      Serial.println("\nHumidity DHT11 is: "); // print DHT11 humidity
//      Serial.println(DHT.humidity);        //to serial
      MQTT_Client.publish("LR_Humidity_sensor1",strConvert); // send to MQTT
    }
    if(DHT.temperature>-20) {
       dtostrf(DHT.temperature,4,2,strConvert);
//      Serial.println("\nTemperature DHT11 is: "); // print DHT11 living room temperature
//      Serial.println(DHT.temperature);        //to serial
      MQTT_Client.publish("LR_Temp_Sensor2",strConvert); // send to MQTT
 //     Serial.println("FreeRAM:");Serial.println(freeRam());
    }
  }
}/**/
  
void ManualAQLogics()  {
  uint8_t mac[6] = {0x21,0x21,0x02,0x03,0x04,0x05};
  IPAddress myIP(192,168,254,35);  //IP address of Arduino
  
  now = millis();
  Serial.write("Starting manual AQ logics\n"); 
  sensors.requestTemperatures(); // Send the command to get temperatures from 1-wire
  if ((now - aq_checkagain)>AQ_MANUAL_INTERVAL){
    aq_checkagain=now;    
    AQ_Temperature=sensors.getTempCByIndex(0);
    if (AQ_Temperature>AQ_Set_Temperature_manual) {
//        Serial.println("Starting AQ Cooler");
        gpio(AQ_COOLER1_PIN,1);
        Ethernet.begin(mac,myIP);
    }  
    if (AQ_Temperature<AQ_Set_Temperature_manual-1) { 
//        Serial.println("Starting AQ Heater");
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
  }
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
  Serial.begin(115200);
  pinMode(AQ_LIGHTS1_PIN, OUTPUT);      // sets the digital pin as output
  pinMode(AQ_HEATER1_PIN, OUTPUT);      // sets the digital pin as output    
  pinMode(AQ_COOLER1_PIN, OUTPUT);      // sets the digital pin as output
  gpio(AQ_LIGHTS1_PIN,0);   // Initialize digital AQ_LIGHTS1_PIN with OFF
  gpio(AQ_HEATER1_PIN,0);   // Initialize digital AQ_HEATER1_PIN with OFF
  gpio(AQ_COOLER1_PIN,0);   // Initialize digital AQ_COOLER1_PIN with OFF
//  Serial.println("Ethernet begin, setting IP and MAC");
  Ethernet.begin(mac,myIP);
  MQTT_Connect ();
//  Serial.println("Sensors begin initializing sensor busses");
  sensors.begin();  //start the one-wire library
}

void loop()
{
 if (!MQTT_Connect()){
    ManualAQLogics();  
    delay(AQ_MANUAL_INTERVAL); // if not connected wait 30sec before next attempt
    ethClient.stop();
    Ethernet.begin(mac,myIP);
  } else {
    OneWireGetAndReport();  
    MQTT_Client.loop();
  }
  Ethernet.maintain();
} 
