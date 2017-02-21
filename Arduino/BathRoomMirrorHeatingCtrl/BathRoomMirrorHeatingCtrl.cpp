// Do not remove the include below
#include "BathRoomMirrorHeatingCtrl.h"
#include "Timer.h"

#define INPUT_PIN_IR_TRIGGER 6
#define OUTPUT_PIN_RELAY 5
#define TIMER_OFFSET 600000

Timer offTimer;


void relayOff() {
	digitalWrite(OUTPUT_PIN_RELAY, LOW);
}

void pinReadDebounced() {
	if (digitalRead(INPUT_PIN_IR_TRIGGER)) {
		// do it with interrupts and debounce timer later...
		digitalWrite(OUTPUT_PIN_RELAY, HIGH);
		offTimer.after(TIMER_OFFSET, relayOff);
	}
}

//The setup function is called once at startup of the sketch
void setup() {
	pinMode(INPUT_PIN_IR_TRIGGER, INPUT);
	pinMode(OUTPUT_PIN_RELAY, OUTPUT);
}

// The loop function is called in an endless loop
void loop() {
	pinReadDebounced();
	offTimer.update();
}
