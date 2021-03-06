/**
 * @file 	rpi/src/request.hpp
 * @brief 	Server request headers
 * 
 * Provides definitions for server request functions
 * 
 * @author 	João Borrego
 */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <ctime>

#include "debug.hpp"
#include "constants.hpp"
#include "System.hpp"

/* Request types */

/** Request to obtain a certain parameter */
#define GET             "g"
/** Request to modify a certain parameter */
#define SET             "s"
/** Request to reset the system */
#define RESET           "r"
/** Get buffer with last minute information on a given variable */
#define LAST_MINUTE     "b"
/** Start "real-time" stream of given variable */
#define START_STREAM    "c"
/** Stop "real-time" stream of given variable */
#define STOP_STREAM     "d"

/** Activate distributed control */
#define DISTRIBUTED_ON  "A"
/** Deactivate distributed control */
#define DISTRIBUTED_OFF "D"

/** Dump stored entries to file */
#define SAVE            "S"

/* Get requests */

/** Get current measured illuminance at desk */
#define LUX             'l' // <i>
/** Get current duty cycle at luminaire */
#define DUTY_CYCLE      'd' // <i>
/** Get current occupancy state at desk */
#define OCCUPANCY       'o' // <i>
/** Get current illuminance lower bound at desk */
#define LUX_LOWER       'L' // <i>
/** Get current external illuminance at desk  */
#define LUX_EXTERNAL    'O' // <i>
/** Get current illuminance control reference at desk */
#define LUX_REF         'r' // <i>
/** Get instantaneous power consumption at desk or total */
#define POWER           'p' // <i> or T
/** Get accumulated energy consumption at desk or total */
#define ENERGY          'e' // <i> or T
/** Get accumulated comfort error at desk or total */
#define COMFORT_ERR     'c' // <i> or T
/** Get accumulated comfort variance at desk or total */
#define COMFORT_VAR     'v' // <i> or T
/** Get time since last restart at desk */
#define TIMESTAMP       't' // <i>

/** Total modifier parameter */
#define TOTAL           'T'

/* Responses */

/** Command acknowledged and executed */
#define ACK             "ack"
/** Invalid request */
#define INVALID         "Invalid request!"

/* Functions */

/**
 * @brief      Performs a request and produces a response.
 *
 * @param[in]  system      The system shared pointer
 * @param      timestamps  The timestamps vector
 * @param      flags       The flags vector
 * @param[in]  request     The request string
 * @param      response    The response string
 */
void parseRequest(
    System::ptr system,
    std::vector< unsigned long > & timestamps,
    std::vector< bool> & flags,
    const std::string & request,
    std::string & response);

/**
 * @brief      Produces a stream update string.
 *
 * @param[in]  system      The system shared pointer
 * @param      timestamps  The timestamps vector
 * @param[in]  flags       The flags vector
 * @param      response    The response string
 */
void streamUpdate(
    System::ptr system,
    std::vector< unsigned long > & timestamps,
    const std::vector< bool> & flags,
    std::string & response);

#endif