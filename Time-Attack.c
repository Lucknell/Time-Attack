#include "xparameters.h"
#include "xgpio.h"
#include "xutil.h"
#include "stdio.h"
#include <xtmrctr.h>
#include <xintc_l.h>
#include <xgpio.h>
#define LEDChan  1
#include "xgpio_l.h"





#define LCD_BASEADDR XPAR_LCD_IP_0_BASEADDR
#define INIT_DELAY 1000 //usec delay timer during initialization, important to change if clock speed changes
#define INST_DELAY 500 //usec delay timer between instructions
#define DATA_DELAY 250 //usec delay timer between data



//====================================================
unsigned int timer_count = 1;
unsigned int count = 1;
unsigned int one_second_flag = 0;
unsigned int turn = 0;
char values[];
unsigned int total =0;
unsigned int k,y;
//struct Bool {
//    int true;
//    int false;
//}
//


XGpio gpio;
void usleep(unsigned int delay)
{
unsigned int j, i;

for(i=0; i<delay; i++)
   for(j=0; j<26; j++);
}

void XromInitInst(void)
{
		XGpio_WriteReg(LCD_BASEADDR, 1, 0x00000003);
		usleep(1);
		XGpio_WriteReg(LCD_BASEADDR, 1, 0x00000043); //set enable and data
		usleep(1);
		XGpio_WriteReg(LCD_BASEADDR, 1, 0x00000003);
		usleep(INIT_DELAY);
}

void XromWriteInst(unsigned long inst1, unsigned long inst2)
{

	unsigned long printinst;

	printinst = 0x00000040 | inst1;

	XGpio_WriteReg(LCD_BASEADDR, 1, inst1); //write data
	usleep(1);
	XGpio_WriteReg(LCD_BASEADDR, 1, printinst); //set enable
	usleep(1);
	XGpio_WriteReg(LCD_BASEADDR, 1, inst1); //turn off enable
	usleep(1);

	printinst = 0x00000040 | inst2;

	XGpio_WriteReg(LCD_BASEADDR, 1, printinst); //set enable and data
	usleep(1);
	XGpio_WriteReg(LCD_BASEADDR, 1, inst2); //turn off enable

	usleep(INST_DELAY);

}

void XromWriteData(unsigned long data1, unsigned long data2)
{

	unsigned long rs_data, enable_rs_data;
	//bool busy=true;

	rs_data = (0x00000020 | data1); //sets rs, data1
	enable_rs_data = (0x00000060 | data1);

	XGpio_WriteReg(LCD_BASEADDR, 1, rs_data); //write data, rs
	usleep(1);
	XGpio_WriteReg(LCD_BASEADDR, 1, enable_rs_data); //set enable, keep data, rs
	usleep(1);
	XGpio_WriteReg(LCD_BASEADDR, 1, rs_data); //turn off enable
	usleep(1);

	rs_data = (0x00000020 | data2); //sets rs, data2
	enable_rs_data = (0x00000060 | data2); //sets rs, data2

	XGpio_WriteReg(LCD_BASEADDR, 1, enable_rs_data); //set enable, rs, data
	usleep(1);
	XGpio_WriteReg(LCD_BASEADDR, 1, rs_data); //turn off enable

	usleep(DATA_DELAY);
}





//==================================================================================
//
//								EXTERNAL FUNCTIONS
//
//==================================================================================

void XromMoveCursorHome(){
	XromWriteInst(0x00000000, 0x00000002);
}

void XromMoveCursorLeft(){
	XromWriteInst(0x00000001, 0x00000000);
}

void XromMoveCursorRight(){
	XromWriteInst(0x00000001, 0x00000004);
}

void XromLCDOn(){
	//xil_printf("DISPLAY ON\r\n");
		XromWriteInst(0x00000000, 0x0000000E);
}

void XromLCDOff(){
	//xil_printf("DISPLAY OFF\r\n");
		XromWriteInst(0x00000000, 0x00000008);
}

