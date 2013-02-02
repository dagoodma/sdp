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
 1-18-13 8:10 PM jash    Used file from Max Dunne/ created
 1-22-13 5:24 PM jash    Move functions from Serial.c to Uart.c
 1-23-13 11:20AM jash    Make two circular buffers
***********************************************************************/


#include <xc.h>
#include <peripheral/uart.h>
#include <stdint.h>
#include "Uart.h"
#include "Board.h"
#include<ports.h>


/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#define F_PB (Board_GetPBClock())
#define QUEUESIZE 512

/*******************************************************************************
 * PRIVATE DATATYPES                                                           *
 ******************************************************************************/
typedef struct CircBuffer {
    unsigned char buffer[QUEUESIZE];
    int head;
    int tail;
    unsigned int size;
    unsigned char overflowCount;
} CircBuffer;
typedef struct CircBuffer* CBRef;


/*******************************************************************************
 * PRIVATE FUNCTIONS PROTOTYPES                                                *
 ******************************************************************************/
void newCircBuffer(CBRef cB);
void freeCircBuffer(CBRef* cB);
unsigned int getLength(CBRef cB);
int readHead(CBRef cB);
int readTail(CBRef cB);
unsigned char peak(CBRef cB);
unsigned char readFront(CBRef cB);
unsigned char writeBack(CBRef cB, unsigned char data);

/*******************************************************************************
 * PRIVATE VARIABLES                                                           *
 ******************************************************************************/
struct CircBuffer outgoingUart1;
CBRef transmitBufferUart1;
struct CircBuffer incomingUart1;
CBRef receiveBufferUart1;
struct CircBuffer outgoingUart2;
CBRef transmitBufferUart2;
struct CircBuffer incomingUart2;
CBRef receiveBufferUart2;



/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: UART_init()
 * @param id: identifies the UART module we want to initialize
 *        baudRate: The baudRate we want to initialize the function too.
 * @return a uint32_t value with the actual baudrate initialized. Check against
 *  desired to see if the system can handle the desired baud rate.
 * @remark Initializes the UART to the specific ID and BaudRate given as
 *  functions to the system. Will return the actual baudrate initialzied.
 *
 * Written by John Ash, help from Max Dunne 1/20/2013
 **********************************************************************/

void UART_init(uint8_t id, uint32_t baudRate){

    //will need buffers for both the UARTS
    if(id == UART1_ID){
        transmitBufferUart1 = (struct CircBuffer*) &outgoingUart1; //set up buffer for receive
        newCircBuffer(transmitBufferUart1);

        receiveBufferUart1 = (struct CircBuffer*) &incomingUart1; //set up buffer for transmit
        newCircBuffer(receiveBufferUart1);

        UARTConfigure(UART1, 0x00);
        UARTSetDataRate(UART1, F_PB, baudRate);
        UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);

        mU1SetIntPriority(4); //set the interrupt priority

        UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
        mU1RXIntEnable(1);
        mU1TXIntEnable(1);
    }else if(id == UART2_ID){
        transmitBufferUart2 = (struct CircBuffer*) &outgoingUart2; //set up buffer for receive
        newCircBuffer(transmitBufferUart2);

        receiveBufferUart2 = (struct CircBuffer*) &incomingUart2; //set up buffer for transmit
        newCircBuffer(receiveBufferUart2);

        UARTConfigure(UART2, 0x00);
        UARTSetDataRate(UART2, F_PB, baudRate);
        UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);

        mU2SetIntPriority(4); //set the interrupt priority

        UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
        mU2RXIntEnable(1);
        mU2TXIntEnable(1);
    }
}

