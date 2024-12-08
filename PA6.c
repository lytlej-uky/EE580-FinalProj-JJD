// Uses ST7735.c LCD.

// ST7735 and TM4C123 COnnections 

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected 
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

// --------------------------------------------------------------------//
// ----------------No need to change this Program----------------------//
// --------------------------------------------------------------------//
#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "ST7735.h"
#include "PLL.h"
#include "fixed.h"
#include "tm4c123gh6pm.h"
#include "ADCCode.h"
#include "UART.h"
#include "Bluetooth.h"
#include "LED.h"

// prototypes for functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void DelayMs(uint32_t n);
void HC05_PrintChar(char iput);
void HC05config_slave(uint32_t mod);//, char *slaveaddress);
void HC05config_master(uint32_t mod);//, char *slaveaddress);
char Bluetooth_Read(void);

void DelayWait10ms(uint32_t n);
void PortF_Init(void);
// const will place these structures in ROM

struct outTestCase1{    // used to test routines
  int32_t InNumber;     // test input number
  char OutBuffer[12];   // Output String  
};
typedef const struct outTestCase1 outTestCaseType1;
outTestCaseType1 outTests1[13]={ 
{     0,  " =  0.000?\r" }, //     0/1000 = 0.000  
{     4,  " =  0.004?\r" }, //     4/1000 = 0.004  
{    -5,  " = -0.005?\r" }, //    -5/1000 = -0.005
{    78,  " =  0.078?\r" }, //    78/1000 = 0.078
{  -254,  " = -0.254?\r" }, //  -254/1000 = -0.254
{   999,  " =  0.999?\r" }, //   999/1000 = 0.999
{ -1000,  " = -1.000?\r" }, // -1000/1000 = -1.000
{  1234,  " =  1.234?\r" }, //  1234/1000 = 1.234
{ -5678,  " = -5.678?\r" }, // -5678/1000 = -5.678
{ -9999,  " = -9.999?\r" }, // -9999/1000 = -9.999
{  9999,  " =  9.999?\r" }, //  9999/1000 = 9.999
{ 10000,  " =  *.***?\r" }, // error
{-10000,  " =  *.***?\r" }  // error
};

// const will place these structures in ROM
struct outTestCase2{   // used to test routines
  uint32_t InNumber;   // test input number
  char OutBuffer[12];  // Output String  
};
typedef const struct outTestCase2 outTestCaseType2;

outTestCaseType2 outTests2[14]={ 
{     0,  " =   0.00?\r" }, //      0/256 = 0.00  
{     4,  " =   0.01?\r" }, //      4/256 = 0.01  
{    10,  " =   0.03?\r" }, //     10/256 = 0.03
{   200,  " =   0.78?\r" }, //    200/256 = 0.78
{   254,  " =   0.99?\r" }, //    254/256 = 0.99
{   505,  " =   1.97?\r" }, //    505/256 = 1.97
{  1070,  " =   4.17?\r" }, //   1070/256 = 4.17
{  5120,  " =  20.00?\r" }, //   5120/256 = 20.00
{ 12184,  " =  47.59?\r" }, //  12184/256 = 47.59
{ 26000,  " = 101.56?\r" }, //  26000/256 = 101.56
{ 32767,  " = 127.99?\r" }, //  32767/256 = 127.99
{ 34567,  " = 135.02?\r" }, //  34567/256 = 135.02
{255998,  " = 999.99?\r" }, // 255998/256 = 999.99
{256000,  " = ***.**?\r" }  // error
};
#define PF2   (*((volatile uint32_t *)0x40025010))
#define PF3   (*((volatile uint32_t *)0x40025020))
#define PF4   (*((volatile uint32_t *)0x40025040))

