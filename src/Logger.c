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
#include <peripheral/uart.h>
#include "Board.h"
#include "Logger.h"
#include "Uart.h"
#include "Timer.h"
#include "Serial.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define DEBUG

#define LOGGER_UART_ID         UART2_ID
#define LOGGER_UART_BAUDRATE   9600


#define STARTUP_TIMEOUT_DELAY   1500

#define STARTUP_CHARACTERS      3

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

/**********************************************************************
 * Function: Logger_init
 * @return none
 * @remark Initializes the OpenLog logger. Expects new file mode. File names
 *  look like: LOG#####.TXT. The ##### number increases every time you power
 *  up OpenLog. This number is stored in EEPROM, and must be reset once 65533
 *  is reached, or else the logger will fail to initialize.
 *  TODO: Add handling for resetting the counter.
 ***********************************************************************/
char Logger_init() {
    #ifdef DEBUG
    printf("Intializing the Logger on UART %d.\n", LOGGER_UART_ID);
    #endif
    UART_init(LOGGER_UART_ID,LOGGER_UART_BAUDRATE);

    char expect[STARTUP_CHARACTERS] = {'1', '2', '<'};
    char *error[STARTUP_CHARACTERS] = {"UART", "SD card", "system"};

    /* Clear the buffer until we get the UART initialized message
        or until timeout. */
    uint16_t in; uint8_t index = 0;
    Timer_new(TIMER_LOGGER, STARTUP_TIMEOUT_DELAY);
    while (index < STARTUP_CHARACTERS) {
        while (((char)in) != expect[index]) {
            in = UART_getChar(LOGGER_UART_ID);
            if (Timer_isExpired(TIMER_LOGGER)) {
                #ifdef DEBUG
                printf("Logger failed initializing %s.\n", error[index]);
                #endif
                return FAILURE;
            }
        }
        Timer_new(TIMER_LOGGER, STARTUP_TIMEOUT_DELAY);
        index++;
    }

    #ifdef DEBUG
    printf("Logger ready for data.\n");
    #endif
    return SUCCESS;
}


/**********************************************************************
 * Function: Logger_write
 * @param String to write to the log.
 * @return none
 * @remark Data is logged to a file created when the device is powed on.
  **********************************************************************/
void Logger_write(char *str) {
    UART_putString(LOGGER_UART_ID, str, sizeof(str));
}



//#define LOGGER_TEST
#ifdef LOGGER_TEST

int main(void) {
    Board_init();
    Timer_init();
    Serial_init();
    if (Logger_init() != SUCCESS)
        return FAILURE;

    char str[] = "OpenLog logger is working!\n";
    printf("Writing: %s", str);

    Logger_write(str);

    return SUCCESS;
}

#endif

