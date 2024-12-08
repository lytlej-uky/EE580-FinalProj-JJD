/* Modified by Dr. Salehi
 2023

 */

void HC05program(uint32_t mod ,char *slaveaddress);

//-------------------HC05_Init --------------
// initializes the module as a client
// Inputs: none
// Outputs: none
void HC05_Init(uint32_t baud);

//------------------- HC05_InitUART-------------------
// intializes uart and gpio needed to communicate with HC05
// Configure UART1 for serial full duplex operation
// Inputs: baud rate (e.g., 115200 or 9600)
//         echo to UART0?
// Outputs: none
void HC05_InitUART(uint32_t baud, int echo);

