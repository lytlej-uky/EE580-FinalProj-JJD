#include <stdint.h>
#include "ADCCode.h"
#include "tm4c123gh6pm.h"

#define TODO 0
//ADC init at PE2, 125K samples/sec
void ADC0_InitSWTriggerSeq3_Ch1(void){volatile unsigned long delay;
	
	// TODO: All the steps in Init are needed			
	
		SYSCTL_RCGCADC_R |= 1; // 1) activate ADC0
		while((SYSCTL_PRADC_R&0x0001) != 0x0001){}; //wait for ADC stabilization
	  SYSCTL_RCGCGPIO_R |= 0x10; // 2) activate clock for Port E
		while((SYSCTL_PRGPIO_R&0x10) != 0x10){}; // 3 for Port E stabilization
		GPIO_PORTE_DIR_R &= ~0x4U; // 4) make PE2 input		TODO
		GPIO_PORTE_AFSEL_R |= 0x4; // 5) enable alternate function on PE2
		GPIO_PORTE_DEN_R &= ~0x4U; // 6) disable digital I/O on PE2
		GPIO_PORTE_AMSEL_R |= 0x4; // 7) enable analog functionality on PE2 //
		ADC0_PC_R &= ~0xF;
		ADC0_PC_R |= 0x1; // 8) configure for 1M samples/sec   TODO
		ADC0_SSPRI_R = 0x0123; // 9) Sequencer 3 is highest priority
		ADC0_ACTSS_R &= ~0x008U; // 10) disable sample sequencer 3
		ADC0_EMUX_R &= ~0xF000; // 11) seq3 is software trigger
		ADC0_SSMUX3_R &= ~0x000F;
		ADC0_SSMUX3_R += 1; // 12) channel Ain1 (PE2)			TODO
		ADC0_SSCTL3_R = 0x6; // 13) no TS0 D0, yes IE0 END0
		ADC0_SAC_R = 0x6;	            // TODO: find the value of ? to Activate averaging for 64 samples
		ADC0_IM_R &= ~0x0008; // 14) disable SS3 interrupts
		ADC0_ACTSS_R |= 0x008; // 15) enable sample sequencer 33
}


//------------ADC0_InSeq3------------
// Busy-wait analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion

uint32_t ADC0_In(void){
	uint32_t result;
  // NotTODO: you don't need to initialize this function bc we don't use busy-wait but interrupt
	ADC0_PSSI_R = 0x0008; // 1) initiate SS3
  while ((ADC0_RIS_R & 0x08) == 0) {}// 2) wait for conversion done
  result = ADC0_SSFIFO3_R & 0xFFF; // 3) read result
  ADC0_ISC_R = 0x8; // 4) acknowledge completion

  return result;
}

