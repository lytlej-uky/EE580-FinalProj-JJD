// Contains the four functions that you need to complete.
// Replace "//Add your code here" with appropriate code

// Hardware Configuration
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

#include <stdint.h>
#include "ST7735.h"
#include "fixed.h"

static int32_t xMin, xMax, yMin, yMax; // the coordinate for drawing

/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    " 2.345"  
 -8100    "-8.100"
  -102    "-0.102" 
    31    " 0.031" 
-12345    " *.***"
 */ 
void ST7735_sDecOut3(int32_t n) {
  if (n > 9999 | n < -9999) {  // out of range
     ST7735_OutString(" *.***"); // print error message
	} else {
		if (n < 0) { // output - and negate if negative
			ST7735_OutChar('-');
			n = -n;
		} else {
			ST7735_OutChar(' ');
		}
		// extract each digit, +48 to get the ascii code of that digit
		// extract 1000s digit and print using function ST7735_OutChar()
		ST7735_OutChar(n/1000 + 48);
		ST7735_OutChar('.');    // To print the DOT
		// extract 100s digit and print using function ST7735_OutChar()
		n = n - n/1000*1000;
		ST7735_OutChar(n/100 + 48);
		// extract 10s and 1s digits and print using function ST7735_OutChar()
		//Add your code here
		ST7735_OutChar((n%100)/10 + 48);
		ST7735_OutChar(n%10 + 48);
	}
	
}

/**************ST7735_uBinOut8***************
 unsigned 32-bit binary fixed-point with a resolution of 1/256. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 256000, it signifies an error. 
 The ST7735_uBinOut8 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
     0	  "  0.00"
     2	  "  0.01"
    64	  "  0.25"
   100	  "  0.39"
   500	  "  1.95"
   512	  "  2.00"
  5000	  " 19.53"
 30000	  "117.19"
255997	  "999.99"
256000	  "***.**"
*/
void ST7735_uBinOut8(uint32_t n) {
  if (n >= 256000) {  // out of range
    //Add your code here // Display Error message using *
		ST7735_OutString("***.**"); // print error message
	} else {
		// since resolution is 256 = 2^8, the last 8 digits are fraction
		uint32_t intPart = n >> 8; //Add your code here // extract the integer part of n
		uint32_t fracPart = n - (intPart << 8);//Add your code here // extract the fraction part of n
		fracPart = fracPart * 100 / 256; // Convert the binary fraction to a decimal fraction. Times 100 because 2 decimal places are used
		
		uint16_t leading0 = 1; // the boolean to track the leading 0's since we do not want to display them
		// intPart three digits, fracPart 2 digits
		// extract and print 100s digit of the integer part. if 100s digit is zero, print a blank digit. Change leading0=0 if 100s digit is not zero.
		uint16_t currentNum = intPart / 100;
		if (currentNum == 0) {
			ST7735_OutChar(' ');
		} else {
			ST7735_OutChar(currentNum + 48);
			leading0=0;
		}
		
		// extract and print 10s digit of the integer part. if 10s digit is zero, print a blank digit. Change leading0=0 if 10s digit is not zero.
		//Add your code here
		currentNum = (intPart % 100) / 10;
		if (currentNum == 0 && leading0 == 1) {
			ST7735_OutChar(' ');
		} else {
			ST7735_OutChar(currentNum + 48);
			leading0=0;
		}
		
		// extract and print ones digit of the integer part.
		//Add your code here
		ST7735_OutChar((intPart % 10) + 48);

		// extract and print 10s and 1s digits of the  fraction part.
		//Add your code here
		ST7735_OutChar('.');
		ST7735_OutChar((fracPart % 100)/10 + 48);
		ST7735_OutChar((fracPart % 10) + 48);	
	}
}

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) {
  ST7735_PlotClear(minY, maxY); //Add your code here // Access the ST7735_PlotClear to clear the plot
	ST7735_FillScreen(ST7735_BLACK); //Add your code here // Use ST7735_FillScreen to fill the screen with the Black color.
	ST7735_SetCursor(0, 0); //Add your code here // Use ST7735_SetCursor to set the cursor to coordinate (0,0)
	ST7735_DrawString(0, 0, title, ST7735_WHITE); //Add your code here // Use ST7735_DrawString to print(Here Draw) the title string. 
	
	//No change for following 4 lines
	xMin = minX;
	xMax = maxX;
	yMin = minY;
	yMax = maxY;
}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data. This function should map (and scale) the drawing window of [(xMin,yMin) to (xMax,yMax)] to window with [(0,32) to (127,159)] corners
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none`
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]) {
  for(int i=0;i<num;i++){
		if (bufX[i] > xMin && bufX[i] <xMax && bufY[i]>yMin && bufY[i]<yMax) { // check if current reading bufX/Y is withing min and Max limit
			// Now code the two formulas for a and b (basically, we will be scale the input window with [(xMin,yMin) to (xMax,yMax)] to window with [(0,32) to (127,159)] corners
			// a = ( (bufX[i] - MinofX)/(MaxofX-MinofX) * 127)  // x-coordinate
			// b = ? // y-coordinate
			// use  ST7735_DrawPixel to draw (a,b)
			// add your code here
			uint32_t x_val = ( ((bufX[i] - xMin)*127)/(xMax-xMin));  // x-coordinate
			uint32_t y_val = (159 - ( (bufY[i] - yMin))*127/(yMax-yMin));  // y-coordinate
			ST7735_DrawPixel(x_val, y_val, ST7735_WHITE); // use  ST7735_DrawPixel to draw (a,b)
		}
  }
}
