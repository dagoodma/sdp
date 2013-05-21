/*
 * File:   pwm.c
 * Author: mdunne
 * Edited by: David Goodman
 *
 * Edited on January 24, 2013, 2:38 PM
 * Created on November 12, 2011, 9:28 AM
 */

#include <xc.h>
#include <peripheral/outcompare.h>
#include <peripheral/timer.h>
#include "PWM.h"
#include <stdio.h>
#include "Board.h"




/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

#define F_PB Board_GetPBClock()
//#define DEBUG_VERBOSE
/*
#ifdef DEBUG_VERBOSE
    #define dbprintf(...) printf(__VA_ARGS__)
#else
    #define dbprintf(...)
#endif
 * */
#define dbprintf(...)

/*******************************************************************************
 * PRIVATE VARIABLES                                                            *
 ******************************************************************************/
static unsigned char SystemActiveFlag;
static unsigned int usedChannels;
static volatile unsigned int * const Duty_Registers[] = {&OC1RS, &OC2RS, &OC3RS, &OC4RS, &OC5RS};

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/


/****************************************************************************
 Function
     PWM_init

 Parameters
    Channels, used #defined PWM_PORTxxx OR'd together for each PWM Channel
    Period, An integer representing the frequency in hertz


 Returns
     SUCCESS or ERROR

 Description
 Initializes the OC channels into PWM mode and sets up the channels at frequncy given
 Notes
     None.

 Author
 Max Dunne, 2011.11.12
 ****************************************************************************/
char PWM_init(unsigned char Channels, unsigned int Period) {
    if ((Channels < 1) || (Channels > 0x1F) || (usedChannels != 0))
        return ERROR;
    if (Period <= 1000) {
        OpenTimer2(T2_ON | T2_PS_1_32, F_PB / 32 / Period);
        dbprintf("Period less than 1KHz, setting prescaler to 32");
    } else {
        OpenTimer2(T2_ON | T2_PS_1_1, F_PB / Period);
        dbprintf("Period greater than 1KHz, setting prescaler to 1");
    }
    usedChannels = Channels;
    if (PWM_PORTZ06 & Channels) {
        OpenOC1(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
        dbprintf("Port Z6 Initialized\r\n");
    }
    if (PWM_PORTY12 & Channels) {
        OpenOC2(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
        dbprintf("Port Y12 Initialized\r\n");
    }
    if (PWM_PORTY10 & Channels) {
        OpenOC3(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
        dbprintf("Port Y10 Initialized\r\n");
    }
    if (PWM_PORTY04 & Channels) {
        OpenOC4(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
        dbprintf("Port Y4 Initialized\r\n");
    }
    if (PWM_PORTX11 & Channels) {
        OpenOC5(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0, 0);
        dbprintf("Port X11 Initialized\r\n");
    }
}

/****************************************************************************
 Function
 PWM_setDutyCycle

 Parameters
    Channel: use #defined PWM_PORTxxx
    Duty: duty cycle for the channel (0-1000)

 Returns
     None.

 Description
    Sets the Duty Cycle for a Single Channel
 Notes
     None.

 Author
 Max Dunne, 2011.11.12
 ****************************************************************************/

char PWM_setDutyCycle(char Channel, unsigned int Duty) {
    unsigned int ScaledDuty = 0;
    unsigned int TranslatedChannel = 0;
    if ((Duty < 0 || Duty > 1000) || (Channel < 1 || Channel > 0x1F) || !(Channel & usedChannels)) {
        return ERROR;
    }
    ScaledDuty = ((PR2 + 1) * Duty) / MAX_PWM;
    while (Channel > 1) {
        Channel >>= 1;
        TranslatedChannel++;
    }
    dbprintf("Translated Channel is %d and Scaled Duty is %d\r\n",TranslatedChannel,ScaledDuty);
    *Duty_Registers[TranslatedChannel] = ScaledDuty;
    return SUCCESS;

}

/****************************************************************************
 Function
     PWM_end

 Parameters
     None.

 Returns
     None

 Description
     Disables the PWM sub-system.

 Notes
     Simply releases the lines and disables the timer

 Author
    Max Dunne, 2011.11.14
 ****************************************************************************/
void PWM_end(void) {
    CloseTimer2();
    CloseOC1();
    CloseOC2();
    CloseOC3();
    CloseOC4();
    CloseOC5();
    usedChannels=0;

}



//#define PWM_TEST
#ifdef PWM_TEST

#include <serial.h>
#include <BOARD.h>
    
    #define MIN_RAMP 0
    #define MAX_RAMP 1000
    #define FIRST_PERIOD PWM_20KHZ
    #define SECOND_PERIOD PWM_500HZ
#ifndef DELAY
    #define DELAY() for(i=0;i < NOPCOUNT; i++) __asm("nop")
    #define NOPCOUNT 520000
#endif
    

int main(void) {
    unsigned int i, j;
    Board_init();
    PWM_init(PWM_PORTX11 | PWM_PORTY04 | PWM_PORTY10 | PWM_PORTY12 | PWM_PORTZ06, FIRST_PERIOD);


    printf("Uno PWM Test Harness\r\n");
    printf("Ramping PWM from %d-%d%% in 10%% steps at %d with a delay between\r\n", MIN_RAMP, MAX_RAMP, FIRST_PERIOD);
    unsigned char ch = 0;
    for (j = MIN_RAMP; j <= MAX_RAMP; j += 100) {
        PWM_setDutyCycle(PWM_PORTX11, j);
        PWM_setDutyCycle(PWM_PORTY04, j + 20);
        PWM_setDutyCycle(PWM_PORTY10, j + 40);
        PWM_setDutyCycle(PWM_PORTY12, j + 60);
        PWM_setDutyCycle(PWM_PORTZ06, j + 80);
        printf("Outputting %d%% Duty Cycle\r\n", j / 10);
        DELAY(10000);
    }
    printf("Setting Period to %d and repeating ramp\r\n", SECOND_PERIOD);
    PWM_end();
    PWM_init(PWM_PORTX11 | PWM_PORTY04 | PWM_PORTY10 | PWM_PORTY12 | PWM_PORTZ06, SECOND_PERIOD);
    for (j = MIN_RAMP; j <= MAX_RAMP; j += 100) {
        PWM_setDutyCycle(PWM_PORTX11, j);
        PWM_setDutyCycle(PWM_PORTY04, j + 20);
        PWM_setDutyCycle(PWM_PORTY10, j + 40);
        PWM_setDutyCycle(PWM_PORTY12, j + 60);
        PWM_setDutyCycle(PWM_PORTZ06, j + 80);
        printf("Outputting %d%% Duty Cycle\r\n", j / 10);
        DELAY(10000);
    }
    PWM_end();
    return 0;
}

#endif