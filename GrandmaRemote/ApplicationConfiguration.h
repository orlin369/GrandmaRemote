/*

MIT License

Copyright (c) [2019] [Orlin Dimitrov]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

// AppplicationConfiguration.h

#ifndef _APPPLICATIONSTATE_h
#define _APPPLICATIONSTATE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Vout = (Vin*R2)/(R1+R2)

/**
 * @brief Battery voltage devider R1.
 * 
 */
#define DIV_R1 100000

/**
 * @brief Battery voltage devider R1.
 * 
 */
#define DIV_R2 100000

/**
 * @brief button input 1.
 * 
 */
#define PIN_INPUT_1 12

/**
 * @brief button input 2.
 * 
 */
#define PIN_INPUT_2 27

/**
 * @brief button input 3.
 * 
 */
#define PIN_INPUT_3 14

/**
 * @brief button input 4.
 * 
 */
#define PIN_INPUT_4 15

/**
 * @brief 
 * 
 */
#define PIN_BATT 35

/**
 * @brief Pin user LED.
 * 
 */
#define PIN_LED 13

/**
 * @brief Endpoint to the home controll server.
 * 
 */
#define END_POINT "http://home.iot.loc:1880/api/kb"

/**
 * @brief Update loop rate interval.
 * 
 */
#define UPDATE_RATE 1

#endif
