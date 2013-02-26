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
#ifndef SONAR_H
#define SONAR_H

#define ANALOG_PIN AD_PORTV4
#define ANALOG_WINDOW_PIN AD_PORTV5


void Sonar_init();



BOOL Sonar_runSM(uint32_t* rawAnalogWindowData, uint32_t* rawAnalogData);



#endif