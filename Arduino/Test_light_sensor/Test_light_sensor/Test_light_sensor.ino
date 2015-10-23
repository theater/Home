//
//    FILE: dht22_test.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.01
// PURPOSE: DHT library test sketch for DHT22 && Arduino
//     URL:
//
// Released to the public domain
//

#define INPUT_PIN 3

void setup()
{
    Serial.begin(115200);
    Serial.println("Type,\tstatus,\t");
}

void loop()
{
  boolean temp=0;
  temp=digitalRead(INPUT_PIN);
    // READ DATA
    Serial.print("INPUT, \t");
    if(temp){
          Serial.println("TOO LIGHT");
    } else {
        Serial.println("TOO DARK");
      }

    delay(2000);
}
//
// END OF FILE
//