void XromLCDClear(){
	//xil_printf("DISPLAY CLEAR\r\n");
		XromWriteInst(0x00000000, 0x00000001);
		XromWriteInst(0x00000000, 0x00000010);
		XromMoveCursorHome();
}

void XromLCDInit(){
	//XGpio_SetDataDirection(LCD_BASEADDR, 1, 0x00000000); //Sets CHAR LCD Reg to Write Mode
	XGpio_WriteReg(LCD_BASEADDR, 1, 0x00000000); //Zeroes CHAR LCD Reg

	//LCD INIT
	usleep(15000);	//After VCC>4.5V Wait 15ms to Init Char LCD
		XromInitInst();
	usleep(4100); //Wait 4.1ms
		XromInitInst();
	usleep(100); //Wait 100us
		XromInitInst();
		XromInitInst();

	//Function Set
		XromWriteInst(0x00000002, 0x00000008);

	//Entry Mode Set
		XromWriteInst(0x00000000, 0x00000006);

	//Display Off
		XromWriteInst(0x00000000, 0x00000008);

	//Display On
		XromWriteInst(0x00000000, 0x0000000F);

	//Display Clear
		XromWriteInst(0x00000000, 0x00000001);
}


void XromLCDSetLine(int line){ //line1 = 1, line2 = 2

	int i;

	if((line - 1)) {
		XromMoveCursorHome();
		for(i=0; i<40; i++)
			XromMoveCursorRight();
	}
	else
		XromMoveCursorHome();

}

void XromLCDPrintChar(char c){
	XromWriteData(((c >> 4) & 0x0000000F), (c & 0x0000000F));
}

void XromLCDPrintString(char * line){

	int i=0;
		while(line[i]){
			XromLCDPrintChar(line[i]);
			i++;
		}

	return;
}

void XromLCDPrint2Strings(char * line1, char * line2){

	int i=0;

		XromLCDSetLine(1);

		for(i=0; i<16; i++){
			if(line1[i])
				XromLCDPrintChar(line1[i]);
			else break;
		}

		XromLCDSetLine(2);

		for(i=0; i<16; i++){
			if(line2[i])
				XromLCDPrintChar(line2[i]);
			else break;
		}
	return;
}


////////////////////////////////////////////////////////////////
// LogNum:  Converts number to character                    //
////////////////////////////////////////////////////////////////
void XromLCDPrintNum(unsigned int x, unsigned int base)
{
  static char hex[]="0123456789ABCDEF";
  char digit[10];
  int i;

  i = 0;
  do
  {
    digit[i] = hex[x % base];
    x = x / base;
    i++;
  } while (x != 0);

  while (i > 0)
  {
  	i--;
    XromLCDPrintChar(digit[i]);
  }
}

///////////////////////////////////////////////////////////////
// tft_WriteInt:  handles the negative case, calls LogNum   //
//                w/ unsigned value                         //
//////////////////////////////////////////////////////////////
void XromLCDPrintInt(unsigned int x)
{
  unsigned int val;

  if (x < 0)
  {
    XromLCDPrintChar('-');
    val = ((unsigned int) ~x ) + 1;
  }
  else
    val = (unsigned int) x;

  XromLCDPrintNum(val, 10);
}

unsigned int addValue(int pbCheck)
	{
		return total+=pbCheck;
	}

