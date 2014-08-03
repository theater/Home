#include <DallasTemperature.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <dht.h>


#define PIN9 9  //AQ_Cooler1
#define PIN8 8 //AQ_Heater1
#define PIN7 7  //AQ_Lights1
#define ONE_WIRE_PIN 4  // Data wire is plugged into pin 4 on the Arduino
#define OW_REPORTING_INTERVAL 60000  // report 1-wire sensors data each minute to MQTT broker
#define DHT11_PIN 5  //DHT11

byte server[] = { 192,168,254,30 };
unsigned int now;
unsigned int sentTime;

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
dht DHT;

void gpio(int ledPin,boolean state) {
  if (state==1) {  
    digitalWrite(ledPin, HIGH);   // sets the LED on
  } else {
    digitalWrite(ledPin, LOW);    // sets the LED off
  }
} 

int MQTT_Connect () {
  if (!client.connected()) {
    if (client.connect("ArduinoNANO-AQ", "mqttuser", "MqTtUser")) {
      client.publish("Arduino","Arduino-AQ is UP");
      client.subscribe("AQ_Light1");
      client.subscribe("AQ_Heater1");
      client.subscribe("AQ_Cooler1");
      Serial.write("Connected to MQTT\n");
      return 0;
    } else {
       Serial.write("Error connecting to MQTT\n");
       delay(2000);
       return 1;
     }
  } 
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
      client.publish("AQ_Temp_Sensor1",strConvert);  // send it to MQTT broker
    dtostrf(sensors.getTempCByIndex(1),6,3,strConvert);  
      Serial.print("\nTemperature for LR-1wire is: "); // print living room temperature
      Serial.print(sensors.getTempCByIndex(1));        //to serial
      client.publish("LR_Temp_Sensor1",strConvert); // send to MQTT

    // DHT11 here...
    dtostrf(DHT.humidity,2,0,strConvert);
      Serial.print("\nHumidity DHT11 is: "); // print DHT11 humidity
      Serial.print(DHT.humidity);        //to serial
      client.publish("LR_Humidity_sensor1",strConvert); // send to MQTT
       dtostrf(DHT.temperature,2,0,strConvert);
      Serial.print("\nTemperature DHT11 is: "); // print DHT11 living room temperature
      Serial.print(DHT.temperature);        //to serial
      client.publish("LR_Temp_Sensor2",strConvert); // send to MQTT
  }
}/**/

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
       if (strPayload=="ON") { gpio(PIN7,0); } else { gpio(PIN7,1); }
  }
  if (String (topic) == "AQ_Heater1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(PIN8,0); } else { gpio(PIN8,1);; }
  }
  if (String (topic) == "AQ_Cooler1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(PIN9,0); } else { gpio(PIN9,1); }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(PIN7, OUTPUT);      // sets the digital pin as output
  pinMode(PIN8, OUTPUT);      // sets the digital pin as output    
  pinMode(PIN9, OUTPUT);      // sets the digital pin as output
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  IPAddress myIP(192,168,254,35);
  Ethernet.begin(mac,myIP);
  MQTT_Connect();
  sensors.begin();  //start the one-wire library
}

void loop()
{
  MQTT_Connect(); //connect if we loose MQTT server  
  OneWireGetAndReport();  
  client.loop();
}

