/**********************************************************************
 Module
   Timer.c

 Revision
   1.0.0

 Description
   This module utilizes the PIC's 16-bit Timer/Counter1 to create an
   array of timers with 1 millisecond resolution.
   
 Notes

 History
 When           Who         What/Why
 -------------- ---         --------
 3-9-12 22:17   dagoodma    Created file.
 12-28-12 11:37 dagoodma    Adapted for lifeguard project.
***********************************************************************/

#define TIMER_H_PRIVATE_INCLUDE

#include <avr/interrupt.h>
#include "Timer.h"
#include "Util.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
#ifndef F_CPU
#define F_CPU               16000000L
#endif

#define TIMER_FREQUENCY     1000 // Hz  (1 ms)
#define PRESCALER           1
//#define TIMER_COUNT_MAX     2000
#define TIMER_COUNT_MAX     ((F_CPU / PRESCALER) / TIMER_FREQUENCY)
#define TIMER_COUNT_MAX_H   ((uint8_t)((TIMER_COUNT_MAX & 0xFF00) >> ONE_BYTE))
#define TIMER_COUNT_MAX_L   ((uint8_t)(TIMER_COUNT_MAX))

/***********************************************************************
 * PRIVATE FUNCTIONS                                                   *
 ***********************************************************************/
static void MillisecondISR(void);

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
static bool     timerInitialized = 0;
static uint16_t timerArray[TIMER_NUMBER_MAX];
static volatile uint16_t timerActiveFlags;
static volatile uint16_t timerEventFlags;
static volatile uint16_t freeRunningTimer; // timer in milliseconds

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: Timer_init()
 * @return none
 * @remark Configures the timer module.
 **********************************************************************/
void Timer_init(void) {
	timerActiveFlags = 0;
	timerEventFlags = 0;
	freeRunningTimer = 0;

    /*  Old AVR Code:
    // disable global IE to prevent corruption during init
    SREG &= ~(1 << SREG_I); 

    // disable power reduction
    PRR0 &= ~(1 << PRTIM1);

    // set Clear Timer on Compare Match (CTC) mode
    //      WGM13:0 = 4, CTC Mode. TOP=OCR1A
    TCCR1A &= ~(1 << WGM10);
    TCCR1A &= ~(1 << WGM11);
    TCCR1B |= (1 << WGM12);
    TCCR1B &= ~(1 << WGM13);
    
    // set the top count for overflowing
    OCR1AH = TIMER_COUNT_MAX_H;
    OCR1AL = TIMER_COUNT_MAX_L;

    // Reset the timer
    TCNT1 = 0;

    // Set the timer and global interrupt enable flags
    TIMSK1 |= (1 << OCIE1A); // compare match IE
    TIMSK1 |= (1 << TOIE1); // timer1 IE

    // Enable the timer (clock select/prescalar)
    //prescalar: none
#if PRESCALER == 1
    TCCR1B |= (1 << CS10);
    TCCR1B &= ~(1 << CS11);
    TCCR1B &= ~(1 << CS12);
    // prescalar: 8
#elif PRESCALER == 8
    TCCR1B &= ~(1 << CS10);
    TCCR1B |= (1 << CS11);
    TCCR1B &= ~(1 << CS12);
#elif PRESCALER == 64
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS11);
    TCCR1B &= ~(1 << CS12);
#endif

    SREG |= (1 << SREG_I); // global IE
    */

    timerInitialized = 1;
}

/**********************************************************************
 * Function: Timer_isInitialized()
 * @return Whether the timer was initialized.
 * @remark none
 **********************************************************************/
bool Timer_isInitialized() {
    return timerInitialized;
}

/**********************************************************************
 * Function: Timer_new()
 * @param Timer number.
 * @param Number of milliseconds to count for until expiring.
 * @return SUCCESS or ERROR.
 * @remark Creates a new active timer that will tick for newTime milliseconds.
 **********************************************************************/
