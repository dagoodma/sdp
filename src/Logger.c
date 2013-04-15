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
#include "Ports.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define DEBUG

#define LOGGER_UART_ID         UART1_ID
#define LOGGER_UART_BAUDRATE   9600


#define STARTUP_TIMEOUT_DELAY   3500

#define STARTUP_CHARACTERS      3

#define LOGGER_RESET_TRIS       PORTZ08_TRIS
#define LOGGER_RESET            PORTZ08_LAT

#define RESET_LOGGER

/*******************************************************************************
 * PRIVATE DATATYPES                                                           *
 ******************************************************************************/



/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/
static BOOL hasNewByte();
static uint8_t readByte();
static void resetLogger();

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
    LOGGER_RESET_TRIS = OUTPUT;

    char expect[STARTUP_CHARACTERS] = {'1', '2', '<'};
    char *error[STARTUP_CHARACTERS] = {"UART", "SD card", "system"};

    /* Clear the buffer until we get the UART initialized message
        or until timeout. */
    uint16_t in; uint8_t index = 0;
    Timer_new(TIMER_LOGGER, STARTUP_TIMEOUT_DELAY);
    resetLogger();
    while (index < STARTUP_CHARACTERS) {
        while (((char)in) != expect[index]) {
            if (hasNewByte()) {
                in = readByte();
                while (!Serial_isTransmitEmpty()) { asm("nop"); }
                //printf("Got %x\n", in);
            }
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
    UART_putString(LOGGER_UART_ID, str, strlen(str));
}




/**********************************************************************
 * Function: hasNewMessage
 * @return Returns true if a new message is ready to be read
 * @remark
 **********************************************************************/
static BOOL hasNewByte() {
    return !UART_isReceiveEmpty(LOGGER_UART_ID);
}

/**********************************************************************
 * Function: readByte
 * @return SUCCESS, FAILURE, or ERROR.
 * @remark Reads a byte if one is available.
 **********************************************************************/
static uint8_t readByte() {
    // Read a new byte from the UART or return FAILURE
    if (hasNewByte())
        return UART_getChar(LOGGER_UART_ID);

    return FALSE;
}

static void resetLogger() {
    LOGGER_RESET = 0;
    DELAY(1);
    LOGGER_RESET = 1;
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

