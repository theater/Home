//#include <DallasTemperature.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
//#include <OneWire.h>
//#include <dht.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define PUBLISH_INTERVAL 60000  // Publish data each minute to MQTT broker
#define MANUAL_INTERVAL 300000  // Manual function check interval will be on each 15 min
long SEALEVEL_PRESSURE=101300;  // get this information from local weather stations eventually
byte server[] = { 192,168,254,30 };
unsigned long now;
unsigned long sentTime=0;

uint8_t mac[6] = {0x36,0x36,0x36,0x36,0x36,0x36};
IPAddress myIP(192,168,254,36);  //IP address of Arduino

Adafruit_BMP085 bmp;
EthernetClient ethClient;
PubSubClient MQTT_Client(server, 1883, callback, ethClient);


int MQTT_Connect () {
  if (!MQTT_Client.connected()) {
    MQTT_Client.disconnect();
    if (MQTT_Client.connect("ArduinoNANO-Weather")) {
      MQTT_Client.publish("Arduino","Arduino-Weather is UP");
      MQTT_Client.subscribe("Weather_pressure_set");
      Serial.println("Connected to MQTT\n");
      return 1;
    } else {
       Serial.println("Error connecting to MQTT\n");
       return 0;
     }
  } else return 1;
}

void Publish_Data () {
  now = millis();
  if ((now - sentTime)>=PUBLISH_INTERVAL) {  
    Serial.println("Entered publishing...");
    sentTime=now;
    bmp.begin();
    int Altitude=int(bmp.readAltitude(SEALEVEL_PRESSURE));
    int Pressure=bmp.readPressure()/100;
    int Temperature=bmp.readTemperature();
    char strConvert[10];
    
    if (Pressure>900 && Pressure < 1100) {    String tmp1=String (Pressure);    
      tmp1.toCharArray(strConvert,10);
      MQTT_Client.publish("Weather_pressure",strConvert); // send to MQTT
    }
    if (Altitude<1000 && Altitude > 0){   String tmp2=String(Altitude);    
      tmp2.toCharArray(strConvert,10);
      MQTT_Client.publish("Weather_altitude",strConvert); // send to MQTT
    }  
    if (Altitude<1000 && Altitude > 0){   String tmp3=String(Temperature);    
      tmp3.toCharArray(strConvert,10);
      MQTT_Client.publish("Weather_temperature",strConvert); // send to MQTT
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
  if (String (topic) == "Weather_pressure_set" ) {
    String strPayload = String(cPayload);
  }
// ####### byte* to String transformation ends here  
//  Serial.print("\n Inside callback:");
//  Serial.println(cPayload);
  SEALEVEL_PRESSURE=atol(cPayload)*100;
// #### Real work/Logics start here. Using IFs to destinguish for the different MQTT subscriptions/relays (unfortunalte string not allowed in switch operator) :(    
/**/
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
  Serial.begin(115200);
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
   } else {
     Serial.println("BMP180 successfully initialized!");
   }
  Ethernet.begin(mac,myIP);
  delay(500);
//  MQTT_Connect();
MQTT_Client.connect("ArduinoNANO-Weather");
MQTT_Client.subscribe("Weather_pressure_set");
}

void loop()
{
/* if (!MQTT_Connect()){
    delay(MANUAL_INTERVAL); // if not connected wait 30sec before next attempt
    ethClient.stop();
    Ethernet.begin(mac,myIP);
  } else {
    Publish_Data();
    MQTT_Client.loop();
  }/**/
  Publish_Data();
  MQTT_Client.loop();
  Ethernet.maintain();/**/
} 
