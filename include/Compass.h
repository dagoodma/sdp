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
#ifndef Compass_H
#define Compass_H

/***********************************************************************
 * PUBLIC DEFINITIONS                                                  *
 ***********************************************************************/

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/
void Compass_message_recieve_start_resuce(mavlink_start_rescue_t* packet);



#endif
