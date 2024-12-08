// HC05.c
// Created by Dr. Salehi for CPE580, Bluetooth PA 
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "Bluetooth.h"
#include "UART.h"

#define BUFFER_SIZE 1024
// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
// prototypes for other functions
void DelayMs(uint32_t delay);
void HC05InitUART(void);
void HC05_PrintChar(char iput);
void HC05FIFOtoBuffer(void);

void HC05config(uint32_t mode);//,char *slaveaddress);
void ATsend(const char* inputString);


uint32_t RXBufferIndex = 0;
uint32_t LastReturnIndex = 0;
uint32_t CurrentReturnIndex = 0;
char RXBuffer[BUFFER_SIZE];
char TXBuffer[BUFFER_SIZE];

/*
=======================================================================
==========              search FUNCTIONS                     ==========
=======================================================================
*/
char SearchString[32]="ok";
volatile bool SearchLooking = false;
volatile bool SearchFound = false;
volatile uint32_t SearchIndex = 0;
char lc(char letter){
  if((letter>='A')&&(letter<='Z')) letter |= 0x20;
  return letter;
}
//-------------------SearchStart -------------------
// Receives and sets the target string that we want to search for in received data stream. Also sets the flags (Looking and Found)
// Inputs: String that we look for
// Outputs: none, but sets SearchString and flags to be used by SearchCheck
void SearchStart(void) { //char *pt){
	strcpy(SearchString,"ok");
 // strcpy(SearchString,pt);
  SearchIndex = 0;
  SearchFound = 0;
  SearchLooking = 1;
}
//-------------------SearchCheck -------------------
// is called by SearchStart to look for target string in received data stream when transfering each character of data from fifo to buffer
// Inputs: a character from the received data
// Outputs: none, sets SearchFound and resets Looking flag if find the target SearchString
void SearchCheck(char letter){
  if(SearchLooking){
    if(SearchString[SearchIndex] == lc(letter)){ // match letter with the character in the Indexth position of SearchString
      SearchIndex++;
      if(SearchString[SearchIndex] == 0){ // reached end of SearchString? then match string.
        SearchFound = 1;
        SearchLooking = false;
				//UART_OutString("\r\n");
      }
    }else{
      SearchIndex = 0; // start over
    }
  }
}



//------------------- HC05_InitUART-------------------
// intializes uart and gpio needed to communicate with HC05
// Configure UART1 for serial full duplex operation
// Inputs: baud rate (e.g., 115200 or 9600)
// Outputs: none
void HC05_InitUART(uint32_t baud, int echo){ volatile int delay;
  SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1; // Enable UART1
  while((SYSCTL_PRUART_R&0x02)==0){};
  SYSCTL_RCGCGPIO_R |= 0x02; // Enable PORT B clock gating
  while((SYSCTL_PRGPIO_R&0x02)==0){};
  GPIO_PORTB_AFSEL_R |= 0x03;
  GPIO_PORTB_DIR_R |= 0x20; // PB5 output to reset
  GPIO_PORTB_PCTL_R =(GPIO_PORTB_PCTL_R&0xFF0FFF00)|0x00000011;
  GPIO_PORTB_DEN_R   |= 0x23; //23
  GPIO_PORTB_DATA_R |= 0x20; // reset high
  UART1_CTL_R &= ~UART_CTL_UARTEN;                  // Clear UART1 enable bit during config
  UART1_IBRD_R = 80000000/16/baud;
  UART1_FBRD_R = ((64*((80000000/16)%baud))+baud/2)/baud;      
 
  UART1_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);  // 8 bit word length, 1 stop, no parity, FIFOs enabled
  UART1_IFLS_R &= ~0x3F;                            // Clear TX and RX interrupt FIFO level fields
  UART1_IFLS_R += UART_IFLS_RX1_8 ;                 // RX FIFO interrupt threshold >= 1/8th full
  UART1_IM_R  |= UART_IM_RXIM | UART_IM_RTIM;       // Enable interupt on RX and RX transmission end
  UART1_CTL_R |= UART_CTL_UARTEN;                   // Set UART1 enable bit
}
// -----------UART1_Handler-----------
// called on one receiver data input followed by timeout
// or     on going from 1 to 2 data input characters
void UART1_Handler(void){
  if(UART1_RIS_R & UART_RIS_RXRIS){   // rx fifo >= 1/8 full
    UART1_ICR_R = UART_ICR_RXIC;      // acknowledge interrupt
    HC05FIFOtoBuffer();
  }
  if(UART1_RIS_R & UART_RIS_RTRIS){   // receiver timed out
    UART1_ICR_R = UART_ICR_RTIC;      // acknowledge receiver time
    HC05FIFOtoBuffer();
  }
}
//--------HC05_EnableRXInterrupt--------
// - enables uart rx interrupt
// Inputs: none
// Outputs: none
void HC05_EnableRXInterrupt(void){
  NVIC_EN0_R = 1<<6; // interrupt 6
}

//--------HC05_DisableRXInterrupt--------
// - disables uart rx interrupt
// Inputs: none
// Outputs: none
void HC05_DisableRXInterrupt(void){
  NVIC_DIS0_R = 1<<6; // interrupt 6
}

