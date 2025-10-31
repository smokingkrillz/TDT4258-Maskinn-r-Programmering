#include "usart.h"
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define LED_bm PIN2_bm
void AC_init() {
  // Configure pin 2 of PORT D as an output pin
  // compare positive input (AIN0) to DAC reference
  PORTD.DIRCLR = LED_bm;
  PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

  AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

  // Enable output and the comparator
  AC0.CTRLA = AC_RUNSTDBY_bm | AC_ENABLE_bm;
  // Enable comparator interrupt
  AC0.DACREF = 25;
    AC0.INTCTRL = AC_CMP_bm;

}

void VREF_init(void) { VREF.ACREF = VREF_REFSEL_1V024_gc; }

void LED_init() { PORTA.DIRSET = LED_bm; }

void set_LED_on() {
  // LED is active low. Set pin LOW to turn LED on
  PORTA.OUTCLR = LED_bm;
}

void set_LED_off() {
  // LED is active low. Set pin HIGH to turn LED off
  PORTA.OUTSET = LED_bm;
}

// Function to initialize sleep mode
void sleep_init(void) {
  set_sleep_mode(SLEEP_MODE_STANDBY); // Standby sleep mode
}


bool AC_above_threshold() {
  // Check the output of the Analog Comparator
  return (bool)(AC0.STATUS & AC_CMPSTATE_bm);
}

ISR(AC0_AC_vect) {
  // Check if the AC output is high (input voltage above threshold)
  if (AC_above_threshold()) {
    set_LED_off();
  } else {
    set_LED_on();
  }

  // Clear interrupt flag
  AC0.STATUS = AC_CMPIF_bm;
}

int main() {
  AC_init();
  VREF_init();
    LED_init();

  // Enable interrupts
  sei();

    sleep_init();

  while (1) {
    sleep_mode();
  }
  return 0;
}