int8_t Timer_new(uint8_t timerNumber, uint16_t newTime) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

	timerArray[timerNumber] = newTime;
    // Clear timers event flag and set its active flag.
	timerEventFlags &= ~(1 << timerNumber);
	timerActiveFlags |= (1 << timerNumber);
    return SUCCESS;
}

/**********************************************************************
 * Function: Timer_start()
 * @param Timer number.
 * @return SUCCESS or ERROR.
 * @remark Starts the timer counting.
 **********************************************************************/
int8_t Timer_start(uint8_t timerNumber) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

	timerActiveFlags |= (1 << timerNumber);
    return SUCCESS;
}

/**********************************************************************
 * Function: Timer_stop()
 * @param Timer number.
 * @return SUCCESS or ERROR.
 * @remark Stops the timer from counting.
 **********************************************************************/
int8_t Timer_stop(uint8_t timerNumber) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

	timerActiveFlags &= ~(1 << timerNumber);
    return SUCCESS;
}

/**********************************************************************
 * Function: Timer_set()
 * @param Timer number.
 * @param Number of milliseconds  to count for until expiring.
 * @return SUCCESS or ERROR.
 * @remark Sets the timer's timeout time, but does not make it active.
 **********************************************************************/
int8_t Timer_set(uint8_t timerNumber, uint16_t newTime) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

    timerArray[timerNumber] = newTime;
    return SUCCESS;
}




/**********************************************************************
 * Function: Timer_isActive()
 * @param Timer number to check.
 * @return TRUE if the specified timer is active and counting down.
 * @remark none
 **********************************************************************/
bool Timer_isActive(uint8_t timerNumber) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

    // Check active bit flag for the timer
	return (timerActiveFlags & (1 << timerNumber)) != 0;
}

/**********************************************************************
 * Function: Timer_isExpired()
 * @param Timer number to check.
 * @return TRUE if the specified timer is active and counting down.
 * @remark none
 **********************************************************************/
bool Timer_isExpired(uint8_t timerNumber) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

    // Check if the event bit flag was set
	return (timerEventFlags & (1 << timerNumber)) != 0;
}

/**********************************************************************
 * Function: Timer_clear()
 * @param Timer number to clear.
 * @return SUCCESS or ERROR.
 * @remark Clears the expired event on the timer.
 **********************************************************************/
int8_t Timer_clear(uint8_t timerNumber) {
	if (timerNumber >= TIMER_NUMBER_MAX) {
        error(ERROR_TIMER_NUMBER);
		return ERROR;
    }

	timerEventFlags &= ~(1 << timerNumber);

    return SUCCESS;
}


/**********************************************************************
 * Function: get_time()
 * @return The free running time.
 * @remark Used to measure elapsed time.
 **********************************************************************/
uint16_t get_time(void) {
	if (!timerInitialized) {
        error(ERROR_TIMER_OFF);
		return ERROR;
    }

	return freeRunningTimer;
}



/**********************************************************************
 * Function: ISR()
 * @return none
 * @remark This timer/counter1 interrupt handler is exectued when the 
 *  OCIE1A of TIMSK1, and OCF1A of TIFR1, and SREG_I of SREG are all set.
 **********************************************************************/
ISR(TIMER1_COMPA_vect)
{
	// Clear interrupt flag?
    // "TOV0 is cleared by hardware" (pg.107, atmega32u4_dastasheet.pdf)
    // OCF1A should be cleared by hardware too
	
	freeRunningTimer++;
	//if (timerActiveFlags == 0)
    //    return;

	uint8_t curTimer = 0;
    for (curTimer = 0; curTimer < TIMER_NUMBER_MAX; curTimer++) {
        if ((timerActiveFlags & (1 << curTimer)) !=  0) {

            if (--timerArray[curTimer] == 0) {
                timerEventFlags |= (1 << curTimer);
                timerActiveFlags &= ~(1 << curTimer);
            } // if curTimer expired
        } // if curTimer is active
    } // foreach timer
} // ISR

