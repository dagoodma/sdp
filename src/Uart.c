/**********************************************************************
 Module
   Uart.c

 Revision
   1.0.0

 Description
 Code for initilazing and running the UART

 Notes

 History
 When           Who         What/Why
 -------------- ---         --------
 12-18-13 8:10 PM jash    Create File
***********************************************************************/


#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include "Uart.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define PERIPHERAL_CLOCK 80000000L

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: UART_init()
 * @param id: identifies the UART module we want to initialize
 *        baudRate: The baudRate we want to initialize the function too.
 * @return a UINT32 value with the actual baudrate initialized. Check against
 *  desired to see if the system can handle the desired baud rate.
 * @remark Initializes the UART to the specific ID and BaudRate given as
 *  functions to the system. Will return the actual baudrate initialzied.
 **********************************************************************/

UINT32 UART_init(UART_MODULE id, UINT32 baudRate){
    UARTConfigure(id,UART_ENABLE_PINS_TX_RX_ONLY);
    UINT32 actual_baudRate = UARTSetDataRate(id, PERIPHERAL_CLOCK, baudRate);
    UARTSetFifoMode(id, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTEnable(id, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
    return actual_baudRate;
}

/**********************************************************************
 * Function: UART_sendData()
 * @param id: identifies the UART module we want to send data over
 *        data: The location of the bytes of data we want to send
 *        size: The number of bytes we want to send over the UART
 * @remark Uses blocking code to send data over the UART. Does NOT
 *  check the bytes to make sure they are not junk.
 **********************************************************************/
void UART_sendData(UART_MODULE id, UINT8* data, int size){
    int count = 0;
    while(count < size){
        if (UARTTransmitterIsReady(UART1)){
            UARTSendDataByte(id, data[count]);
            count++;
        }
    }
}

/**********************************************************************
 * Function: UART_getByte()
 * @param id: identifies the UART module we want to get data from
 * @remark 
 **********************************************************************/
UINT16 UART_getByte(UART_MODULE id){
    UINT16 data;
    if (UARTReceivedDataIsAvailable(id)){
        data = UARTGetDataByte(id);
    }
    return data;
}