//--------HC05_PrintChar--------
// prints a character to the HC05 via uart
// Inputs: character to transmit
// Outputs: none
// busy-wait synchronization
void HC05_PrintChar(char input){
  while((UART1_FR_R&UART_FR_TXFF) != 0) {};
  UART1_DR_R = input;
  UART_OutChar(input); // echo debugging
}
//----------HC05FIFOtoBuffer----------

void HC05FIFOtoBuffer(void){
  char letter;
  while((UART1_FR_R&UART_FR_RXFE) == 0){
    letter = UART1_DR_R;        // retrieve char from FIFO
    
      UART_OutCharNonBlock(letter); // echo
    if(RXBufferIndex >= BUFFER_SIZE){
      RXBufferIndex = 0; // store last BUFFER_SIZE received
    }
    RXBuffer[RXBufferIndex] = letter; // put char into buffer
    RXBufferIndex++; // increment buffer index
    SearchCheck(letter);               // check for end of command
  }
}


////////////////////
// DelayMs
//  - busy wait n milliseconds
// Input: time to wait in msec
// Outputs: none
void DelayMs(uint32_t n){
  volatile uint32_t time;
  while(n){
    time = 6665;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
    }
    n--;
  }
}
void DelayMsSearching(uint32_t n){
  volatile uint32_t time;
  while(n){
    time = 4065;  // 1msec, tuned at 80 MHz
    while(time){
      time--;
      if(SearchFound) return;
    }
    n--;
  }
}

//Bluetooth
void ATsend( const char* inputString){
  int index;
	int try=3;
		UART_OutString("\r\n");
		SearchStart();
		while(try){
			index = 0;
		//		ATsend("AT+UART?\r\n");
			while(inputString[index] != 0){
				HC05_PrintChar(inputString[index++]);
			}
			DelayMsSearching(2000); // wait and search for OK
			if(SearchFound) break; // success
			try--;
		}
}

/*
=======================================================================
==========          HC05 configuration                 ==========*/

void HC05config(uint32_t mode){

	  ATsend("AT\r\n"); //  ATsend("AT\r\n"); check if hc05 is in AT mode. TODOpart1
		if(SearchFound == 0) {
			printf("Can't enter AT mode"); while(1){};
			}
		
			ATsend("AT+NAME=HC-05-MASTER\r\n");  // ATsend("AT+NAME=????\r\n"); set a name. TODOpart1
			if(SearchFound == 0) {
					printf("Can't rename slave module"); while(1){};
			}
			ATsend("AT+PSWD=4321\r\n");// ATsend("AT+PSWD=???\r\n");    set a pasword. TODOpart1
			if(SearchFound == 0) {
					printf("Can't change password of slave module"); while(1){};
			}
			ATsend("AT+UART=9600,0,0\r\n"); // ATsend("AT+UART=9600,0,0\r\n"); set a uart rate to9600. TODOpart1  
			if(SearchFound == 0) {
				printf("Can't set uart baud rate"); while(1){};
			}
			ATsend("AT+ROLE=1\r\n"); //	ATsend("AT+ROLE=1\r\n");set to master mode. TODOpart1 
			if(SearchFound == 0) {
				printf("Can't enter set Master mode"); while(1){};
			}
			ATsend("AT+INIT\r\n");
			if (SearchFound == 0) {
					printf("Can't initialize module"); while(1){}
			}
			ATsend("AT+IAC=9e8b33\r\n");
			if (SearchFound == 0) {
					printf("Failed to reset module");
			}
			ATsend("AT+CLASS=0\r\n");
			if (SearchFound == 0) {
					printf("Failed to reset module");
			}
			ATsend("AT+INQM=1,9,48\r\n"); //	configure inquiry mode
			if(SearchFound == 0) {
				printf("Can't set Inquiry mode"); while(1){};
			}
			
			

			ATsend("AT+ROLE?\r\n");
			if(SearchFound == 0) {
					printf("Can't get module role"); while(1){};
			}
			ATsend("AT+NAME?\r\n");
			if(SearchFound == 0) {
					printf("Can't get module name"); while(1){};
			}
			ATsend("AT+PSWD?\r\n");
			if(SearchFound == 0) {
					printf("Can't rename slave module"); while(1){};
			}
			ATsend("AT+UART?\r\n");
			if(SearchFound == 0) {
					printf("UART mode"); while(1){};
			}
			ATsend("AT+INQM?\r\n"); //	configure inquiry mode
			if(SearchFound == 0) {
				printf("Can't get Inquiry mode"); while(1){};
			}
			
			ATsend("AT+INQ\r\n"); // inquire
			if(SearchFound == 0) {
				printf("Can't inquiry devices"); while(1){};
			}
  }
	


void HC05_Init(uint32_t baud){
  HC05_InitUART(baud,true); // baud rate, no echo to UART0
  HC05_EnableRXInterrupt();
  SearchLooking = false;
  SearchFound = false;
  EnableInterrupts();
}


