/**********************************************************************
 Module
 Lcd.c

 Author: Bryant Mairs, Pavlo ManoviDavid Goodman

 History
 When                   Who         What/Why
 --------------         ---         --------
4/17/2013   10:07 AM    dagoodma    Rewrote from Pavlo's code.
***********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <peripheral/timer.h>
#include "Lcd.h"
#include "Serial.h"
#include "Uart.h"
#include "Board.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
// Control signal data pins
// RW - Read/Write signal
#define  RW        LATDbits.LATD2
// RS signal
#define  RS        LATDbits.LATD1
// E signal
#define  E         LATDbits.LATD9

// Control signal pin direction
#define  RW_TRIS   TRISDbits.TRISD2
#define  RS_TRIS   TRISDbits.TRISD1
#define  E_TRIS    TRISDbits.TRISD9

// Data signals pins
#define  DATA      LATE
#define  DATA_TRIS TRISE


/**
 * This macro simplifies updating the prescalar register associated with the
 * timers period. If used with the Timer2Init() function it will set the delay
 * between when the Timer 2 interrupts are called.
 */
#define SET_TIMER2_DELAY(x) (PR2 = (x))

/**
 * 1.64ms delay
 * This macro defines a short delay of 1.53ms (129 counts of 156,250 Hz clock)
 * in units that are appropriate for the prescalar register of a timer. It can
 * therefore be passed directly into the SET_TIMER2_DELAY() macro.
 */
#define LONG_DELAY  129

/**
 * This macro is similar to LONG_DELAY above, but defines a short 39us delay
 * (4 counts of 156,250 Hz clock).
 */
#define SHORT_DELAY 14
//#define LONG_DELAY  239
//#define SHORT_DELAY 6

/***********************************************************************
 * PRIVATE PROTOTYPES                                                  *
 ***********************************************************************/

enum LcdStates {
// Init states
	initStartState,
	enableDisplayState,
	setEntryModeState,
        clearDisplayState,

// Update states
	waitForUpdateState,
        moveCursorLineState,
	updateLineState
}  lcdState;


static void runSM(void);
static char getNextCharacter(void);
static void setLine(uint8_t line);
void driverInit(void);
void timer2Init(void (*timerCallbackFcn)(void), unsigned int prescalar);
inline void sendCommand(char command);
inline void sendData(char data);


/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/

static char pendingUpdate = true; // A boolean for whether the data has changed since the last call
                                  // to LcdUpdatestate().
static const char defaultLine[] = "                    "; // A blank line that is muxed with the data a
                                                      // user wants to set the line too so that the

static char lineBuffer[LCD_LINE_TOTAL][LCD_LINE_LENGTH + 1];

static uint8_t currentLine = 0, currentLineWrite = 0, currentCharWrite = 0;

static void (*timerCallback)(void);

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


void LCD_init(void)
{
    // First initialize the low-level LCD driver
    driverInit();
    // Next configure Timer2 for proper interrupts and initialize it to a SHORT_DELAY period.
    timer2Init(runSM, SHORT_DELAY);
    lcdState = initStartState;

}

void LCD_clearDisplay() {
    currentLineWrite = 0;
    currentCharWrite = 0;
    uint8_t i;
    for (i=0; i < LCD_LINE_TOTAL; i++) {
        strncpy(lineBuffer[i], defaultLine, LCD_LINE_LENGTH);
    }
    lcdState = clearDisplayState;
    DELAY(10);
}

