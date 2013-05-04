/*
 * @file  Interface.h
 *
 * @author John Ash
 *
 * @brief
 * State machine for Xbee module.
 *
 * @details
 * Module that wraps the bee in a statemachine that
 * reads from the UART.
 *
 * @date February 1, 2013 2:59 AM -- created
 *
 */
#ifndef INTERFACE_H
#define INTERFACE_H

typedef enum {
    NO_MESSAGE = 0x0,
    CALIBRATE_SUCCESS_MESSAGE,
    CALIBRATE_PITCH_MESSAGE
} message_t;

//typedef enum error_enum  error_t ;

const char *INTERFACE_MESSAGE[] = {
    "You should never see this",
    "Calibration successful",
    "CALIBRATE THE PITCH!!! DO IT",
};


/**********************************************************************
 * Function: Interface_runSM
 * @param None.
 * @return None.
 * @remark Must be called every cycle of the Command Center, checks the
 *  timers and reacts.
 **********************************************************************/
void Interface_runSM();

/**********************************************************************
 * Function: Interface_isCancelPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isCancelPressed();

/**********************************************************************
 * Function: Interface_isOkPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isOkPressed();


/**********************************************************************
 * Function: Interface_isStopPressed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isStopPressed();

/**********************************************************************
 * Function: Interface_isRescuePessed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isRescuePessed();

/**********************************************************************
 * Function: Interface_isSetStationPessed
 * @param None.
 * @return True if the button has been presed, false if the button is
 *  untouched
 * @remark
 **********************************************************************/
bool Interface_isSetStationPessed();

/**********************************************************************
 * Function: Interface_readyLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_readyLightOn();

/**********************************************************************
 * Function: Interface_readyLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_readyLightOff();


/**********************************************************************
 * Function: Interface_waitLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_waitLightOn();

/**********************************************************************
 * Function: Interface_waitLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_waitLightOff();

/**********************************************************************
 * Function: Interface_errorLightOn
 * @param None.
 * @return None.
 * @remark Turns the LED On by
 **********************************************************************/
void Interface_errorLightOn();

/**********************************************************************
 * Function: Interface_errorLightOff
 * @param None.
 * @return None.
 * @remark Turns the LED off by
 **********************************************************************/
void Interface_errorLightOff();

/**********************************************************************
 * Function: Interface_readyLightOnTimer
 * @param amount of time in ms that you want the light to remain on
 * @return None.
 * @remark Turns the LED on for a certain amount of time
 **********************************************************************/
void Interface_readyLightOnTimer(uint16_t ms);

/**********************************************************************
 * Function: Interface_errorLightOnTimer
 * @param amount of time in ms that you want the light to remain on
 * @return None.
 * @remark Turns the LED on for a certain amount of time
 **********************************************************************/
void Interface_errorLightOnTimer(uint16_t ms);

/**********************************************************************
 * Function: Interface_showMessageOnTimer
 * @param message_t you want to display on a timer
 * @param amount of time in ms that you want the message to remain on
 * @return None.
 * @remark After the timer expires the next message will be displayed
 **********************************************************************/
void Interface_showMessageOnTimer(message_t msgCode, uint16_t ms);

/**********************************************************************
 * Function: Interface_showMessage
 * @param message_t you want to display
 * @return None.
 * @remark Message will be printed out to the screen
 **********************************************************************/
void Interface_showMessage(message_t msgCode);

/**********************************************************************
 * Function: Interface_clearAll
 * @param None.
 * @return None.
 * @remark Clear the LCD and clear all messages on timers
 **********************************************************************/
void Interface_clearAll();



#endif