void UART_putChar(uint8_t id, char ch)
{
    if(id == UART1_ID){
        if (getLength(transmitBufferUart1) != QUEUESIZE) {
            writeBack(transmitBufferUart1, ch);
            if (U1STAbits.TRMT) {
                IFS0bits.U1TXIF = 1;
            }
        }
    }else if(id == UART2_ID){
        if (getLength(transmitBufferUart2) != QUEUESIZE) {
            writeBack(transmitBufferUart2, ch);
            if (U2STAbits.TRMT) {
                IFS1bits.U2TXIF = 1;
            }
        }
    }
}

void UART_putString(uint8_t id, char* Data, int Length){
    int x;
    for(x = 0; x < Length; x++){
        UART_putChar(id, Data[x]);
        DELAY(10);
    }
}


uint16_t UART_getChar(uint8_t id)
{
    uint16_t ch;
    if(id == UART1_ID){
        if (getLength(receiveBufferUart1) == 0) {
            ch = 0xFF00;
        } else {
            ch = (readFront(receiveBufferUart1) & 0x00FF);
        }
        return ch;
    }else if(id == UART2_ID){
        if (getLength(receiveBufferUart2) == 0) {
            ch = 0xFF00;
        } else {
            ch = (readFront(receiveBufferUart2) & 0x00FF);
        }
        return ch;
    }
}

char UART_isTransmitEmpty(uint8_t id)
{
    if(id == UART1_ID){
        if (getLength(transmitBufferUart1) == 0)
            return TRUE;
        return FALSE;
    } else if(id == UART2_ID){
        if (getLength(transmitBufferUart2) == 0)
            return TRUE;
        return FALSE;
    }
}

char UART_isReceiveEmpty(uint8_t id)
{
    if(id == UART1_ID){
        if (getLength(receiveBufferUart1) == 0)
            return TRUE;
        return FALSE;
    } else if(id == UART2_ID){
        if (getLength(receiveBufferUart2) == 0)
            return TRUE;
        return FALSE;
    }
}


/***************************************************
 *              Uno32/Pic Functions
 ***************************************************/

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
    UART_putChar(UART_SERIAL_ID, c);
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
        UART_putChar(UART_SERIAL_ID, s[i]);
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
    if (getLength(receiveBufferUart1) == 0)
        return -1;
    return UART_getChar(UART_SERIAL_ID);
}




/****************************************************************************
 Function
    IntUart1Handler

 Parameters
    None.

 Returns
    None.

 Description
    Interrupt Handle for the uart. with the PIC32 architecture both send and receive are handled within the same interrupt

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void __ISR(_UART1_VECTOR, ipl4) IntUart1Handler(void)
{
    if (mU1RXGetIntFlag()) {
        mU1RXClearIntFlag();
        writeBack(receiveBufferUart1, (unsigned char) U1RXREG);
    }
    if (mU1TXGetIntFlag()) {
        mU1TXClearIntFlag();
        if (!(getLength(transmitBufferUart1) == 0)) {
            U1TXREG = readFront(transmitBufferUart1);
        }
    }
}

/****************************************************************************
 Function
    IntUart1Handler

 Parameters
    None.

 Returns
    None.

 Description
    Interrupt Handle for the uart. with the PIC32 architecture both send and receive are handled within the same interrupt

 Notes


 Author
 Max Dunne, 2011.11.10
 ****************************************************************************/
