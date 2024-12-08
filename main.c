/* Uses UART1 to connect to HC05
Uses UART0 to connect to PuTTy and echo communication between TIVAboard and HC05 on PuTTy

*/

//***********************  main.c  ***********************

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "pll.h"
#include "UART.h"
#include "Bluetooth.h"
#include "LED.h"


// Bluetooth
#define slave  0
#define master 1

#define Configured	0  // Set this if you don't want to config HC05. TODOpart1&2
#define mode master
#define slaveaddress  "98d3,11,fc8eff"



// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void DelayMs(uint32_t n);
void HC05_PrintChar(char iput);
void HC05config(uint32_t mod);//, char *slaveaddress);

int main(void){
//	char inputDATA;
  DisableInterrupts();
  PLL_Init(Bus80MHz);
  LED_Init();
  Output_Init();       // UART0 only used for debugging
  printf("\n\r-----------\n\rMaster...\n\r");
	HC05_Init(38400);
	//bluetooth
	if (! Configured) { 
		printf("disconnect HC05 Vcc pin, connect it again, and press SW1/SW2 to enter AT mode \r\n");//"press HC-05 button while powering up to enter AT mode \r\n");
		GPIO_PORTB_DATA_R |= 0x20; // EN high
		while(Board_Input()==0){};
		HC05config(mode);//, slaveaddress);
		printf("HC05 is configured. Disconnect HC05 Vcc pin, connect it again to exit AT mode, if not paired pair it with computer and then press SW1/SW2  \r\n");
		GPIO_PORTB_DATA_R &= ~0x20; // EN low
		while(Board_Input()==0){};
	} else {
		printf("If HC05 is in AT mode, Disconnect HC05 Vcc pin, connect it again to exit AT mode; otherwise wait a few seconds if not paired pair it with computer and press SW1/SW2   \r\n");
		GPIO_PORTB_DATA_R &= ~0x20; // EN low
		while(Board_Input()==0){};
	}
	DelayMs(500);
	DisableInterrupts();
	// set UART1 baudrate to 9600 TODOpart2
	HC05_InitUART(9600,1);
   while(1){
		 // Add your code here. TODOpart2
		 //Use Board_Input() to read from switches. Then use HC05_PrintChar(), simpleUART1_OutChar(), or UART_OutUDec() 
		 //	to send a 1 or 2 to hc05 module if sw1 or sw2 is pressed, respectively.
		 // use DelayMs() to add 150ms delay to avoid sending too many 1s or 0s to the bluetooth module.
		 // in your gui.py code send a 'b' or 'B' to the COM port of bluetooth module. Here, if a 'B' is received in 
		 //	UART1 turn on blue LED, If a 'b' is received in UART1 turn off blue LED, and typing other letters toggles the red LED.
			
		 char data;
		 
		 
		 
			 uint8_t sw = Board_Input();
			 if (sw == 0x02) {  // SW1 pressed 
					HC05_PrintChar('1'); 
			 } 
			 if (sw == 0x01) {  // SW2 pressed 
					HC05_PrintChar('2');		
			 }
		 
		 DelayMs(150); 
			 
		 if ((UART1_FR_R & UART_FR_RXFE) == 0) {
			 data = (char)(UART1_DR_R);
		 }
		 if(data == 'B'){
				 LED_Blueon();
				 LED_RedOff();
		 } else if(data == 'b'){
				 LED_Blueoff();
				 LED_RedOff();
		 } 
  }
}