void LCD_writeString(const char *str) {
    uint8_t i = 0;
    uint8_t len = (strlen(str) < LCD_CHARACTER_TOTAL)? strlen(str)
        : LCD_CHARACTER_TOTAL;
    char tmp;
    //printf("Writing line of length: %d\n",len);
    // Determine newline count
    for(i = 0; i < len; i++) {
        tmp =  str[i];
        if (currentCharWrite >= (LCD_LINE_LENGTH) || tmp == '\n')
            tmp = '\0';
        if (currentCharWrite < LCD_LINE_LENGTH)
            lineBuffer[currentLineWrite][currentCharWrite] =  tmp;

        if (str[i] == '\0')
            break;

        if (str[i] == '\n') {
            //printf("found a newline i:%d, j:%d, line:%d\n",i,j,line);
            currentLineWrite++;
            currentLineWrite = (currentLineWrite >= LCD_LINE_TOTAL)? 0 : currentLineWrite; // wrap around from top
            currentCharWrite = 0;
        }
        else {
            currentCharWrite++;
        }
    }
    //printf("Line1: %s\n Line2: %s\n Line3: %s\n Line4: %s\n",
    //    lineBuffer[0], lineBuffer[1], lineBuffer[2], lineBuffer[3]);
    pendingUpdate = true;
}

void LCD_setPosition(uint8_t line, uint8_t col) {
    if (line < LCD_LINE_TOTAL && col < LCD_LINE_LENGTH) {
        currentLineWrite = line;
        currentCharWrite = col;
    }

}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/
/*
static void lcdNewline(uint8_t pos) {
    if ( pos < LCD_START_LINE2 )
        addressCounter = LCD_START_LINE2;
    else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE3) )
        addressCounter = LCD_START_LINE3;
    else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE4) )
        addressCounter = LCD_START_LINE4;
    else
        addressCounter = LCD_START_LINE1;

    sendCommand((1<<LCD_DDRAM)+addressCounter);
}*/

/*************************************************************************
Display character at current cursor position
Input:    character to be displayed
Returns:  none
*************************************************************************/
/*
void lcdPutc(char c)
{
    uint8_t pos;


    pos = lcd_waitbsy();   // read busy-flag and address counter
    if (c=='\n')
    {
        lcdNewline(pos);
    }
    else
    {
        lcd_waitbusy();
        lcd_write(c, 1);
    }

}*//* lcd_putc */


/**
 * GetNextTopLineChar() muxes the default line and the user input into a single
 * 16-character string. This is done in a separate function instead of within
 * the line-setters in order to store the original string unprocessed. This
 * allows the getters to be trivial.
 * This function remembers its state and as such will return each character
 * one-at-a-time until it reaches the end at which point it will reset its
 * internal state and return NULL.
 */
static char getNextCharacter(void) {
	static char foundEndOfLine = false;
        static int currentCharacter = 0;

	char rv;

	if (!foundEndOfLine) {
            if (lineBuffer[currentLine][currentCharacter] == '\0') {
                foundEndOfLine = true;
                rv = defaultLine[currentCharacter];
            }
            else {
                rv = lineBuffer[currentLine][currentCharacter];
            }
	}
        else {
            // At the end, so use spaces to fill display
            rv = defaultLine[currentCharacter];
	}
	currentCharacter++;

	// Once we've reached the end of the line, reset everything
	if (rv == '\0') {
            currentCharacter = 0;
            foundEndOfLine = false;
	}
	
	return rv;
}


static void setLine(uint8_t line) {
    if (line >= LCD_LINE_TOTAL)
        return;

    char cmd = (1<<LCD_DDRAM);
    switch (line) {
        case 0:
            cmd += MoveCursorToLine1;
            break;
        case 1:
            cmd += MoveCursorToLine2;
            break;
        case 2:
            cmd += MoveCursorToLine3;
            break;
        case 3:
            cmd += MoveCursorToLine4;
            break;
    }
    sendCommand(cmd);
}

// ------------------------------ LCD Driver code ------------------------------

void driverInit(void)
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

void timer2Init(void (*timerCallbackFcn)(void), unsigned int prescalar) {

    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, prescalar);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_3);


    timerCallback = timerCallbackFcn;

    mT2IntEnable(1);
}

inline void sendCommand(char command)
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

inline void sendData(char data)
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
void __ISR(_TIMER_2_VECTOR, ipl3) Timer2IntHandler(void) {
    timerCallback();

    // Reset Timer 2 interrupt flag
    mT2ClearIntFlag();
}

