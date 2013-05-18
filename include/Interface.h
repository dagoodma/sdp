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
#include "Error.h"

#ifndef INTERFACE_H
#define INTERFACE_H


/**********************************************************************
 * PUBLIC VARIABLES                                                   *
 **********************************************************************/

typedef enum {
    NO_MESSAGE = 0x0,
    /* - Calibrate state messages - */
    CALIBRATE_SUCCESS_MESSAGE,
    CALIBRATE_PITCH_MESSAGE,
    CALIBRATE_YAW_MESSAGE,
    /* - Ready state messages - */
    READY_MESSAGE,
    /* - Rescue state messages - */
    STARTING_RESCUE_MESSAGE,
    STARTED_RESCUE_MESSAGE,
    RESCUE_SUCCESS_MESSAGE,
    CANCEL_RESCUE_MESSAGE,
    START_RETURN_MESSAGE, // shared with stop state
    RETURNING_MESSAGE, // shared with stop state
    /* - Stop state messages - */
    STOPPING_BOAT_MESSAGE,
    STOPPED_BOAT_MESSAGE,
    CANCEL_STOP_MESSAGE,
    /* - Set station state messages - */
    SAVING_STATION_MESSAGE,
    SAVED_STATION_MESSAGE,
    SET_STATION_MESSAGE,
    /* - Set origin state messages - */
    SETTING_ORIGIN_MESSAGE,
    SET_ORIGIN_MESSAGE,
    /* - Other messages - */
    BOAT_ONLINE_MESSAGE,
    RESET_BOAT_MESSAGE,
    RESET_SYSTEM_MESSAGE,
    CANCEL_RETURN_MESSAGE,
    CANCEL_SETSTATION_MESSAGE
} message_t;



/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: Interface_init
 * @param None.
 * @return None.
 * @remark Initialzies the pins used by the buttons and LEDs.
 **********************************************************************/
void Interface_init();


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
 * @return TRUE if the cancel button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isCancelPressed();

/**********************************************************************
 * Function: Interface_isOkPressed
 * @param None.
 * @return TRUE if the ok button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isOkPressed();


/**********************************************************************
 * Function: Interface_isStopPressed
 * @param None.
 * @return TRUE if the stop button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isStopPressed();

/**********************************************************************
 * Function: Interface_isRescuePessed
 * @param None.
 * @return TRUE if the rescue button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isRescuePressed();

/**********************************************************************
 * Function: Interface_isSetStationPessed
 * @param None.
 * @return TRUE if the set station button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isSetStationPressed();

/**********************************************************************
 * Function: Interface_isResetPressed
 * @param None.
 * @return TRUE if reset button was pressed.
 * @remark
 **********************************************************************/
bool Interface_isResetPressed();

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
 * Function: Interface_waitLightOnTimer
 * @param amount of time in ms that you want the light to remain on
 * @return None.
 * @remark Turns the LED on for a certain amount of time
 **********************************************************************/
void Interface_waitLightOnTimer(uint16_t ms);


/**********************************************************************
 * Function: Interface_pitchLightsOff
 * @return None.
 * @remark Turns pitch calibration lights off.
 **********************************************************************/
void Interface_pitchLightsOff();

/**********************************************************************
 * Function: Interface_yawLightsOff
 * @return None.
 * @remark Turns yaw calibration lights off.
 **********************************************************************/
void Interface_yawLightsOff();

/**********************************************************************
 * Function: Interface_pitchLightsOn
 * @return None.
 * @remark Turns pitch calibration lights on, which will use both
 *  top calibration LEDs to signal to the user when the scope is level
 *  by lighting both lights. If the scope is not level, the lights
 *  will indicate which way the scope should be pitched.
 **********************************************************************/
void Interface_pitchLightsOn();

/**********************************************************************
 * Function: Interface_yawLightsOn
 * @return None.
 * @remark Turns yaw calibration lights on, which will use both
 *  top calibration LEDs to signal to the user when the scope is facing
 *  true North, by turning both LEDs on.
 **********************************************************************/
void Interface_yawLightsOn();

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
 * Function: Interface_showErrorMessage
 * @param Error code for the error message to print.
 * @return None.
 * @remark Prints an error code to the LCD screen, and turns on the
 *  error LED, while clearing all other lights and messages.
 **********************************************************************/
void Interface_showErrorMessage(error_t errorCode);

/**********************************************************************
 * Function: Interface_showBoatErrorMessage
 * @param Error code for the error message to print.
 * @return None.
 * @remark Prints a boat error code to the LCD screen, and turns on the
 *  error LED, while clearing all other lights and messages.
 **********************************************************************/
void Interface_showBoatErrorMessage(error_t errorCode);

/**********************************************************************
 * Function: Interface_clearAll
 * @param None.
 * @return None.
 * @remark Clear the LCD and clear all messages on timers
 **********************************************************************/
void Interface_clearAll();

/**********************************************************************
 * Function: Interface_clearDisplay
 * @param None.
 * @return None.
 * @remark Clear the LCD.
 **********************************************************************/
void Interface_clearDisplay();

/**********************************************************************
 * Function: getMessage
 * @param None.
 * @return None.
 * @remark Returns the interface message for the given message code.
 **********************************************************************/
char *getMessage(message_t code);


#endif
