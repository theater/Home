// Do not remove the include below

#include "BathRoomMirrorHeatingCtrl.h"


#include <Arduino.h>
#include <Timer.h>
#include <DHT.h>

#include "C:/eclipse/arduinoPlugin/packages/arduino/hardware/avr/1.6.5/variants/standard/pins_arduino.h"

// PINS:
#define INPUT_PIN_IR_TRIGGER 3
#define OUTPUT_PIN_RELAY 5
#define DHT_PIN 8

// PROXIMITY SENSOR SETTINGS
#define TIMER_OFFSET 20000
#define DEBOUNCE_TIMEOUT 750

// DHT SENSOR TIMER SETTINGS
#define DHT_TIMER_EVERY 30000

Timer offTimer;
Timer sensorReadTimer;
DHT dhtSensor(DHT_PIN, DHT22);

void relayOff() {
	digitalWrite(OUTPUT_PIN_RELAY, LOW);
	Serial.println("Shutdown by timer");
}

void pinReadDebounced() {
	detachInterrupt(INPUT_PIN_IR_TRIGGER);
	delay(15);
	if (digitalRead(INPUT_PIN_IR_TRIGGER)) {
		delay(DEBOUNCE_TIMEOUT);
		if(digitalRead(INPUT_PIN_IR_TRIGGER)) {
			int pinState = digitalRead(OUTPUT_PIN_RELAY);
			Serial.print("State before set:");Serial.println(pinState);
			digitalWrite(OUTPUT_PIN_RELAY, !pinState);
			pinState = digitalRead(OUTPUT_PIN_RELAY);
			Serial.print("State after set:");Serial.println(pinState);
			if(digitalRead(OUTPUT_PIN_RELAY)) {
				int id = offTimer.after(TIMER_OFFSET, relayOff);
				Serial.print("Timer initialized ! ID:");
				Serial.println(id);
			} else {
				offTimer.stop(0);
			}
		}
	}
	delay(30);
	attachInterrupt(digitalPinToInterrupt(INPUT_PIN_IR_TRIGGER), pinReadDebounced, RISING);
}

void readDhtInfo() {
	float hum = dhtSensor.readHumidity();
	float temp = dhtSensor.readTemperature();
	Serial.print("Humidity:"); Serial.println(hum);
	Serial.print("Temperature:"); Serial.println(temp);
}

//The setup function is called once at startup of the sketch
void setup() {
	Serial.begin(115200);
	pinMode(INPUT_PIN_IR_TRIGGER, INPUT);
	attachInterrupt(digitalPinToInterrupt(INPUT_PIN_IR_TRIGGER), pinReadDebounced, RISING);
	pinMode(OUTPUT_PIN_RELAY, OUTPUT);
	dhtSensor.begin();
	sensorReadTimer.every(DHT_TIMER_EVERY, readDhtInfo);
}

// The loop function is called in an endless loop
void loop() {
	offTimer.update();
}