// ----------------------------- State machine -------------------------------
/**
 * This internal function implements the state machine logic used when sending commands to the LCD.
 * It has both an initialization state machine and an update state machine, so the first few times
 * it will be called, it will run through the initialization portion. After that it enters the
 * update portion. It has been optimized so that only the necessary lines are updated depending on
 * what data has been changed since the last time it was called.
 */
static void runSM(void)
{
	switch (lcdState) {

            // LCD initialization states
            case initStartState:
                    SET_TIMER2_DELAY(LONG_DELAY);
                    sendCommand(SetDisplayMode);
                    lcdState = enableDisplayState;
                    break;

            case enableDisplayState:
                    SET_TIMER2_DELAY(SHORT_DELAY);
                    sendCommand(EnableDisplay);
                    lcdState = setEntryModeState;
                    break;
            case setEntryModeState:
                    SET_TIMER2_DELAY(SHORT_DELAY);
                    sendCommand(SetEntryMode);
                    lcdState = clearDisplayState;
                    break;
            case clearDisplayState:
                    SET_TIMER2_DELAY(LONG_DELAY);
                    sendCommand(ClearDisplay);
                    lcdState = waitForUpdateState;
                    break;
            // LCD update states
            case waitForUpdateState:
                if (pendingUpdate) {
                    SET_TIMER2_DELAY(SHORT_DELAY);
                    lcdState = moveCursorLineState;
                    currentLine = 0;
                    pendingUpdate = false;
                }
                break;
            case moveCursorLineState:
                SET_TIMER2_DELAY(SHORT_DELAY);
                setLine(currentLine);
                lcdState = updateLineState;
                break;
            case updateLineState:
                {
                    char tmp = getNextCharacter();
                    if (tmp != '\0') {
                        SET_TIMER2_DELAY(SHORT_DELAY);
                        sendData(tmp);
                    }
                    else {
                        SET_TIMER2_DELAY(LONG_DELAY);
                        currentLine++;
                        if (currentLine >= LCD_LINE_TOTAL) {
                            currentLine = 0;
                            lcdState = waitForUpdateState;
                        }
                        else {
                            lcdState = moveCursorLineState;
                        }
                    }
                }
                break;
            default:
                    lcdState = initStartState;
                    break;
	}
}


// ------------------------------ Test Harnesses ------------------------------

//#define LCD_TEST
#ifdef LCD_TEST
#include "Board.h"

#define WRITE_DELAY     2000

int main(void)
{
    Board_init();
    Serial_init();
    LCD_init();
    Timer_init();
    printf("Testing the LCD display.\nYou should see: Hello World!\n");

    LCD_writeString("Hello World!\nLine2\nLine3\nLine4\n");
    Timer_new(TIMER_TEST,WRITE_DELAY);
    while (!Timer_isExpired(TIMER_TEST)) {
        asm("nop");
    }
    printf("Overwriting line 1\n");
    LCD_writeString("Testing overtop line1.\n");

    Timer_new(TIMER_TEST,WRITE_DELAY);
    while (!Timer_isExpired(TIMER_TEST)) {
        asm("nop");
    }
    printf("Clearing Display\n");
    LCD_clearDisplay();

    Timer_new(TIMER_TEST,WRITE_DELAY);
    while (!Timer_isExpired(TIMER_TEST)) {
        asm("nop");
    }
    LCD_writeString("Here.we.go\n");
    LCD_writeString("CLEARED!\n");
    LCD_writeString("CLEARED2!\n");
    LCD_writeString("CLEARED3!\n");

    Timer_new(TIMER_TEST,WRITE_DELAY);
    while (!Timer_isExpired(TIMER_TEST)) {
        asm("nop");
    }

    LCD_clearDisplay();
    
    LCD_writeString("N:        E:        \n");
    char tmp[10];
    LCD_setPosition(0,3);
    sprintf(tmp,"%.2f",12.5f);
    LCD_writeString(tmp);
    
    LCD_setPosition(0,13);
    sprintf(tmp,"%.2f",-0.32f);
    LCD_writeString(tmp);

    return SUCCESS;
}

#endif
