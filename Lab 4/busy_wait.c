#include <util/delay.h>
#include <stdbool.h>
#include <avr/io.h>

#define LED_bm PIN2_bm
void AC_init() {
    // Configure pin 2 of PORT D as an output pin. See chapter 18.3.1 of the datasheet
    PORTD.DIRCLR = PIN2_bm;

        //PORTD.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
        PORTB.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
        PORTC.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;
        PORTE.PINCONFIG = PORT_ISC_INPUT_DISABLE_gc | PORT_PULLUPEN_bm;

        PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
    //PORTA.PINCTRLUPD = 0xFF;
    PORTB.PINCTRLUPD = 0xFF;
    PORTC.PINCTRLUPD = 0xFF;
    //PORTD.PINCTRLUPD = 0xFF;
PORTE.PINCTRLUPD = 0xFF;


    // Disable digital input buffer and pull-up resistor for pin PD2
    // Set the positive input source to AINP0 and the negative input source to the DAC reference (DACREF)
    AC0.MUXCTRL = AC_MUXPOS_AINP0_gc | AC_MUXNEG_DACREF_gc;
    // Enable the Analog Comparator by writing AC_ENABLE_bm to the CTRLA register
    AC0.DACREF = 25;
    AC0.CTRLA = AC_OUTEN_bm | AC_ENABLE_bm;



}


void LED_init() {
    PORTA.DIRSET = PIN2_bm;
}
void set_LED_on(){
    // LED is active low. Set pin LOW to turn LED on
    PORTA.OUTCLR = PIN2_bm;
}
void set_LED_off(){
    // LED is active low. Set pin HIGH to turn LED off
    PORTA.OUTSET = PIN2_bm;
}

bool AC_above_threshold() {
    // Check the output of the Analog Comparator
    return (bool)(AC0.STATUS & AC_CMPSTATE_bm);
}

void VREF_init(void) {
    VREF.ACREF = VREF_REFSEL_1V024_gc;
}

int main() {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, CLKCTRL_PDIV_4X_gc | CLKCTRL_PEN_bm); // 4 MHz / 4
    // Initialize Analog Comparator
    AC_init();
    VREF_init();
    LED_init();



    while (1)
    {
        if (AC_above_threshold())
        {
            set_LED_off();
            
        }
        else
        {
            set_LED_on();
        }
    }
    return 0;
}