unsigned int printValue()
{
	return total;
}
void clearTotal()
{
	total = 0;
}
void clearLine2()
{
	XromLCDSetLine(2);
	XromLCDPrintString("                ");
	XromLCDSetLine(2);
}
void checkScore()
{
	if (turn == 0){
		turn++;
		if (total == 10)
	{
			total =0;
		XromLCDSetLine(2);
		clearLine2();
		XromLCDPrintString("Great Job!");
		y=1;
		for (k=0; k<9999; k++)
		{}
		clearLine2();
		XromLCDPrintChar('G');XromLCDPrintChar('R');XromLCDPrintChar('E');XromLCDPrintChar('A');XromLCDPrintChar('T');XromLCDPrintChar(' ');XromLCDPrintChar('J');XromLCDPrintChar('O');XromLCDPrintChar('B');XromLCDPrintChar('!');

	}
	else
	{
		XromLCDSetLine(2);
		clearLine2();
		y=1;
		XromLCDPrintString("FAILURE");
	}
	}
	else
	{
		if (turn == 2){
			turn++;
		if (total == 255)
				{
						total =0;
					XromLCDSetLine(2);
					clearLine2();
					XromLCDPrintString("Great Job!");
					y=1;
					for (k=0; k<9999; k++);
					clearLine2();
					XromLCDPrintChar('G');XromLCDPrintChar('R');XromLCDPrintChar('E');XromLCDPrintChar('A');XromLCDPrintChar('T');XromLCDPrintChar(' ');XromLCDPrintChar('J');XromLCDPrintChar('O');XromLCDPrintChar('B');XromLCDPrintChar('!');
				}
				else
				{
					XromLCDSetLine(2);
					clearLine2();
					y=1;
					XromLCDPrintString("FAILURE");
				}
				}
		else
		{
			if (turn == 1){
				turn++;
				total =0;
				clearLine2();
				XromLCDPrintString("       !FF!");
				//XromLCDPrintInt(round2);
			}
			else
			{
				XromLCDClear();
				XromLCDPrintString("GAME OVER!");
				XromLCDSetLine(2);
				XromLCDPrintString("GAME OVER!");
			}
		}
	}
}

void timer_int_handler(void * baseaddr_p) {
	/* Add variable declarations here */
	unsigned int csr;
	/* Read timer 0 CSR to see if it raised the interrupt */
	csr = XTmrCtr_GetControlStatusReg(XPAR_DELAY_BASEADDR,0);
	/* If the interrupt occured, then increment a counter and set one_second_flag */
	if (csr & XTC_CSR_INT_OCCURED_MASK)
	{
		count++;
		one_second_flag = 1;
	}
	/* Display the count on the LEDS and print it using the UART */
	//XGpio_DiscreteWrite(&gpio,LEDChan,count);
	//xil_printf("Count Value is : %x\r\n", count);
	/* Clear the timer interrupt */
	XTmrCtr_SetControlStatusReg(XPAR_DELAY_BASEADDR,0,csr);
}
void ISRTimer()
{
	count =0;
}
void clearTimer()
{
	count =1;
}
void resetGame()
{
	count =1;
	turn =0;
	total =0;
	XromLCDClear();
	XromLCDPrintChar(' ');
	XromLCDPrintChar('!');
	XromLCDPrintString("Time  Attack");
	XromLCDPrintChar('!');
	XromLCDSetLine(2);
	XromLCDPrintString("       !A");
	XromLCDPrintChar('!');

}
unsigned int correctInput(int check)
{
	if (check == 10&& turn ==0)
	{

		return 1;

	}
	else {if(check == 255 && turn >0 )
	{

		return 1;
	}
	else {return 0;}
	}

	}
void simonSays(int buttoncheck)
{
	XromLCDClear();
	XromLCDPrintString("  !SIMON SAYS!");
	XromLCDSetLine(2);
	XromLCDPrintString("Press North");

	if (buttoncheck == 1)
	{
		clearLine2();
		XromLCDPrintString("  GREAT JOB");
	}
	else
	{
		clearLine2();
		XromLCDPrintString("  FAILURE");
	}
}

