#include <UIPEthernet.h>
#include <PubSubClient.h>

#define MANUAL_INTERVAL 90000  // Manual function check interval will be on each 15 min

byte server[] = { 192,168,254,30 };  // IP Address of the MQTT server

uint8_t mac[6] = {0x36,0x22,0x02,0x03,0x04,0x05};
IPAddress myIP(192,168,254,35);  //IP address of Arduino

EthernetClient ethClient;
PubSubClient MQTT_Client(server, 1883, callback, ethClient);

int MQTT_Connect () {
  if (!MQTT_Client.connected()) {
    if (MQTT_Client.connect("ArduinoNANO-TEST")) {
      MQTT_Client.publish("Arduino","Arduino-TEST is UP");
//      Serial.println("Connected to MQTT\n");
      return 1;
    } else {
//       Serial.println("Error connecting to MQTT\n");
       return 0;
     }
  } else return 1;
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
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup()
{
  Serial.begin(115200);
  Ethernet.begin(mac,myIP);
  MQTT_Connect ();
//  Serial.println("Sensors begin initializing sensor busses");
}

void loop()
{
 if (!MQTT_Connect()){
    delay(MANUAL_INTERVAL); // if not connected wait 30sec before next attempt
    ethClient.stop();
    Ethernet.begin(mac,myIP);
  } else {
    MQTT_Client.loop();
  }
  Ethernet.maintain();
} 