void Pause(void){
  while(PF4==0x00){ 
    DelayWait10ms(10);
  }
  while(PF4==0x10){
    DelayWait10ms(10);
  }
}
// 180 points on a circle of radius 2.000
const int32_t CircleXbuf[180] = { 2000, 1999, 1995, 1989, 1981, 1970, 1956, 1941, 1923, 1902, 1879, 1854, 1827, 1798, 1766, 1732, 1696, 1658, 1618, 1576, 1532, 1486, 1439, 1389, 1338, 1286, 1231, 1176, 1118, 1060, 1000, 939, 877, 813, 749, 684, 618, 551, 484, 416, 347, 278, 209, 140, 70, 0, -70, -140, -209, -278, -347, -416, -484, -551, -618, -684, -749, -813, -877, -939, -1000, -1060, -1118, -1176, -1231, -1286, -1338, -1389, -1439, -1486, -1532, -1576, -1618, -1658, -1696, -1732, -1766, -1798, -1827, -1854, -1879, -1902, -1923, -1941, -1956, -1970, -1981, -1989, -1995, -1999, -2000, -1999, -1995, -1989, -1981, -1970, -1956, -1941, -1923, -1902, -1879, -1854, -1827, -1798, -1766, -1732, -1696, -1658, -1618, -1576, -1532, -1486, -1439, -1389, -1338, -1286, -1231, -1176, -1118, -1060, -1000, -939, -877, -813, -749, -684, -618, -551, -484, -416, -347, -278, -209, -140, -70, 0, 70, 140, 209, 278, 347, 416, 484, 551, 618, 684, 749, 813, 877, 939, 1000, 1060, 1118, 1176, 1231, 1286, 1338, 1389, 1439, 1486, 1532, 1576, 1618, 1658, 1696, 1732, 1766, 1798, 1827, 1854, 1879, 1902, 1923, 1941, 1956, 1970, 1981, 1989, 1995, 1999
};
const int32_t CircleYbuf[180] = {0, 70, 140, 209, 278, 347, 416, 484, 551, 618, 684, 749, 813, 877, 939, 1000, 1060, 1118, 1176, 1231, 1286, 1338, 1389, 1439, 1486, 1532, 1576, 1618, 1658, 1696, 1732, 1766, 1798, 1827, 1854, 1879, 1902, 1923, 1941, 1956, 1970, 1981, 1989, 1995, 1999, 2000, 1999, 1995, 1989, 1981, 1970, 1956, 1941, 1923, 1902, 1879, 1854, 1827, 1798, 1766, 1732, 1696, 1658, 1618, 1576, 1532, 1486, 1439, 1389, 1338, 1286, 1231, 1176, 1118, 1060, 1000, 939, 877, 813, 749, 684, 618, 551, 484, 416, 347, 278, 209, 140, 70, 0, -70, -140, -209, -278, -347, -416, -484, -551, -618, -684, -749, -813, -877, -939, -1000, -1060, -1118, -1176, -1231, -1286, -1338, -1389, -1439, -1486, -1532, -1576, -1618, -1658, -1696, -1732, -1766, -1798, -1827, -1854, -1879, -1902, -1923, -1941, -1956, -1970, -1981, -1989, -1995, -1999, -2000, -1999, -1995, -1989, -1981, -1970, -1956, -1941, -1923, -1902, -1879, -1854, -1827, -1798, -1766, -1732, -1696, -1658, -1618, -1576, -1532, -1486, -1439, -1389, -1338, -1286, -1231, -1176, -1118, -1060, -1000, -939, -877, -813, -749, -684, -618, -551, -484, -416, -347, -278, -209, -140, -70
};
// 50 points of a star
const int32_t StarXbuf[50] = {0, -6, -12, -18, -24, -30, -35, -41, -47, -53, 59, 53, 47, 41, 35, 30, 24, 18, 12, 6, 95, 76, 57, 38, 19, 0, -19, -38, -57, -76, -59, -44, -28, -13, 3, 18, 33, 49, 64, 80, -95, -80, -64, -49, -33, -18, -3, 13, 28, 44
};
const int32_t StarYbuf[50] = {190, 172, 154, 136, 118, 100, 81, 63, 45, 27, 9, 27, 45, 63, 81, 100, 118, 136, 154, 172, 121, 121, 121, 121, 121, 121, 121, 121, 121, 121, 9, 20, 31, 43, 54, 65, 76, 87, 99, 110, 121, 110, 99, 87, 76, 65, 54, 43, 31, 20
};

// ADC Globals
unsigned long Size = 2; 			// potentiomter range in cm
unsigned char String[10]; 		// string to output to screen
unsigned long Distance; 			// potentiometer distance in units of 0.001 cm
unsigned long ADCvalue;  			// 12-bit 0 to 4095 sample
unsigned long Flag;     			// 1 means valid Distance, 0 means Distance is empty, A global variable to keep track if Systick is overflow
unsigned long Bar;						// y size of the bar to be made

unsigned long Convert(unsigned long sample);
void ConvertDistancetostring(unsigned long n);
void SysTick_Init_Interrupts(unsigned long period);
void SysTick_Handler(void);

//int main(void){uint32_t i;
//  PLL_Init(Bus80MHz); 
//	ADC0_InitSWTriggerSeq3_Ch1();					// ADC initialization PE2/AIN1
//	SysTick_Init_Interrupts(25000000);			// Pass the proper value such that we get Interrupt at every 500ms
//  PortF_Init();
//  ST7735_InitR(INITR_REDTAB);
//	int reading = 0;
//  while(1){

//			
//		// is "Flag" set?
//		// If yes then disable Flag and Print the Value using UART_printf
//		while (Flag == 0){ } // wait

//    ST7735_FillScreen(0);  // set screen to black
//    ST7735_SetCursor(0,0);
//		
//    printf("ADC Value\r");
//    printf("%s\r", String);
//		
//		if (Bar < 50) {
//			ST7735_FillRect(52, 150 - Bar, 24, Bar, ST7735_RED);
//		} else if (Bar < 90) {
//			ST7735_FillRect(52, 150 - Bar, 24, Bar, ST7735_YELLOW);
//		} else {
//			ST7735_FillRect(52, 150 - Bar, 24, Bar, ST7735_GREEN);
//		}