int main (void)
{
	int count_mod_3;
	XIntc_RegisterHandler(XPAR_XPS_INTC_0_BASEADDR,
	                             XPAR_XPS_INTC_0_DELAY_INTERRUPT_INTR,
	                             (XInterruptHandler) timer_int_handler,
	                             (void *)XPAR_DELAY_BASEADDR);

	  /* Initialize and set the direction of the GPIO connected to LEDs */
	  XGpio_Initialize(&gpio, XPAR_LEDS_8BIT_DEVICE_ID);
	  XGpio_SetDataDirection(&gpio,LEDChan, 0);

	  /* Start the interrupt controller */
	  XIntc_MasterEnable(XPAR_XPS_INTC_0_BASEADDR);
	  XIntc_EnableIntr(XPAR_XPS_INTC_0_BASEADDR, 0x1);

	  /* Set the gpio as output on high 8 bits (LEDs)*/
	  XGpio_WriteReg(XPAR_LEDS_8BIT_DEVICE_ID,LEDChan, ~count);
	  //xil_printf("The value of count = %d\n\r", count);

	  /* Set the number of cycles the timer counts before interrupting */
	  XTmrCtr_SetLoadReg(XPAR_DELAY_BASEADDR, 0, (timer_count*timer_count+1) * 80000000);

	  /* Reset the timers, and clear interrupts */
	  XTmrCtr_SetControlStatusReg(XPAR_DELAY_BASEADDR, 0, XTC_CSR_INT_OCCURED_MASK | XTC_CSR_LOAD_MASK );

	  /* Enable timer interrupts in the interrupt controller */
	  XIntc_EnableIntr(XPAR_DELAY_BASEADDR, XPAR_DELAY_INTERRUPT_MASK);

	  /* Start the timers */
	  XTmrCtr_SetControlStatusReg(XPAR_DELAY_BASEADDR, 0, XTC_CSR_ENABLE_TMR_MASK | XTC_CSR_ENABLE_INT_MASK |
							XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_DOWN_COUNT_MASK);
	  /* Enable MB interrupts */
	  microblaze_enable_interrupts();

   XGpio dip, push;
	int i, psb_check, dip_check,compare;

	// define instance pointer for LEDs_8Bit device
	XGpio LEDs8_Bit;


   XGpio_Initialize(&dip, XPAR_DIP_DEVICE_ID);
	XGpio_SetDataDirection(&dip, 1, 0xffffffff);

	XGpio_Initialize(&push, XPAR_PUSH_DEVICE_ID);
	XGpio_SetDataDirection(&push, 1, 0xffffffff);

	// initialize and set data direction for LEDs_8Bit device
	XGpio_Initialize(&LEDs8_Bit, XPAR_LEDS_8BIT_DEVICE_ID);
	XGpio_SetDataDirection(&LEDs8_Bit, 1, 0x0);
	compare =99;
	XromLCDInit();
	XromLCDOn();
	XromLCDClear();


	XromLCDPrintChar(' ');
	XromLCDPrintChar('!');
	XromLCDPrintString("Time  Attack");
	XromLCDPrintChar('!');
	XromLCDSetLine(2);
	XromLCDPrintString("       !A");
	XromLCDPrintChar('!');
	
	while (1)
	{
	  psb_check = XGpio_DiscreteRead(&push, 1);
	  //xil_printf("Push Buttons Status %x\r\n", psb_check);
	  if(compare != psb_check){
	  compare = psb_check;
		  addValue(psb_check);
	  }
	  //correctInput(printValue());
	  xil_printf("Total is %x\r\n", printValue());
	  dip_check = XGpio_DiscreteRead(&dip, 1);
	  //xil_printf("DIP Switch Status %x\r\n", dip_check);
	  if (dip_check == 128)
	  {
		clearTotal();
	  }
	  if (dip_check == 1)
	  {
		clearTimer();

	  }
	  if (dip_check == 2)
	  	  {
	  		ISRTimer();
	  	  }
	  if (dip_check == 64 )
	  	 {
	  	  	resetGame();
	  	 }
	  if(dip_check == 255)
	  {
		  simonSays(psb_check);
	  }

	  // output dip switches value on LEDs_8Bit device
	  XGpio_DiscreteWrite(&LEDs8_Bit, 1, dip_check);

	  if(one_second_flag){
	  		count_mod_3 = count % 15;
	  		if(count_mod_3 == 0|| correctInput(printValue())==1||y==1){
	  			y=0;
	  			xil_printf("Interrupt taken at %d seconds \n\r",count);
	  			checkScore();
	  		}
	  			one_second_flag=0;
	  		}


	  for (i=0; i<999999; i++);
	}
}
