/*
 * @file  Xbee.h
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
#ifndef Xbee_H
#define Xbee_H

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/
#define XBEE_TEST //used for testing Xbee

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/


/**********************************************************************
 * Function: Xbee_init()
 * @param N/A
 * @return Failure or Success
 * @remark Initializes the Xbee module. Sets up API mode if the flag is
 * enabled. The function than initializes the sendArray.
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
uint8_t Xbee_init();


/**********************************************************************
 * Function: void Xbee_runSM();
 * @remark This function should be called every iteration of the
 *  statemachine. We load the recieve array with the data bytes, if we
 *  are actually reading a packet and not just noise.
 * @return none
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
void Xbee_runSM();


#ifdef XBEE_TEST
/**********************************************************************
 * Function: void Xbee_message_data_test();
 * @remark This function will be calle once a "data_test" packet has been
 *  recieved. It will send the data back immediatly, and restart the time out
 *  timer.
 * @param The test_data struct from Mavlink
 * @return none
 * @author John Ash
 * @date February 1st 2013
 **********************************************************************/
void Xbee_message_data_test(mavlink_xbee_heartbeat_t* packet);
#endif

#endif
