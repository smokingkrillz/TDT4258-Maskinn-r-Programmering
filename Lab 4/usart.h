#define F_CPU 4000000UL
#ifndef USART3_H
#define USART3_H

#define USART3_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 *(float)BAUD_RATE)) + 0.5)

#include <avr/io.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

void USART3_Init(void);
void USART3_SendChar(char c);
void USART3_SendString(char *str);
bool USART3_IsTxReady(void);
bool USART3_IsRxReady(void);
uint8_t USART3_Read();


#endif  // USART3_H
