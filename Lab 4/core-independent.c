#include <avr/io.h>
#include <stdbool.h>
#include <avr/power.h>
#include <avr/sleep.h>
#define LED_bm PIN2_bm
void AC_init() { // Disable input buffer and enable pull-up resistor
  PORTA.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
  PORTB.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
  PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
  PORTD.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
  PORTE.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
  PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PINCTRLUPD = 0xFF;
  PORTB.PINCTRLUPD = 0xFF;
  PORTC.PINCTRLUPD = 0xFF;
  PORTD.PINCTRLUPD = 0xFF;
  PORTE.PINCTRLUPD = 0xFF;
  PORTD.DIRCLR = LED_bm;
  PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
  AC0.MUXCTRL = AC_MUXPOS_AINP0_gc |  AC_MUXNEG_DACREF_gc;
  AC0.CTRLA =  AC_POWER_PROFILE2_gc | AC_RUNSTDBY_bm | AC_ENABLE_bm;
  // Enable comparator interrupt
  AC0.DACREF = 25;
  AC0.INTCTRL = 0;
  EVSYS.CHANNEL0 = EVSYS_CHANNEL0_AC0_OUT_gc;
}
void VREF_init(void) {
  VREF.ACREF = VREF_REFSEL_1V024_gc;
}

void LED_init() {
  PORTA.DIRSET = LED_bm;
  EVSYS.USEREVSYSEVOUTA = 1;
           // Enable EVOUTA output

} // Function to initialize sleep mode
void sleep_init(void) {
  set_sleep_mode(SLEEP_MODE_STANDBY);
}

void disable_unused_peripherals(void) {
  USART0.CTRLA = 0;
  USART0.CTRLB = 0; // Disable TCA0 and TCB0 timers
  TCA0.SINGLE.CTRLA = 0;
  TCB0.CTRLA = 0; // Disable ADC and DAC
  ADC0.CTRLA = 0;
  DAC0.CTRLA = 0; // Optional: Set unused pins to inputs (example for PORTA)
  PORTA.DIR = 0x00;
  // All pins as inputs
   PORTA.PIN0CTRL = 0;
   SPI0.CTRLA = 0;
TWI0.MCTRLA = 0;
}

int main() {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm);
    AC_init();
    VREF_init();
    LED_init();
    disable_unused_peripherals();
    sleep_init();
    BOD.CTRLA = BOD_SLEEP_DIS_gc | BOD_ACTIVE_ENABLED_gc;
  sleep_mode();
  while (1) {
  }
  return 0; 
 
}