void __ISR(_UART2_VECTOR, ipl4) IntUart2Handler(void)
{
    if (mU2RXGetIntFlag()) {
        mU2RXClearIntFlag();
        writeBack(receiveBufferUart2, (unsigned char) U2RXREG);
    }
    if (mU2TXGetIntFlag()) {
        mU2TXClearIntFlag();
        if (!(getLength(transmitBufferUart2) == 0)) {
            U2TXREG = readFront(transmitBufferUart2);
        }
    }
}
/*******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

void newCircBuffer(CBRef cB)
{

    // initialize to zero
    int i;
    for (i = 0; i < QUEUESIZE; i++) {
        cB->buffer[i] = 0;
    }

    // initialize the data members
    cB->head = 0;
    cB->tail = 0;
    cB->size = QUEUESIZE;
    cB->overflowCount = 0;

}

// this function frees the Circular Buffer CB Ref

void freeCircBuffer(CBRef* cB)
{
    // if it is already null, nothing to free
    if (cB == NULL || *cB == NULL) {
        return;
    }

    // free and nil the pointer
    //free(*cB);
    *cB = NULL;
}




// Accesor Methods
// ===============

// returns the amount of unread bytes in the circular buffer

unsigned int getLength(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        if (cB->head <= cB->tail) {
            return (cB->tail - cB->head);
        } else {
            return (cB->size + cB->tail - cB->head);
        }
    } else {
        return 0;
    }


}

// returns the actual index of the head

int readHead(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        return (cB->head);
    } else {
        return 0;
    }

}

// returns the actual index of the tail

int readTail(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        return (cB->tail);
    } else {
        return 0;
    }

}

// returns the byte (actual value) that the head points to. this
// does not mark the byte as read, so succesive calls to peak will
// always return the same value

unsigned char peak(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        // if there are bytes in the buffer
        if (getLength(cB) > 0) {
            return cB->buffer[cB->head];
        }
    }
    return 0;
}


// Manipulation Procedures
// ======================
// returns the front of the circular buffer and marks the byte as read

unsigned char readFront(CBRef cB)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        char retVal;
        // if there are bytes in the buffer
        if (getLength(cB) > 0) {
            retVal = cB->buffer[cB->head];
            cB->head = cB->head < (cB->size - 1) ? cB->head + 1 : 0;
            return retVal;
        }
        return 128;
    }
    return 254;
}

// writes one byte at the end of the circular buffer,
// increments overflow count if overflow occurs

unsigned char writeBack(CBRef cB, unsigned char data)
{
    // if the circular buffer is not null
    if (cB != NULL) {
        if (getLength(cB) == (cB->size - 1)) {
            cB->overflowCount++;
            //return 1;
        } else {
            cB->buffer[cB->tail] = data;
            cB->tail = cB->tail < (cB->size - 1) ? cB->tail + 1 : 0;
            //return 0;
        }
        //return 0;
    } else {
        return 1;
    }
    return 0;
}

// empties the circular buffer. It does not change the size. use with caution!!

void makeEmpty(CBRef cB)
{
    if (cB != NULL) {
        int i;
        for (i = 0; i < cB->size; ++i) {
            cB->buffer[i] = 0;
        }
        cB->head = 0;
        cB->tail = 0;
        cB->overflowCount = 0;
    }
}

// returns the amount of times the CB has overflown;

unsigned char getOverflow(CBRef cB)
{
    if (cB != NULL) {
        return cB->overflowCount;
    }
    return 0;
}




//#define UART_TEST
#ifdef UART_TEST
#include "Serial.h"
#include "Board.h"
#include <GenericTypeDefs.h>
#include <p32xxxx.h>
#include <stdio.h>
#include <plib.h>


int main(void)
{

    Board_init();
    UART_init(UART2_ID,9600);
    printf("\r\nUno Serial Test Harness\r\nAfter this Message the terminal should mirror anything you type.\r\n");
    unsigned char ch = 0;
    int x;
    while (1) {
        if (UART_isTransmitEmpty(UART1_ID) == TRUE){
            if (UART_isReceiveEmpty(UART1_ID) == FALSE){
                ch = UART_getChar(UART1_ID);
                UART_putChar(UART1_ID, ch);
                printf("  Through UART2: ");
                UART_putChar(UART2_ID, ch);
                x = 1024;
                while(--x){
                    if(UART_isReceiveEmpty(UART2_ID) ==FALSE)
                        break;
                }
                if(UART_isReceiveEmpty(UART2_ID) == FALSE){
                    printf("%c\n", UART_getChar(UART2_ID));
                } else{
                    printf("FAILED\n");
                }
                //DELAY(1000)
            }
        }
    }
    return 0;
}
#endif