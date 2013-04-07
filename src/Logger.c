/*
 * File:   Logger.c
 * Author: dagoodma
 *
 * This library is for communicating with an OpenLog device over UART.
 *
 * Created on April 6, 2013, 4:49 PM
 */

#include <xc.h>
#include <stdint.h>
#include "Board.h"
#include "Uart.h"

#define LOGGER_UART_ID         UART1_ID
#define LOGGER_UART_BAUDRATE   9600


/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE DATATYPES                                                           *
 ******************************************************************************/



/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE VARIABLES                                                           *
 ******************************************************************************/
BOOL canBlock = TRUE;
const char *buffer;
uint16_t bufferIndex = 0;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**********************************************************************
 * Function: GPS_init
 * @return none
 * @remark Initializes the GPS.
 **********************************************************************/

/****************************************************************************
 Function
     Serial_Init

 Parameters
     none

 Returns
     None.

 Description
    Initializes the UART subsystem to 115200 and sets up the circular buffer
 Notes
     None.

 Author
 Max Dunne, 2011.11.10 
 ****************************************************************************/

char Serial_init()
{
    #ifdef DEBUG
    printf("Intializing the Serial on UART %d.\n", SERIAL_UART_ID);
    #endif
    UART_init(SERIAL_UART_ID,SERIAL_UART_BAUDRATE);
    return SUCCESS;
}

char Serial_initSM()
{
    canBlock = TRUE;
    return Serial_init();
}

/****************************************************************************
 Function
     Serial_putChar

 Parameters
    char ch, the char to be sent out the serial port

 Returns
     None.

 Description
    adds char to the end of the circular buffer and forces the interrupt flag high is nothing is currently transmitting
 Notes


 Author
    Max Dunne, 2011.11.10
 ****************************************************************************/

void Serial_putChar(char ch)
{
    UART_putChar(SERIAL_UART_ID, ch);
}

/****************************************************************************
 Function
     Serial_runSM

 Parameters
     None.

 Returns
     None.

 Description
    Sends one character from buffer if there are any characters and the trasmit
 *  buffer is empty.


 Author
    Max Dunne, 2011.11.10
 ****************************************************************************/
void Serial_runSM() {
    if (Serial_isTransmitEmpty() && bufferIndex < sizeof(buffer))
        Serial_putChar(buffer[bufferIndex++]);
}


/****************************************************************************
 Function
     Serial_getChar

 Parameters
     None.

 Returns
    ch - char from the serial port

 Description
    reads first character from buffer or returns 0 if no chars available
 Notes
     

 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
char Serial_getChar(void)
{
    return UART_getChar(SERIAL_UART_ID);
}

/****************************************************************************
 Function
     _mon_putc

 Parameters
    c - char to be sent

 Returns
    None.

 Description
    overwrites weakly define extern to use circular buffer instead of Microchip functions
 
 Notes
     

 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void _mon_putc(char c)
{
    Serial_putChar(c);
}

/****************************************************************************
 Function
     _mon_puts

 Parameters
    s - pointer to the string to be sent

 Returns
    None.

 Description
    overwrites weakly defined extern to use circular buffer instead of Microchip functions

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void _mon_puts(const char* s)
{
    int i;
    if (canBlock) {
        for (i = 0; i<sizeof (s); i++)
            Serial_putChar(s[i]);
    }
    else {
        bufferIndex = 0;
    }
}

/****************************************************************************
 Function
     _mon_getc

 Parameters
    canblock - unused variable but required to match Microchip form

 Returns
    None.

 Description
    overwrites weakly defined extern to use circular buffer instead of Microchip functions

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
int _mon_getc(int canblock)
{
    if (UART_isReceiveEmpty(SERIAL_UART_ID))
        return -1;
    return Serial_getChar();
}

/****************************************************************************
 Function
    Serial_isReceiveEmpty

 Parameters
     None.

 Returns
    TRUE or FALSE

 Description
    returns the state of the receive buffer
 Notes
     

 Author
 Max Dunne, 2011.12.15
 ****************************************************************************/
char Serial_isReceiveEmpty(void)
{
    return UART_isReceiveEmpty(SERIAL_UART_ID);
}

/****************************************************************************
 Function
    Serial_isTransmitEmpty

 Parameters
     None.

 Returns
    TRUE or FALSE

 Description
    returns the state of the transmit buffer
 Notes


 Author
 Max Dunne, 2011.12.15
 ****************************************************************************/
char Serial_isTransmitEmpty(void)
{
    return UART_isTransmitEmpty(SERIAL_UART_ID);
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/




//#define SERIAL_TEST
#ifdef SERIAL_TEST
#include "Serial.h"
#include "Board.h"
#include "Uart.h"
#include <GenericTypeDefs.h>
//#include <plib.h>

int main(void)
{
    Board_init();
    Serial_init();
    printf("\r\nUno Serial Test Harness\r\nAfter this Message the terminal should mirror anything you type.\r\n");

    unsigned char ch = 0;
    while (1) {
        if (Serial_isTransmitEmpty() == TRUE)
            if (Serial_isReceiveEmpty() == FALSE)
                Serial_putChar(Serial_getChar());
    }

    return 0;
}

#endif

//#define SERIAL_TEST2
#ifdef SERIAL_TEST2
#include "Serial.h"
#include "Board.h"
#include "Uart.h"
#include <GenericTypeDefs.h>
//#include <plib.h>

int main(void)
{
    Board_init();
    Serial_initSM();
    printf("\r\nUno Serial Test Harness\r\nAfter this Message the terminal should mirror anything you type.\r\n");

    while (1) {
        Serial_runSM();
    }

    return 0;
}

#endif

