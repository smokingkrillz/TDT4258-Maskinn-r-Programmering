// #include "usart.h"
// #include <avr/sleep.h>
// #include <avr/interrupt.h>

// #define LED_bm PIN2_bm
// void AC_init() {
//   // Configure pin 2 of PORT D as an output pin

//   // Disable input buffer and enable pull-up resistor
//   PORTB.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
//   PORTB.PINCTRLUPD = 0xFF;
//   PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
//   PORTC.PINCTRLUPD = 0xFF;
//   PORTE.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
//   PORTE.PINCTRLUPD = 0xFF;

//   // compare positive input (AIN0) to DAC reference
//   PORTD.DIRCLR = LED_bm;
//   PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

//   AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

//   // Enable output and the comparator
//   AC0.CTRLA = AC_POWER_PROFILE2_gc | AC_RUNSTDBY_bm | AC_ENABLE_bm;
//   AC0.CTRLB = AC_WINSEL_DISABLED_gc;

//   AC0.DACREF = 25;
// }

// void VREF_init(void) { VREF.ACREF = VREF_REFSEL_1V024_gc; }

// void LED_init() { PORTA.DIRSET = LED_bm; }

// void set_LED_on() {
//   // LED is active low. Set pin LOW to turn LED on
//   PORTA.OUTCLR = LED_bm;
// }

// void set_LED_off() {
//   // LED is active low. Set pin HIGH to turn LED off
//   PORTA.OUTSET = LED_bm;
// }

// // Function to initialize sleep mode
// void sleep_init(void) {
//   set_sleep_mode(SLEEP_MODE_STANDBY); // Standby sleep mode
// }

// void TCA0_init() {
//   // Set the period of the timer. PER = period[s] * F_CPU / Prescaler = 0.01s *
//   // 4 000 000 Hz / 2
//   TCA0.SINGLE.PER = 20000;
//   // Enable timer overflow interrupt
//   TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
//   // Run timer in standby mode, set prescaler to 2, enable timer
//   TCA0.SINGLE.CTRLA =
//       TCA_SINGLE_RUNSTDBY_bm | TCA_SINGLE_CLKSEL_DIV2_gc | TCA_SINGLE_ENABLE_bm;
// }

// bool AC_above_threshold() {
//   // Check the output of the Analog Comparator
//   return (bool)(AC0.STATUS & AC_CMPSTATE_bm);
// }

// ISR(TCA0_OVF_vect) {
//   // Check if the AC output is high (input voltage above threshold)
//   if (AC_above_threshold()) {
//     set_LED_off();
//   } else {
//     set_LED_on();
//   }

//   // Clear interrupt flag
//   TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
// }

// int main() {
//   AC_init();
//   VREF_init();
//     LED_init();
//   // Initialize timer
//   TCA0_init();
//   // Enable interrupts
//   sei();

//     sleep_init();

//   while (1) {
//     sleep_mode();
//   }
//   return 0;
// }