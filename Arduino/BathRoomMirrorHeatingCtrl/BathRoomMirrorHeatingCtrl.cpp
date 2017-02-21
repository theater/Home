// Do not remove the include below

#include "BathRoomMirrorHeatingCtrl.h"

#include <Arduino.h>
#include <Timer.h>

#include "C:/eclipse/arduinoPlugin/packages/arduino/hardware/avr/1.6.5/variants/standard/pins_arduino.h"

#define INPUT_PIN_IR_TRIGGER 3
#define OUTPUT_PIN_RELAY 5
#define TIMER_OFFSET 600000
#define DEBOUNCE_TIMEOUT 80

Timer offTimer;


void relayOff() {
	digitalWrite(OUTPUT_PIN_RELAY, LOW);
}

void pinReadDebounced() {
	detachInterrupt(INPUT_PIN_IR_TRIGGER);
	delay(15);
	if (digitalRead(INPUT_PIN_IR_TRIGGER)) {
		delay(DEBOUNCE_TIMEOUT);
		if(digitalRead(INPUT_PIN_IR_TRIGGER)) {
			digitalWrite(OUTPUT_PIN_RELAY, !digitalRead(OUTPUT_PIN_RELAY));
			if(digitalRead(OUTPUT_PIN_RELAY)) {
				offTimer.after(TIMER_OFFSET, relayOff);
			}
		}
	}
	delay(30);
	attachInterrupt(digitalPinToInterrupt(INPUT_PIN_IR_TRIGGER), pinReadDebounced, RISING);
}

//The setup function is called once at startup of the sketch
void setup() {
	pinMode(INPUT_PIN_IR_TRIGGER, INPUT);
	attachInterrupt(digitalPinToInterrupt(INPUT_PIN_IR_TRIGGER), pinReadDebounced, RISING);
	pinMode(OUTPUT_PIN_RELAY, OUTPUT);
}

// The loop function is called in an endless loop
void loop() {
	offTimer.update();
}
