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
#define AQ_MANUAL_INTERVAL 900000  // Aquarium manual function check interval will be on each 15 min
#define DHT11_PIN 5  //DHT11
#define AQ_Set_Temperature_manual 25 // Set temperature in manual mode (if no MQTT connection available)

byte server[] = { 192,168,254,30 };
unsigned long now;
unsigned long sentTime;
float AQ_Temperature;
unsigned long aq_checkagain;

EthernetClient ethClient;
EthernetServer ethServer(80);
PubSubClient MQTT_Client(server, 1883, callback, ethClient);
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
dht DHT;

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

void HTTP_Server() {
  if (ethClient) {  // got client?
        boolean currentLineIsBlank = true;
        while (ethClient.connected()) {
            if (ethClient.available()) {   // client data available to read
                char c = ethClient.read(); // read 1 byte (character) from client
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    ethClient.println("HTTP/1.1 200 OK");
                    ethClient.println("Content-Type: text/html");
                    ethClient.println("Connection: close");
                    ethClient.println();
                    // send web page
                    ethClient.println("<!DOCTYPE html>");
                    ethClient.println("<html>");
                    ethClient.println("<head>");
                    ethClient.println("<title>Arduino Web Page</title>");
                    ethClient.println("</head>");
                    ethClient.println("<body>");
                    ethClient.println("<h1>Hello from Arduino!</h1>");
                    ethClient.println("<p>A web page from the Arduino server</p>");
                    ethClient.println("</body>");
                    ethClient.println("</html>");
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        ethClient.stop(); // close the connection
    } // end if (client)
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
  }
}/**/

void ManualAQLogics()  {
  now = millis();
  Serial.write("Starting manual AQ logics\n"); 
  sensors.requestTemperatures(); // Send the command to get temperatures from 1-wire
  if ((now - aq_checkagain)>AQ_MANUAL_INTERVAL){
    aq_checkagain=now;    
    AQ_Temperature=sensors.getTempCByIndex(0);
    if (AQ_Temperature>AQ_Set_Temperature_manual) {
        Serial.write("Starting AQ Cooler");
        gpio(PIN9,1);
    }  
    if (AQ_Temperature<AQ_Set_Temperature_manual-1) { 
        Serial.write("Starting AQ Heater");
        gpio(PIN8,1);
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
       if (strPayload=="ON") { gpio(PIN7,1); } else { gpio(PIN7,0); }
  }
  if (String (topic) == "AQ_Heater1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(PIN8,1); } else { gpio(PIN8,0);; }
  }
  if (String (topic) == "AQ_Cooler1" ) {
       Serial.println("Message arrived topic: " + String(topic));
       if (strPayload=="ON") { gpio(PIN9,1); } else { gpio(PIN9,0); }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(PIN7, OUTPUT);      // sets the digital pin as output
  pinMode(PIN8, OUTPUT);      // sets the digital pin as output    
  pinMode(PIN9, OUTPUT);      // sets the digital pin as output
  gpio(PIN7,0);   // Initialize digital pin 7 with OFF
  gpio(PIN8,0);   // Initialize digital pin 8 with OFF
  gpio(PIN9,0);   // Initialize digital pin 9 with OFF
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  IPAddress myIP(192,168,254,35);  //IP address of Arduino
  Ethernet.begin(mac,myIP);
  ethServer.begin();
  sensors.begin();  //start the one-wire library
}

void loop()
{
  if (!MQTT_Connect()){
    ManualAQLogics();
    delay(30000); // if not connected wait 30sec before next attempt
  } else {
    OneWireGetAndReport();  
    MQTT_Client.loop();
  }
  HTTP_Server();
} 