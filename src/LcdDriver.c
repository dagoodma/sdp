#include <xc.h>
#include <peripheral/timer.h>
#include "LcdDriver.h"

// Control signal data pins
// RW - Read/Write signal
#define  RW        LATDbits.LATD5
// RS signal
#define  RS        LATBbits.LATB15
// E signal
#define  E         LATDbits.LATD4

// Control signal pin direction 
#define  RW_TRIS   TRISDbits.TRISD5 
#define  RS_TRIS   TRISBbits.TRISB15
#define  E_TRIS    TRISDbits.TRISD4

// Data signals pins
#define  DATA      LATE
#define  DATA_TRIS TRISE


#define F_PB (Board_GetPBClock())
#define TIMER_FREQUENCY 1000


static void (*timerCallback)(void);


void LCD_driverInit(void)
{
	// Set initial states for the data and control pins to low.
	DATA &= 0xFF00;
	RW = 0;
	RS = 0;
	E = 0;

	// Set all data and control pins to outputs
	DATA_TRIS &= 0xFF00;
	RW_TRIS = 0;
	RS_TRIS = 0;
	E_TRIS = 0;

}

void LCD_timerInit(void (*timerCallbackFcn)(void), unsigned int prescalar) {

    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, prescalar);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_3);


    timerCallback = timerCallbackFcn;
    
    mT2IntEnable(1);
}

inline void LCD_driverSendCommand(char command)
{
	DATA &= 0xFF00;  // prepare RD0 - RD7
	DATA |= command; // command byte to lcd
	RS = 0;
	RW = 0;          // ensure RW is 0
	E = 1;           // toggle E line
	Nop();           // 300 ns of hold time, assuming 40 MHz clock
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	E = 0;
}

inline void LCD_driverSendData(char data)
{
	DATA &= 0xFF00;  // prepare RD0 - RD7
	DATA |= data;    // data byte to lcd
	RS = 1;          // assert register select to 1
	RW = 0;          // ensure RW is 0
	E = 1;
	Nop();           // 300 ns of hold time, assuming 40 MHz clock
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	Nop();
	E = 0;           // toggle E signal
	RS = 0;          // negate register select to 0
}



/**********************************************************************
 * Function: Timer2IntHandler
 * @return none
 * @remark This is the interrupt handler used by the LCD module to update
 *  the display by calling an update function every time this interrupt
 *  occurs.
 **********************************************************************/
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void)
{
    timerCallback();

    // Reset Timer 2 interrupt flag
    mT2ClearIntFlag();
}