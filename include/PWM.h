/**
 * @file    PWM.h
 * @author  Max Dunne
 * @author  David Goodman
 *
 * @brief
 * Multiplexes a PWM line into many channels.
 *
 * @details
 * Software module to run the PWM module of the Uno32. The available pins for
 * which the PWM works are #defined below (PortZ-6, PortY-4,10,12, and
 * PortX-11), and are set by the hardware (cannot be modified).
 *
 * @note
 * Module uses TIMER2 for its interrupts.
 *
 * @note
 * PWM_TEST (in the .c file) conditionally compiles the test harness for
 * the code. Make sure it is commented out for module useage.
 * 
 * @date January 24, 2013, 2:38 PM  -- Edited
 * @date November 12, 2011, 9:27 AM -- Created
 */

#ifndef PWM_H
#define PWM_H

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
#define PWM_500HZ 500
#define PWM_1KHZ 1000
#define PWM_2KHZ 2000
#define PWM_5KHZ 5000
#define PWM_10KHZ 10000
#define PWM_20KHZ 20000
#define PWM_30KHZ 30000
#define PWM_40KHZ 40000

#define PWM_PORTZ06 (1<<0)
#define PWM_PORTY12 (1<<1)
#define PWM_PORTY10 (1<<2)
#define PWM_PORTY04 (1<<3)
#define PWM_PORTX11 (1<<4)

#define MIN_PWM 0
#define MAX_PWM 1000



/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**
 * Function: PWM_init
 * @param Channels, used #defined PWM_PORTxxx OR'd together for each PWM Channel
 * @param Period, An integer representing the frequency in hertz
 * @return SUCCESS or ERROR
 * @remark Initializes the OC channels into PWM mode and sets up the channels at frequncy given
 * @author Max Dunne
 * @date 2011.11.12  */
char PWM_init(unsigned char Channels, unsigned int Period);

/**
 * Function: PWM_setDutyCycle
 * @param Channels, use #defined PWM_PORTxxx
 * @param Duty, duty cycle for the channel (0-1000)
 * @return SUCCESS or ERROR
 * @remark Sets the Duty Cycle for a Single Channel
 * @author Max Dunne
 * @date 2011.11.12  */
char PWM_setDutyCycle(char Channel, unsigned int Duty);


/**
 * Function: PWM_end
 * @param None
 * @return None
 * @remark Disables the PWM sub-system.
 * @author Max Dunne
 * @date 2011.11.12  */
void PWM_end(void);




#endif // PWM_H