//		DelayWait10ms(100);
//		Flag = 0;	

//		reading++;
//		
//		
// 


////    ST7735_XYplotInit("Circle",-2500, 2500, -2500, 2500);
////    ST7735_XYplot(180,(int32_t *)CircleXbuf,(int32_t *)CircleYbuf);
////    Pause();
////    
////    ST7735_XYplotInit("Star- upper right",-450, 150, -400, 200);
////    ST7735_XYplot(50,(int32_t *)StarXbuf,(int32_t *)StarYbuf);
////    Pause(); 
//  } 
//} 


int main(void ) {
	
	DisableInterrupts();
  PLL_Init(Bus80MHz);
  Output_Init();       // UART0 only used for debugging
  printf("\n\r-----------\n\rSlave...\n\r");
	HC05_Init(38400);
	//bluetooth
	{ 
		printf("disconnect HC05 Vcc pin, connect it again, and press SW1/SW2 to enter AT mode \r\n");//"press HC-05 button while powering up to enter AT mode \r\n");
		GPIO_PORTB_DATA_R |= 0x20; // EN high
		while(Board_Input()==0){};
		HC05config_slave(0);//, slaveaddress);
		//HC05config_master(0);
		printf("HC05 is configured. Disconnect HC05 Vcc pin, connect it again to exit AT mode, if not paired pair it with computer and then press SW1/SW2  \r\n");
		GPIO_PORTB_DATA_R &= ~0x20; // EN low
		DelayMs(150);
		while(Board_Input()==0){};
	}
	
	return 0;
}

// PF4 is input
// Make PF2 an output, enable digital I/O, ensure alt. functions off
void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTF_PCTL_R &= ~0x000F0F00; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x14;      // 4) disable analog function on PF2, PF4
  GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set direction to output
  GPIO_PORTF_AFSEL_R &= ~0x14;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x14;         // 7) enable digital port
}


// Subroutine to wait 10 msec
// Inputs: None
// Outputs: None
// Notes: ...
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
	  	time--;
    }
    n--;
  }
}
void SysTick_Init_Interrupts(unsigned long period){
	// TODO: Initialize SysTick interrupts
	// Input: 32bit desired reload value
	NVIC_ST_CTRL_R = 0;	// disable systick
	NVIC_ST_RELOAD_R = period - 1; // reload value
	Flag = 0; // clear flag
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // set priority
	NVIC_ST_CTRL_R = 0x00000007; // PLL clock and interrupts
}

//********Convert****************
// Convert a 12-bit binary ADC sample into a 32-bit unsigned
// fixed-point distance (resolution 0.001 cm).  Calibration
// data is gathered using known distances and reading the
// ADC value measured on PE1.  
// Overflow and dropout should be considered 
// Input: sample  12-bit ADC sample
// Output: 32-bit distance (resolution 0.001cm)
unsigned long Convert(unsigned long sample){
	// TODO
	// (samples * size *1000)/4095
	// Use the above formula to convert to fixed value
	// "Size" is global variable that indicates the length of the Potentiometer, for our case its 2 CM (2.000)
	return ((sample * 1000 * Size) / 4095);
}


// TODO: executes every 25 ms, collects a sample, converts and stores in String
void SysTick_Handler(void){ 
	//Use the handler to read ADC value, then Conver () function and finally converting values using pre-written ConvertDistancetostring
	ADCvalue = ADC0_In();	// 1) Read ADC and store in ADCvalue
	Distance = Convert(ADCvalue); // 2) convert ADCvalue to an integer that represents distance in fixed point. Store the converted value to variable "Distance" which is global varaible
	Bar = ADCvalue * 120 / 4095;
	ConvertDistancetostring(Distance); //  3) use ConvertDistancetostring. This function automatically update "Setting" which is a global variable
	Flag = 1;           //  set Flag when existing
}

/*---------------Instructor Written Function-------------------*/
// Converts ADC input into an ASCII string of the actual distance
// Converts a 32-bit distance into an ASCII string
// Input: 32-bit number to be converted (resolution 0.001cm)
// Output: store the conversion in global variable String[10]
// Fixed format 1 digit, point, 3 digits, space, units, null termination
// Examples
//    4 to "0.004 cm"  
//   31 to "0.031 cm" 
//  102 to "0.102 cm" 
// 2210 to "2.210 cm"
//10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
// TODO: just read this function and try to understand how it works
void ConvertDistancetostring(unsigned long n){
	int i;// used in for loops
	
	if(n>9999){ // 10000 to "*.*** cm"
		for(i = 1; i < 5; i++)
			String[i] = '*';
	}
	
	else{ // get digits from least to most significant
		for(i = 4; i > 0; i--){
			String[i] = n % 10 + 0x30;
			n = n / 10;
		}
	}
	
	// Rest of String
	String[0] = String[1];
	String[1] = '.';
	String[5] = ' ';
	String[6] = 'c';
	String[7] = 'm';
	String[8] = ' ';
	String[9] = 0;
}
