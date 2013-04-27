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

#include <xc.h>
#include <peripheral/timer.h>
#include "Timer.h"
#include "Board.h"

/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/

#define F_PB (Board_GetPBClock())
#define TIMER_FREQUENCY 1000
//Change to alter number of used timers with a max of 32

/***********************************************************************
 * PRIVATE FUNCTIONS                                                   *
 ***********************************************************************/

/***********************************************************************
 * PRIVATE VARIABLES                                                   *
 ***********************************************************************/
static bool     timerInitialized = FALSE;
static uint32_t timerArray[TIMER_NUMBER_MAX];
static volatile uint32_t timerActiveFlags;
static volatile uint32_t timerEventFlags;
static volatile uint32_t freeRunningTimer; // timer in milliseconds

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

    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, F_PB / TIMER_FREQUENCY);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_3);

    mT1IntEnable(1);

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
    if (timerNumber >= TIMER_NUMBER_MAX)
        return ERROR;

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
    if (timerNumber >= TIMER_NUMBER_MAX)
        return ERROR;


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
    if (timerNumber >= TIMER_NUMBER_MAX)
        return ERROR;

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
    if (timerNumber >= TIMER_NUMBER_MAX)
	return ERROR;

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
    if (timerNumber >= TIMER_NUMBER_MAX)
	return ERROR;

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
    if (timerNumber >= TIMER_NUMBER_MAX)
        return ERROR;

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
    if (timerNumber >= TIMER_NUMBER_MAX)
	return ERROR;

    timerEventFlags &= ~(1 << timerNumber);

    return SUCCESS;
}


/**********************************************************************
 * Function: get_time()
 * @return The free running time.
 * @remark Used to measure elapsed time.
 **********************************************************************/
uint32_t get_time(void) {
    if (!timerInitialized)
        return ERROR;

    return freeRunningTimer;
}


/**********************************************************************
 * Function: Timer1IntHandler
 * @return none
 * @remark This is the interrupt handler to support the timer module.
     It will increment time, to maintain the functionality of the
     GetTime() timer and it will check through the active timers,
     decrementing each active timers count, if the count goes to 0, it
     will set the associated event flag and clear the active flag to
     prevent further counting.
 **********************************************************************/
void __ISR(_TIMER_1_VECTOR, ipl3) Timer1IntHandler(void) {
    mT1ClearIntFlag();
    freeRunningTimer++;
    char curTimer = 0;
    if (timerActiveFlags != 0) {
        for (curTimer = 0; curTimer < TIMER_NUMBER_MAX; curTimer++) {
            if ((timerActiveFlags & (1 << curTimer)) != 0) {
                if (--timerArray[curTimer] == 0) {
                    timerEventFlags |= (1 << curTimer);
                    timerActiveFlags &= ~(1 << curTimer);
                }
            }
        }
    }
} // ISR


#ifdef TIMERS_TEST

    #include "serial.h"
    #include "timers.h"
    #define TIMERS_IN_TEST NUM_TIMERS
//#include <plib.h>

int main(void) {
    int i = 0;
    SERIAL_Init();
    TIMERS_Init();
    INTEnableSystemMultiVectoredInt();

    printf("\r\nUno Timers Test Harness\r\n");
    printf("Setting each timer for one second longer than the last and waiting for all to expire.  There are %d available timers\r\n", TIMERS_IN_TEST);
    for (i = 0; i <= TIMERS_IN_TEST; i++) {
        InitTimer(i, (i + 1)*1000); //for second scale
    }
    while (IsTimerActive(TIMERS_IN_TEST - 1) == TIMER_ACTIVE) {
        for (i = 0; i <= TIMERS_IN_TEST; i++) {
            if (IsTimerExpired(i) == TIMER_EXPIRED) {
                printf("Timer %d has expired and the free running counter is at %d\r\n", i, GetTime());
                ClearTimerExpired(i);
            }
        }
    }
    printf("All timers have ended\r\n");
    printf("Setting and starting 1st timer to 2 seconds using alternative method. \r\n");
    SetTimer(0, 2000);
    StartTimer(0);
    while (IsTimerExpired(0) != TIMER_EXPIRED);
    printf("2 seconds should have elapsed\r\n");
    printf("Starting 1st timer for 8 seconds but also starting 2nd timer for 4 second\r\n");
    InitTimer(0, 8000);
    InitTimer(1, 4000);
    while (IsTimerExpired(1) != TIMER_EXPIRED);
    printf("4 seconds have passed and now stopping 1st timer\r\n");
    StopTimer(0);
    printf("Waiting 6 seconds to verifiy that 1st timer has indeed stopped\r\n");
    InitTimer(1, 3000);
    i = 0;
    while (IsTimerActive(1) == TIMER_ACTIVE) {
        if (IsTimerExpired(0) == TIMER_EXPIRED) {
            i++;
            ClearTimerExpired(0);
        }
    }
    if (i == 0) {
        printf("Timer did not expire, module working correctly\r\n");
    } else {
        printf("Timer did expire, module not working correctly\r\n");
    }

    return 0;
}

#endif