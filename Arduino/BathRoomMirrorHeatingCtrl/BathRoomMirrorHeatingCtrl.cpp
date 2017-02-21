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

//LOCAL void ICACHE_FLASH_ATTR read_input_pin(void *arg)
//{
//	uint32 gpio_status;
//	ETS_GPIO_INTR_DISABLE(); // Disable gpio interrupts
//	sleepms(15);
//	if(!GPIO_INPUT_GET(PIN_GPIO13)) {
//	button_counter++;
//	console_printf("READ_INPUT_PIN executed...");
//	console_printf("COUNTER:%d\n",button_counter);
//	os_timer_disarm(&input_pin_timer); // Disarm input pin timer
//	os_timer_setfn(&input_pin_timer, (os_timer_func_t *)handle_pin_press, NULL); // Setup input pin timer
//	os_timer_arm(&input_pin_timer, button_timeout, 1); // Arm input pin timer, 0,5sec, repeat
//	sleepms(button_debounce);  // debounce time
//	}
//	gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
//	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status); 	//clear interrupt status
//
//	ETS_GPIO_INTR_ENABLE(); // Enable gpio interrupts}
//}


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
	pinMode(INPUT_PIN_IR_TRIGGER, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(INPUT_PIN_IR_TRIGGER), pinReadDebounced, RISING);
	pinMode(OUTPUT_PIN_RELAY, OUTPUT);
}

// The loop function is called in an endless loop
void loop() {
	offTimer.update();
}
