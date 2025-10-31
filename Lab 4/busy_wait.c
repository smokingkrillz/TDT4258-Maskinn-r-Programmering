// #include <avr/io.h>
// #include <stdbool.h>
// #include <util/delay.h>


// #define LED_bm PIN2_bm
// void AC_init() {
//   // Configure pin 2 of PORT D as an output pin. See chapter 18.3.1 of the
//   PORTD.DIRCLR = PIN2_bm;
  

//   // Disable input buffer and enable pull-up resistor
//   PORTB.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
//   PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
//   PORTE.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;

//   PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
//   PORTB.PINCTRLUPD = 0xFF;
//   PORTC.PINCTRLUPD = 0xFF;
//   // PORTD.PINCTRLUPD = 0xFF;
//   PORTE.PINCTRLUPD = 0xFF;

//     //compare positive input (AIN0) to DAC reference
//   AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;

//   AC0.DACREF = 25;

//   // Enable output and the comparator
//   AC0.CTRLA = AC_OUTEN_bm | AC_ENABLE_bm;
// }

// void LED_init() 
// {
//     // Configure pin 2 of PORT A as an output pin
//     PORTA.DIRSET = PIN2_bm;
// }
// void set_LED_on() 
// {
//   // LED is active low. Set pin LOW to turn LED on
//   PORTA.OUTCLR = PIN2_bm;
// }
// void set_LED_off() 
// {
//   // LED is active low. Set pin HIGH to turn LED off
//   PORTA.OUTSET = PIN2_bm;
// }

// bool AC_above_threshold() 
// {
//   // Check the output of the Analog Comparator
//   return (bool)(AC0.STATUS & AC_CMPSTATE_bm);
// }

// // set reference voltage to 1.024V
// void VREF_init(void)
// { 
//     VREF.ACREF = VREF_REFSEL_1V024_gc; 
// }

// int main() 
// {
//   _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_4X_gc | CLKCTRL_PEN_bm);
//   // Initialize Analog Comparator
//   AC_init();
//   VREF_init();
//   LED_init();

//   while (1) {
//     if (AC_above_threshold()) {
//       set_LED_off();

//     } else {
//       set_LED_on();
//     }
//   }
//   return 0;
// }