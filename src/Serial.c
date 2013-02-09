
/*
 * File:   Serial.c
 * Edited by: dagoodma
 * Author: mdunne
 *
 *  This library is for using communicating over USB and UART.
 *
 * Edited on January 18, 2013, 11:42 PM
 * Created on November 10, 2011, 8:42 AM
 */

#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include "Board.h"
#include "Serial.h"
#include "Uart.h"
//#include <plib.h>
//#include <stdlib.h>


#define SERIAL_UART_ID         UART1_ID
#define SERIAL_UART_BAUDRATE   115200


/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

#define F_PB (Board_GetPBClock())
#define QUEUESIZE 512

/*******************************************************************************
 * PRIVATE DATATYPES                                                           *
 ******************************************************************************/



/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE VARIABLES                                                           *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

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

char Serial_init(void)
{
    #ifdef DEBUG
    printf("Intializing the Serial on UART %d.\n", SERIAL_UART_ID);
    #endif
    UART_init(SERIAL_UART_ID,SERIAL_UART_BAUDRATE);
    return SUCCESS;
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
    for (i = 0; i<sizeof (s); i++)
        Serial_putChar(s[i]);
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
