#include "SysTick.h"
#include "tm4c123gh6pm.h"
#include "stdint.h"

// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(unsigned long period){
	// Initialize SysTick interrupts
	// Input: 32bit desired reload value
	NVIC_ST_CTRL_R = 0; // disable systick
	NVIC_ST_RELOAD_R = period - 1; // reload value
	NVIC_ST_CURRENT_R = 0; // clear flag
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // set priority
	NVIC_ST_CTRL_R = 0x07; // enable systick
}

// executes every 25 ms, collects a sample, converts and stores in mailbox
void SysTick_Handler(void){ 
	// Reads ADC & store in mailbox w/ flag
	Distance = Convert(ADC0_In());
	UART_ConvertDistance(Distance);
	Flag = 1;
}