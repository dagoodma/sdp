/*
 * File: Timer.h
 *
 * Multiplexes a timer into 16 timers with 1 ms resoluton
 *
 */
#ifndef Timer_H
#define Timer_H

#include "Error.h"
#include "Util.h"

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

#define TIMER_NUMBER_MAX    16


/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


/**********************************************************************
 * Function: Timer_init()
 * @return none
 * @remark Configures the timer module.
 **********************************************************************/
void Timer_init(void);

/**********************************************************************
 * Function: Timer_isInitialized()
 * @return Whether the timer was initialized.
 * @remark none
 **********************************************************************/
bool Timer_isInitialized();

/**********************************************************************
 * Function: Timer_new()
 * @param Timer number.
 * @param Number of milliseconds to count for until expiring.
 * @return SUCCESS or ERROR.
 * @remark Creates a new active timer that will tick for newTime milliseconds.
 **********************************************************************/
int8_t Timer_new(uint8_t timerNumber, uint16_t newTime);

/**********************************************************************
 * Function: Timer_start()
 * @param Timer number.
 * @return SUCCESS or ERROR.
 * @remark Starts the timer counting.
 **********************************************************************/
int8_t Timer_start(uint8_t timerNumber);

/**********************************************************************
 * Function: Timer_stop()
 * @param Timer number.
 * @return SUCCESS or ERROR.
 * @remark Stops the timer from counting.
 **********************************************************************/
int8_t Timer_stop(uint8_t timerNumber);

/**********************************************************************
 * Function: Timer_set()
 * @param Timer number.
 * @param Number of milliseconds  to count for until expiring.
 * @return SUCCESS or ERROR.
 * @remark Sets the timer's timeout time, but does not make it active.
 **********************************************************************/
int8_t Timer_set(uint8_t timerNumber, uint16_t newTime);

/**********************************************************************
 * Function: Timer_isActive()
 * @param Timer number to check.
 * @return TRUE if the specified timer is active and counting down.
 * @remark none
 **********************************************************************/
bool Timer_isActive(uint8_t timerNumber);

/**********************************************************************
 * Function: Timer_isExpired()
 * @param Timer number to check.
 * @return TRUE if the specified timer is active and counting down.
 * @remark none
 **********************************************************************/
bool Timer_isExpired(uint8_t timerNumber);

/**********************************************************************
 * Function: Timer_clear()
 * @param Timer number to clear.
 * @return SUCCESS or ERROR.
 * @remark Clears the expired event on the timer.
 **********************************************************************/
int8_t Timer_clear(uint8_t timerNumber);


/**********************************************************************
 * Function: get_time()
 * @return The free running time.
 * @remark Used to measure elapsed time.
 **********************************************************************/
uint16_t get_time(void);

#endif
