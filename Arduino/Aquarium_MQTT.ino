#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <OneWire.h>

int Pin9=9;

byte server[] = { 192,168,254,30 };
int num_received=0;
EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

void gpio(int ledPin,boolean state) {
  //  Simple function that triggers digital pins ON and OFF.
  if (state==1) {  
    Serial.println("PIN "+String(ledPin)+" ON");
    digitalWrite(ledPin, HIGH);   // sets the LED on
  } else {
    Serial.println("PIN "+String(ledPin)+" OFF");
    digitalWrite(ledPin, LOW);    // sets the LED off
  }
}

int MQTT_Connect () {
  if (!client.connected()) {
    if (client.connect("ArduinoNANO-AQ", "mqttuser", "MqTtUser")) {
      client.publish("arduino","Arduino-AQ is UP");
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
  if (String (topic) == "AQ_Light1" ) {
       Serial.println("Message arrived topic: " + String(topic));
        
  }
  if (String (topic) == "AQ_Heater1" ) {
       Serial.println("Message arrived topic: " + String(topic));
  }
  if (String (topic) == "AQ_Cooler1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(Pin9,1); } else { gpio(Pin9,0); }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(Pin9, OUTPUT);      // sets the digital pin as output
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  IPAddress myIP(192,168,254,35);
  Ethernet.begin(mac,myIP);
  MQTT_Connect();
}

void loop()
{
  client.loop();
  MQTT_Connect(); //connect if we loose MQTT serve
}

