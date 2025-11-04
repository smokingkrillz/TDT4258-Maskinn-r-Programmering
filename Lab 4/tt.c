// void ADC_init_window(void) 
// {
//     // Reference = 1.024 V
//     VREF.ADC0REF = VREF_REFSEL_1V024_gc;     // Or use VDD if that's sensor supply

//     // Average 8 samples, single-ended, 12-bit
//     ADC0.CTRLB = ADC_SAMPNUM_ACC8_gc;
//     ADC0.CTRLC = ADC_PRESC_DIV4_gc; // uses VREF defined above

//     // Input = PD2 (AIN2)
//     ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
//     ADC0.CTRLD  = ADC_INITDLY_DLY16_gc | ADC_SAMPDLY_DLY2_gc;

//     // Window thresholds
//     ADC0.WINLT = 100; 
//     ADC0.WINHT = 300;

//     // Window comparison mode: fire when outside window
//     ADC0.CTRLE = ADC_WINCM_OUTSIDE_gc;

//     // Enable window comparator interrupt
//     ADC0.INTCTRL = ADC_WCMP_bm;

//     // Enable ADC, run in standby, free-running
//     ADC0.CTRLA = ADC_ENABLE_bm | ADC_FREERUN_bm | ADC_RUNSTBY_bm;
//     ADC0.COMMAND = ADC_STCONV_bm;
// }

// ISR(ADC0_WCMP_vect)
// {
//     uint16_t result = ADC0.RES;
//     ADC0.INTFLAGS = ADC_WCMP_bm; // clear flag

//     if (result < ADC0.WINLT) {
//         PORTA.OUTCLR = LED_bm; // Dark → LED ON (active-low)
//     } else if (result > ADC0.WINHT) {
//         PORTA.OUTSET = LED_bm; // Bright → LED OFF
//     }
//     // No OUTTGL, and remove AC0.STATUS = AC_CMPIF_bm;
// }

// // ... rest of code ...
// Main Setup (no key changes, but for clarity):
// core-independent.c

// Apply
// int main(void)
// {
//     USART3_Init();
//     PORTA.DIRSET = LED_bm;
//     PORTA.OUTSET = LED_bm; // LED off (active-low)
//     PORTD.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;

//     ADC_init_window();
//     sei();

//     set_sleep_mode(SLEEP_MODE_STANDBY);
//     sleep_enable();
//     while (1) {
//         sleep_cpu(); // sleep until ADC triggers 
//     }
//     // No return needed unless for C++/MISRA
// }
