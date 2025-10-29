#include <util/delay.h>
#include "usart.h"


int main() {
    // Initialize USART3
    USART3_Init();
    
    // Infinite loop to send character every second
    while(1) {
        // Send character 'A'
        USART3_SendChar('A');
        
        // Wait 1 second (1000 ms)
        _delay_ms(1000);
    }
    
    return 0;
}