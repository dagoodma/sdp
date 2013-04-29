/*
 * File:   Gps.c
 * Author: David Goodman
 *
 * Created on February 4, 2013, 9:19 PM
 */

#include <xc.h>
#include <stdio.h>
#include <plib.h>
//#define __XC32
#include <math.h>
#include "Serial.h"
#include "Timer.h"
#include "Board.h"
#include "Uart.h"
#include "Gps.h"



/***********************************************************************
 * PRIVATE DEFINITIONS                                                 *
 ***********************************************************************/
//#define DEBUG
//#define DEBUG_STATE


#define START_BAUDRATE      38400 // (baud)
#define DESIRED_BAUDRATE    START_BAUDRATE

#define GPS_UART_ID         UART2_ID
#define GPS_UART_BAUDRATE   START_BAUDRATE


#define RAW_BUFFER_SIZE         255 // (bytes) maximum gps message size
#define CHECKSUM_BYTES          2 // (bytes) number in checksum

// Field indexes ( see pg. 60 in ublox UBX protocol specifications)
#define SYNC1_INDEX             0 // index of first sync byte
#define SYNC2_INDEX             1 // index of second sync byte
#define CLASS_INDEX             2 // index of class byte
#define ID_INDEX                3 // index of id byte
#define LENGTH1_INDEX           4 // start of length field (2 bytes long)
#define LENGTH2_INDEX           5 // end of length field (2 bytes long)
#define PAYLOAD_INDEX           6 // start of the payload

// Field constants
#define SYNC1_CHAR              0xB5 // first byte in message
#define SYNC2_CHAR              0x62 // second byte in message
// Message Classes
#define NAV_CLASS               0x01 // navigation message class
// Navgation message IDs
#define NAV_POSLLH_ID           0x02 // geodetic postion message id
#define NAV_STATUS_ID           0x03 // receiver navigation status (fix/nofix)
#define NAV_SOL_ID              0x06 // navigation solution message id
#define NAV_VELOCITY_ID         0x12 // velocity NED navigation message

#define NOFIX_STATUS            0x00

// GPS connection timeout for packet not seen
#define DELAY_TIMEOUT           5000

// unpacking functions
#define UNPACK_LITTLE_ENDIAN_32(data, start) \
    ((uint32_t)(data[start] + ((uint32_t)data[start+1] << 8) \
    + ((uint32_t)data[start+2] << 16) + ((uint32_t)data[start+3] << 24)))

#define MM_TO_M(unit)                   ((float)unit/1000)
#define CM_TO_M(unit)                   ((float)unit/100)
#define GEODETIC_1E7_TO_DECIMAL(coord)  ((float)coord/10000000)
#define HEADING_1E5_TO_DEGREE(heading)  ((float)heading/100000)

// Cooordinate conversion constants

// Ellipsoid (olbate) constants for coordinate conversions
#define ECC     0.0818191908426f // eccentricity
#define ECC2    (ECC*ECC)
#define ECCP2   (ECC2 / (1.0 - ECC2)) // square of second eccentricity
#define FLATR   (ECC2 / (1.0 + sqrt(1.0 - ECC2))) // flattening ratio

// Radius of earth's curviture on semi-major and minor axes respectively
#define R_EN    6378137.0f     // (m) prime vertical radius (semi-major axis)
#define R_EM    (R_EN * (1 - FLATR)) // meridian radius (semi-minor axis)



/**********************************************************************
 * PRIVATE VARIABLES                                                  *
 **********************************************************************/
BOOL gpsInitialized = FALSE;

static enum {
    STATE_IDLE      = 0x0,
    STATE_READ      = 0x1, // Reading GPS packet from UART
    STATE_PARSE     = 0x2, // Parsing a received GPS packet
} state;

uint8_t rawMessage[RAW_BUFFER_SIZE];
uint8_t byteIndex = 0, messageLength = LENGTH2_INDEX + 1,
        messageClass = 0, messageId = 0, gpsStatus = NOFIX_STATUS;


BOOL hasNewMessage = FALSE, isConnected = FALSE, hasPosition = FALSE;

// Variables read from the GPS


#ifndef USE_GEOCENTRIC_COORDINATES
GeodeticCoordinate myPosition;
GeodeticCoordinate myTempPosition;
#else
GeocentricCoordinate myPosition;
GeocentricCoordinate myTempPosition;
#endif

struct {
    int32_t northVelocity, eastVelocity, heading;
} myCourse;






/**********************************************************************
 * PRIVATE PROTOTYPES                                                 *
 **********************************************************************/

static BOOL hasNewByte();
static void startReadState();
static void startIdleState();
static void startParseState();
static int8_t readMessageByte();
static int8_t parseMessage();
static void parsePayloadField();

/**********************************************************************
 * PUBLIC FUNCTIONS                                                   *
 **********************************************************************/

/**********************************************************************
 * Function: GPS_init
 * @return none
 * @remark Initializes the GPS.
 **********************************************************************/
BOOL GPS_init() {
#ifdef DEBUG
    printf("Intializing the GPS on uart %d.\n", GPS_UART_ID);
#endif

    UART_init(GPS_UART_ID,GPS_UART_BAUDRATE);

    startIdleState();
    gpsInitialized = TRUE;
    return SUCCESS;
}

/**********************************************************************
 * Function: GPS_isInitialized
 * @return Whether the GPS was initialized.
 * @remark none
 **********************************************************************/
BOOL GPS_isInitialized() {
    return gpsInitialized;
}


/**********************************************************************
 * Function: GPS_runSM
 * @return None
 * @remark Executes the GPS's currently running state.
 **********************************************************************/
void GPS_runSM() {
    switch (state) {
        // Waiting for data
        case STATE_IDLE:
            // check for a new packet to start reading
            if (hasNewByte())
                startReadState();
            break;
        // Reading the message in and verifying sync, length, and checksum
        case STATE_READ:
            if (hasNewByte() && (readMessageByte() != SUCCESS)) {
                #ifdef DEBUG
                printf("Failed reading GPS message at byte %d.\n", byteIndex);
                while (!Serial_isTransmitEmpty()) { asm("nop"); }
                #endif
                startIdleState();
            }
            if (hasNewMessage)
                startParseState(); // finished reading, start parsing the payload
            break;
        // Parsing the new message's payload
        case STATE_PARSE:
            if (hasNewMessage)
                parseMessage();
            else
                startIdleState();
           break;
            // Should not be here!
    } // switch

    // Update connected variable
    if (Timer_isExpired(TIMER_GPS))
        isConnected = FALSE;
}


/**********************************************************************
 * Function: GPS_hasFix
 * @return TRUE if a lock has been obtained.
 * @remark
 **********************************************************************/
BOOL GPS_hasFix() {
    return gpsStatus != NOFIX_STATUS;
}


/**********************************************************************
 * Function: GPS_hasPosition
 * @return TRUE if a valid position has been obtained.
 * @remark
 **********************************************************************/
BOOL GPS_hasPosition() {
    return hasPosition;
}

/**********************************************************************
 * Function: GPS_isConnected
 * @return Returns true if GPS data seen in last 5 seconds.
 * @remark
 **********************************************************************/
BOOL GPS_isConnected() {
    return isConnected;
}


#ifdef USE_GEOCENTRIC_COORDINATES

/**********************************************************************
 * Function: GPS_getPosition
 * @param New geocentric coordinate to copy position into.
 * @return none
 * @remark  Copies the measured geocentric (ECEF) position in meters into the
 *  given coordinate object.
 **********************************************************************/
void GPS_getPosition(GeocentricCoordinate *ecefPos) {
    ecefPos->x = myPosition.x;
    ecefPos->y = myPosition.y;
    ecefPos->z = myPosition.z;
}
#else

/**********************************************************************
 * Function: GPS_getPosition
 * @param New geodetic coordinate to copy position into.
 * @return none
 * @remark  Copies the measured geodetic (LLA) position in degrees (altitude
 *  in meters) into the given coordinate object.
 **********************************************************************/
 void GPS_getPosition(GeodeticCoordinate *llaPos) {
    llaPos->lat = myPosition.lat;
    llaPos->lon = myPosition.lon;
    llaPos->alt = myPosition.alt;
}
#endif


/**********************************************************************
 * Function: GPS_getNorthVelocity
 * @return Returns the current velocity in the north direction in cm/s.
 * @remark Centimeters per second in the north direction.
 **********************************************************************/
int32_t GPS_getNorthVelocity() {
    return myCourse.northVelocity;
}

/**********************************************************************
 * Function: GPS_getEastVelocity
 * @return Returns the current velocity in the east direction in cm/s.
 * @remark Centimeters per second in the east direction.
 **********************************************************************/
int32_t GPS_getEastVelocity() {
    return myCourse.eastVelocity;
}

/**********************************************************************
 * Function: GPS_getVelocity
 * @return Returns the current velocity in m/s.
 * @remark
 **********************************************************************/
float GPS_getVelocity() {
    float nVel = CM_TO_M(myCourse.northVelocity);
    float eVel = CM_TO_M(myCourse.eastVelocity);
    return sqrtf(nVel*nVel + eVel*eVel);
}


/**********************************************************************
 * Function: GPS_getHeading
 * @return Returns the current heading in degrees.
 * @remark
 **********************************************************************/
float GPS_getHeading() {
    return HEADING_1E5_TO_DEGREE(myCourse.heading);
}


/**********************************************************************
 * PRIVATE FUNCTIONS                                                  *
 **********************************************************************/


/**********************************************************************
 * Function: hasNewMessage
 * @return Returns true if a new message is ready to be read
 * @remark 
 **********************************************************************/
static BOOL hasNewByte() {
    return !UART_isReceiveEmpty(GPS_UART_ID);
}

/**********************************************************************
 * Function: startIdleState
 * @return None
 * @remark Switches into the GPS idle state.
 **********************************************************************/
static void startIdleState() {
    state = STATE_IDLE;
#ifdef DEBUG_STATE
    printf("Entered idle state.\n");
    while (!Serial_isTransmitEmpty()) { asm("nop"); }
#endif

}

/**********************************************************************
 * Function: startReadState
 * @return None
 * @remark Switches into the read state for the GPS.
 **********************************************************************/
static void startReadState() {
    state = STATE_READ;
    byteIndex = 0;
    messageLength = PAYLOAD_INDEX;
    hasNewMessage = FALSE;
    
#ifdef DEBUG_STATE
    printf("Entered read state.\n");
    while (!Serial_isTransmitEmpty()) { asm("nop"); }
#endif
}

/**********************************************************************
 * Function: startParseState
 * @return None
 * @remark Switches into the GPS parse state.
 **********************************************************************/
static void startParseState() {
    state = STATE_PARSE;
    byteIndex = PAYLOAD_INDEX;
    
#ifdef DEBUG_STATE
    printf("Entered parse state.\n");
    while (!Serial_isTransmitEmpty()) { asm("nop"); }
#endif
}

/**********************************************************************
 * Function: setConnected
 * @return None
 * @remark Sets the GPS connected state to TRUE and starts a timeout timer.
 **********************************************************************/
static void setConnected() {
    isConnected = TRUE;
    Timer_new(TIMER_GPS,DELAY_TIMEOUT);

#ifdef DEBUG
    printf("Connected to GPS.\n");
    while (!Serial_isTransmitEmpty()) { asm("nop"); }
#endif
}

/**********************************************************************
 * Function: parseMessageByte
 * @return SUCCESS, FAILURE, or ERROR.
 * @remark Reads a GPS packet from the UART. This function will return
 *  SUCCESS every time a valid byte is read (interpets the sync, length,
 *  and checksum fields). The hasNewMessage field will be set to TRUE
 *  when a new message is received and ready for parsing.
 **********************************************************************/
static int8_t readMessageByte() {
    // Read a new byte from the UART or return FAILURE
    if (hasNewByte() && !hasNewMessage)
        rawMessage[byteIndex] = UART_getChar(GPS_UART_ID);
    else
        return FAILURE;

    // Look at the new byte
    switch(byteIndex) {
        case SYNC1_INDEX:
            if (rawMessage[byteIndex] != SYNC1_CHAR)
                return FAILURE;
            break;
        case SYNC2_INDEX:
            if (rawMessage[byteIndex] != SYNC2_CHAR)
                return FAILURE;
            // Two sync packets mean we see the GPS
            setConnected();
            break;
        case CLASS_INDEX:
            messageClass = rawMessage[byteIndex];
            break;
        case ID_INDEX:
            messageId = rawMessage[byteIndex];
            break;
        case LENGTH1_INDEX:
            messageLength = rawMessage[byteIndex];
            break;
        case LENGTH2_INDEX:
            messageLength += rawMessage[byteIndex] << 8;
            // Make length total for whole message
            messageLength += PAYLOAD_INDEX + CHECKSUM_BYTES;
            break;
        default:
            // Just reading payload and checksum (look these later)
            if (byteIndex >= (messageLength - 1)) {
                hasNewMessage = TRUE;
            }
    } // switch

    byteIndex++;
    return SUCCESS;
}

/**********************************************************************
 * Function: parseMessageByte()
 * @return SUCCESS, FAILURE, or ERROR.
 * @remark Parses the payload fields of a newly received GPS message.
 **********************************************************************/
static int8_t parseMessage() {
    //for (byteIndex = 0; byteIndex < RAW_BUFFER_SIZE; byteIndex++) {

    // interpret message by parsing payload fields
    if (byteIndex < (messageLength - CHECKSUM_BYTES)) {
        parsePayloadField(); // Processing payload field by field
    }
    /*
    else if (byteIndex > (messageLength - CHECKSUM_BYTES) < messageLength) {
        //if (verifyChecksum() != SUCCESS) return FAILURE;
        startIdleState();
        return SUCCESS;
    }*/
    else {
        // Done parsing the message
        hasNewMessage = FALSE;
    }

    return SUCCESS; // ready to parse next byte
}


/**********************************************************************
 * Function: parsePayloadField()
 * @return None
 * @remark Parses one field of the payload for the new GPS message.
 **********************************************************************/
static void parsePayloadField() {
    switch (messageClass) {
        // #################### Navigation Message #######################
        case NAV_CLASS:
            switch (messageId) {
#ifndef USE_GEOCENTRIC_COORDINATES
                // ------------- NAV-POSLLH (0x01 0x02) --------------
                case NAV_POSLLH_ID:
                    switch (byteIndex - PAYLOAD_INDEX) {
                        case 0: // iTow (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 4: // lon
                            myPosition.lon = GEODETIC_1E7_TO_DECIMAL(
                                    (int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24)));
                            //(int32_t)UNPACK_LITTLE_ENDIAN_32(rawMessage,byteIndex);
                            byteIndex += sizeof(int32_t);
                            break;
                        case 8: // lat
                            myPosition.lat = GEODETIC_1E7_TO_DECIMAL(
                                    (int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24)));
                            byteIndex += sizeof(int32_t);
                            break;
                        case 12: // height (not implemented)
                            byteIndex += sizeof(int32_t);
                            break;
                        case 16: // hMSL
                            myPosition.alt = MM_TO_M(
                                    (int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24)));
                            // copy to myPosition
                            myPosition.lat = myTempPosition.lat;
                            myPosition.lon = myTempPosition.lon;
                            myPosition.alt = myTempPosition.alt;
                            hasPosition = TRUE;
                            byteIndex += sizeof(int32_t);
                            break;
                        case 20: // hAcc (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 24: // vAcc (not implemented)
                            byteIndex += sizeof(uint32_t);
                    } // NAV_POSLLH_ID byte
                    break;
#endif
                // ------------- NAV-STATUS (0x01 0x03) --------------
                case NAV_STATUS_ID:
                    switch (byteIndex - PAYLOAD_INDEX) {
                        case 0: // iTow (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 4: // gpsFix
                            gpsStatus = rawMessage[byteIndex];
                            if (!GPS_hasFix() && hasPosition)
                                hasPosition = FALSE;
                            byteIndex += sizeof(uint8_t);
                            break;
                        case 5: // flags (not implemented)
                            byteIndex += sizeof(uint8_t);
                            break;
                        case 6: // diffStat (not implemented)
                            byteIndex += sizeof(uint8_t);
                            break;
                        case 7: // res (not implemented)
                            byteIndex += sizeof(uint8_t);
                            break;
                        case 8: // ttff (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 12: // msss (not implemented)
                            byteIndex += sizeof(uint32_t);
                    } //  NAV_STATUS_ID byte
                    break;
#ifdef USE_GEOCENTRIC_COORDINATES
                // ------------- NAV-SOL (0x01 0x06) --------------
                case NAV_SOL_ID:
                    switch (byteIndex - PAYLOAD_INDEX) {
                        case 0: // iTow (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 4: // fTow (not implemented)
                            byteIndex += sizeof(int32_t);
                            break;
                        case 8: // week (not implemented)
                            byteIndex += sizeof(int16_t);
                            break;
                        case 10: // gpsFix (not implemented -- see NAV_STATUS)
                            byteIndex += sizeof(uint8_t);
                            break;
                        case 11: // flags (not implemented)
                            byteIndex += sizeof(uint8_t);
                            break;
                        case 12: // ecefX
                            myTempPosition.x = CM_TO_M((int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24)));
                            byteIndex += sizeof(int32_t);
                            break;
                        case 16: // ecefY
                            myTempPosition.y = CM_TO_M((int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24)));
                            byteIndex += sizeof(int32_t);
                            break;
                        case 20: // ecefZ
                            myTempPosition.z = CM_TO_M((int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24)));
                            // copy to myPosition
                            myPosition.x = myTempPosition.x;
                            myPosition.y = myTempPosition.y;
                            myPosition.z = myTempPosition.z;
                            hasPosition = TRUE;
                            byteIndex += sizeof(int32_t);
                            break;
                        case 24: // pAcc (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 28: // ecefVX (not implemented)
                            byteIndex += sizeof(int32_t);
                            break;
                        case 32: // ecefVY (not implemented)
                            byteIndex += sizeof(int32_t);
                        case 36: // ecefVZ (not implemented)
                            byteIndex += sizeof(int32_t);
                        case 40: // sAcc (not implemented)
                            byteIndex += sizeof(uint32_t);
                        case 44: // pDOP (not implemented)
                            byteIndex += sizeof(uint16_t);
                        case 46: // res1 (not implemented)
                            byteIndex += sizeof(uint8_t);
                        case 47: // numSV (not implemented)
                            byteIndex += sizeof(uint8_t);
                        case 48: // res2 (not implemented)
                            byteIndex += sizeof(uint32_t);
                    } // NAV_SOL_ID byte
                    break;
#endif
                // ------------- NAV-VELNED (0x01 0x12) --------------
                case NAV_VELOCITY_ID:
                    switch (byteIndex - PAYLOAD_INDEX) {
                        case 0: // iTow (not implemented)
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 4: // velN
                            myCourse.northVelocity = (int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24));
                            byteIndex += sizeof(int32_t);
                            break;
                        case 8: // velE
                            myCourse.eastVelocity = (int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24));
                            byteIndex += sizeof(int32_t);
                            break;
                        case 12: // velD
                            byteIndex += sizeof(int32_t);
                            break;
                        case 16: // speed
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 20: // gSpeed
                            byteIndex += sizeof(uint32_t);
                            break;
                        case 24: // heading
                            myCourse.heading = (int32_t)(rawMessage[byteIndex]
                                    + ((int32_t)rawMessage[byteIndex + 1] << 8)
                                    + ((int32_t)rawMessage[byteIndex + 2] << 16)
                                    + ((int32_t)rawMessage[byteIndex + 3] << 24));
                            byteIndex += sizeof(int32_t);
                            break;
                        case 28: // sAcc
                            byteIndex += sizeof(uint32_t);
                        case 32: // cAcc
                            byteIndex += sizeof(uint32_t);
                    } //  NAV_VELOCITY_ID byte
                    break;// ------------- End of Valid NAV message IDs --------------
                default:
                    byteIndex = messageLength; // skip rest of message
                #ifdef DEBUG
                    printf("Received unhandled navigation message id: 0x%X.\n", messageId);
                    while (!Serial_isTransmitEmpty()) { asm("nop"); }
                #endif
            } // switch messageID = NAV_CLASS
            break;
        // #################### End of NAV messages #######################
        default:
            byteIndex = messageLength; // skip rest of message
        #ifdef DEBUG
            printf("Received unhandled message class: 0x%X.\n", messageClass);
            while (!Serial_isTransmitEmpty()) { asm("nop"); }
        #endif
    } // switch messageClass
}




/*******************************************************************************
 * LIBRARY FUNCTIONS                                                           *
 ******************************************************************************/


void convertGeodetic2ECEF(GeocentricCoordinate *ecef, GeodeticCoordinate *lla) {
    float sinlat = sinf(DEGREE_TO_RADIAN*lla->lat);
    float coslat = cosf(DEGREE_TO_RADIAN*lla->lat);

    float rad_ne = R_EN / sqrt(1.0 - (ECC2 * sinlat * sinlat));
    ecef->x = (rad_ne + lla->alt) * coslat * cosf(lla->lon*DEGREE_TO_RADIAN);
    ecef->y = (rad_ne + lla->alt) * coslat * sinf(lla->lon*DEGREE_TO_RADIAN);
    ecef->z = (rad_ne*(1.0 - ECC2) + lla->alt) * sinlat;
}


/*
void convertECEF2Geodetic(Coordinate *var, float ecef_x, float ecef_y, float ecef_z) {
    float lat = 0, lon = 0, alt = 0;

    lon = atan2(ecef_y, ecef_x);

    float rho = hypotf(ecef_x,ecef_y); // distance from z-axis
    float beta = atan2(ecef_z, (1 - FLATR) * rho);

    lat = atan2(ecef_z + R_EM * ECCP2 * sinf(beta)*sinf(beta)*sinf(beta),
        rho - R_EN * ECC2 * cosf(beta)*cosf(beta)*cosf(beta));

    float betaNew = atan2((1 - FLATR)*sinf(lat), cosf(lat));
    int count = 0;
    while (beta != betaNew && count < 5) {
        beta = betaNew;
        var->x = atan2(ecef_z  + R_EM * ECCP2 * sinf(beta)*sinf(beta)*sinf(beta),
            rho - R_EN * ECC2 * cosf(beta)*cosf(beta)*cosf(beta));

        betaNew = atan2((1 - FLATR)*sinf(lat), cosf(lat));
        count = count + 1;
    }

    float sinlat = sinf(lat);
    float rad_ne = R_EN / sqrt(1.0 - (ECC2 * sinlat * sinlat));

    alt = rho * cosf(lat) + (ecef_z + ECC2 * rad_ne * sinlat) * sinlat - rad_ne;

    // Convert radian geodetic to degrees
    var->x = RADIAN_TO_DEGREE*lat;
    var->y = RADIAN_TO_DEGREE*lon;
    var->z = alt;
}
*/



void convertECEF2NED(LocalCoordinate *ned, GeocentricCoordinate *ecef_cur,
    GeocentricCoordinate *ecef_ref, GeodeticCoordinate *geo_ref) {
    GeocentricCoordinate ecef_path;
    ecef_path.x = ecef_cur->x - ecef_ref->x;
    ecef_path.y = ecef_cur->y - ecef_ref->y;
    ecef_path.z = ecef_cur->z - ecef_ref->z;

    float cosLat = cosf(geo_ref->lat * DEGREE_TO_RADIAN);
    float sinLat = sinf(geo_ref->lat * DEGREE_TO_RADIAN);
    float cosLon = cosf(geo_ref->lon * DEGREE_TO_RADIAN);
    float sinLon = sinf(geo_ref->lon * DEGREE_TO_RADIAN);

    // Offset vector from reference and rotate
    float t =  cosLon * ecef_path.x + sinLon * ecef_path.y;

    ned->n = -sinLat * t + cosLat * ecef_path.z;
    ned->e = -sinLon * ecef_path.x + cosLon * ecef_path.y;
    ned->d = -(cosLat * t + sinLat * ecef_path.z);
    //ned->d = 0;
}


void projectEulerToNED(LocalCoordinate *ned, float yaw, float pitch, float height) {
    //printf("At angle: %.3f and pitch: %.3f\n",yaw,pitch);

    float mag = height * tan((90.0-pitch)*DEGREE_TO_RADIAN);
    #ifdef DEBUG
    printf("\tMagnitude: %.3f\n",mag);
    #endif

    //printf("At mag: %.3f\n",mag);

    if (yaw <= 90.0) {
        //First quadrant
        ned->n = mag * cosf(yaw*DEGREE_TO_RADIAN);
        ned->e = mag * sinf(yaw*DEGREE_TO_RADIAN);
    }
    else if (yaw > 90.0 && yaw <= 180.0) {
        // Second quadrant
        yaw = yaw - 270.0;
        ned->n = mag * sinf(yaw*DEGREE_TO_RADIAN);
        ned->e = -mag * cosf(yaw*DEGREE_TO_RADIAN);
    }
    else if (yaw > 180.0 && yaw <= 270.0) {
        // Third quadrant
        yaw = yaw - 180.0;
        ned->n = -mag * cosf(yaw*DEGREE_TO_RADIAN);
        ned->e = -mag * sinf(yaw*DEGREE_TO_RADIAN);
    }
    else if (yaw > 270 < 360.0) {
        // Fourth quadrant
        yaw = yaw - 90.0;
        ned->n = -mag * sinf(yaw*DEGREE_TO_RADIAN);
        ned->e = mag * cosf(yaw*DEGREE_TO_RADIAN);
    }

    ned->d = height;
    //printf("Desired coordinate -- N:%.2f, E: %.2f, D: %.2f (m)\n",
    //    ned->n, ned->e, ned->d);
}


void getCourseVector(CourseVector *course, LocalCoordinate *ned_cur,
        LocalCoordinate *ned_des) {
    LocalCoordinate ned_path;
    ned_path.n = ned_des->n - ned_cur->n;
    ned_path.e = ned_des->e - ned_cur->e;
    //ned_path.d = ned_des->d - ned_cur->d;

    // Calculate heading (in degrees from North) of path
    if (ned_path.n > 0.0 && ned_path.e > 0.0) {
        course->yaw = atanf(fabsf(ned_path.e)/fabsf(ned_path.n))*RADIAN_TO_DEGREE;
    }
    else if (ned_path.n < 0.0 && ned_path.e > 0.0) {
        course->yaw = atanf(fabsf(ned_path.n)/fabsf(ned_path.e))*RADIAN_TO_DEGREE;
        course->yaw += 90.0;
    }
    else if (ned_path.n < 0.0 && ned_path.e < 0.0) {
        course->yaw = atanf(fabsf(ned_path.e)/fabsf(ned_path.n))*RADIAN_TO_DEGREE;
        course->yaw += 180.0;
    }
    else if (ned_path.n > 0.0 && ned_path.e < 0.0) {
        course->yaw = atanf(fabsf(ned_path.n)/fabsf(ned_path.e))*RADIAN_TO_DEGREE;
        course->yaw += 270.0;
    }

    // Calculate distance to point
    course->d = sqrtf((ned_path.n)*(ned_path.n) + (ned_path.e)*(ned_path.e));

}






/****************************** TESTS ************************************/
// Test harness that spits out GPS packets over the serial port
//#define GPS_TEST
#ifdef GPS_TEST
int main() {
    Board_init(); // initalize interrupts
    Serial_init(); // init serial port for printing messages
    GPS_init();
    Timer_init();
    printf("GPS initialized.\n");

    Timer_new(TIMER_TEST,1000);
    while(1) {
        if (Timer_isExpired(TIMER_TEST)) {
            if (!GPS_isConnected()) {
                printf("GPS not connected.\n");
            }
            else if (GPS_hasFix() && GPS_hasPosition()) {
#ifndef USE_GEOCENTRIC_COORDINATES
                GeodeticCoordinate llaPos;
                GPS_getPosition(&llaPos);
                printf("Position: lat=%.6f, lon=%.6f, alt=%.2f (deg and m)\n",llaPos.lat,
                    llaPos.lon, llaPos.alt);
#else
                GeocentricCoordinate ecefPos;
                GPS_getPosition(&ecefPos);
                printf("Position: x=%.2f, y=%.2f, z=%.2f (m)\n", ecefPos.x,
                    ecefPos.y, ecefPos.z);
#endif
                
                printf("Velocity: %.2f (m/s), Heading: %.2f (deg)\n",
                    GPS_getVelocity(), GPS_getHeading());
            }
            else {
                printf("No fix!\n");
            }

            Timer_new(TIMER_TEST,1000);
        }
        GPS_runSM();

    }
}

#endif


//#define GPS_HEADING_TEST
#ifdef GPS_HEADING_TEST

#include "I2C.h"
#include "TiltCompass.h"

// Pick the I2C_MODULE to initialize
// Set Desired Operation Frequency
#define I2C_CLOCK_FREQ  100000 // (Hz)

int main() {
    Board_init(); // initalize interrupts
    Serial_init(); // init serial port for printing messages
    Timer_init();
    I2C_init(I2C1, I2C_CLOCK_FREQ);
    GPS_init();
    TiltCompass_init();
    printf("GPS and Compass initialized.\n");

    Timer_new(TIMER_TEST,1000);
    while(1) {
        if (Timer_isExpired(TIMER_TEST)) {
            if (!GPS_isConnected()) {
                printf("GPS not connected.\n");
            }
            else if (GPS_hasFix() && GPS_hasPosition()) {
#ifndef USE_GEOCENTRIC_COORDINATES
                GeodeticCoordinate llaPos;
                GPS_getPosition(&llaPos);
                printf("Position: lat=%.6f, lon=%.6f, alt=%.2f (deg and m)\n",llaPos.lat,
                    llaPos.lon, llaPos.alt);
#else
                GeocentricCoordinate ecefPos;
                GPS_getPosition(&ecefPos);
                printf("GPS Position: x=%.2f, y=%.2f, z=%.2f (m)\n", ecefPos.x,
                    ecefPos.y, ecefPos.z);
#endif

                printf("\tVelocity: %.2f (m/s), Heading: %.2f (deg)\n",
                    GPS_getVelocity(), GPS_getHeading());
            }
            else {
                printf("GPS has no fix!\n");
            }

            printf("Compass heading: %.1f\n", TiltCompass_getHeading());
            Timer_new(TIMER_TEST,1000);
        }

        GPS_runSM();
        TiltCompass_runSM();
    }
}


#endif


//#define GPS_LIBRARY_TEST

#ifdef GPS_LIBRARY_TEST

int main() {
    Board_init();
    Serial_init();
    Timer_init();
    printf("Ready to test GPS library functions.\n\n");
    uint32_t timer;

    // convertGeodetic2ECEF
    GeodeticCoordinate lla1;
    // (deg and m)
    lla1.lat = 57.23125421f;
    lla1.lon = 122.412334f;
    lla1.alt = 10.0f;

    printf("Converting geodetic to ECEF...\n");
    printf("\t Lat = %.6f, lon = %.6f, alt = %.1f [deg]\n",lla1.lat,lla1.lon,lla1.alt);
    GeocentricCoordinate ecef1;
    timer = get_time();
    convertGeodetic2ECEF(&ecef1, &lla1);
    timer = get_time() - timer;
    printf("\t x = %.2f, y = %.2f, z = %.1f [m]\n",ecef1.x,ecef1.y,ecef1.z);
    const char *result1 = ( (ecef1.x < (-1854786.87f + 1.0) && ecef1.x > (-1854786.87f - 1.0))
        && (ecef1.y < (2921286.75f + 1.0) && ecef1.y > (2921286.75f - 1.0))
        && (ecef1.z < (5339891.0f + 1.0) && ecef1.z > (5339891.0f - 1.0)))?
            "Passed" : "Failed";
    printf("\t %s -- Elapsed: %d [ms]\n\n", result1, timer);


    // convertECEF2NED
    GeocentricCoordinate ecef2_cur, ecef2_ref;
    GeodeticCoordinate lla2_ref;
    LocalCoordinate ned2;
    // boat location (m)
    ecef2_cur.x = -1854793.0f;
    ecef2_cur.y = 2921293.00f;
    ecef2_cur.z = 5339865.0f;
    // command center location (m and deg)
    ecef2_ref.x = -1854786.87f;
    ecef2_ref.y = 2921286.75f;
    ecef2_ref.z = 5339891.0f;
    lla2_ref.lat = 57.23125421f;
    lla2_ref.lon = 122.412334f;
    lla2_ref.alt = 10.0f;

    printf("Converting ECEF to NED...\n");
    printf("\t Reference: Lat = %.6f, lon = %.6f [deg]\n\t\t x = %.2f, y = %.2f, z = %2.f [m]\n",
        lla2_ref.lat,lla2_ref.lon,ecef2_ref.x, ecef2_ref.y, ecef2_ref.z );
    printf("\t Current: x = %.2f, y = %.2f, z = %2.f [m]\n",
        ecef2_cur.x, ecef2_cur.y, ecef2_cur.z );
    timer = get_time();
    convertECEF2NED(&ned2, &ecef2_cur, &ecef2_ref, &lla2_ref);
    timer = get_time() - timer;
    printf("\t N = %.3f, E = %.3f, D = %.3f [m]\n",ned2.n,ned2.e,ned2.d);
    const char *result2 = ( (ned2.n < (-21.270f + 1.0) && ned2.n > (-21.270f - 1.0))
        && (ned2.e < (1.821f + 1.0) && ned2.e > (1.821f - 1.0))
        && (ned2.d < (17.23f + 1.0) && ned2.d > (17.23f - 1.0)))?
            "Passed" : "Failed";
    printf("\t %s -- Elapsed: %d [ms]\n\n", result2, timer);


    // projectEulerToNED
    LocalCoordinate ned3;
    float yaw = 18.4, pitch = 65.3, height = 5.74; // (m)
    printf("Projecting euler to NED...\n");
    printf("\t Yaw = %.2f, pitch = %.2f, height = %.2f [m]\n", yaw, pitch, height);
    timer = get_time();
    projectEulerToNED(&ned3, yaw, pitch, height);
    timer = get_time() - timer;
    printf("\t N = %.3f, E = %.3f, D = %.3f [m]\n",ned3.n,ned3.e,ned3.d);
    const char *result3 = ( (ned3.n < (2.505f + 0.05) && ned3.n > (2.505f - 0.05))
        && (ned3.e < (0.833f + 0.05) && ned3.e > (0.833f - 0.05))
        && (ned3.d < (5.740f + 0.05) && ned3.d > (5.740f - 0.05)))?
            "Passed" : "Failed";
    printf("\t %s -- Elapsed: %d [ms]\n\n", result3, timer);


    // getCourseVector
    LocalCoordinate ned4_cur, ned4_des;
    CourseVector desiredCourse;
    // current and desired locations (m)
    ned4_cur.n = -21.138f;
    ned4_cur.e = 1.5489f;
    ned4_cur.d = 17.2288f;
    ned4_des.n = -56.4f;
    ned4_des.e = 76.423f;
    ned4_des.d = 17.2288f;

    printf("Calculating course vector...\n");
    printf("\t Current: N = %.3f, E = %.3f [m]\n", ned4_cur.n, ned4_cur.e);
    printf("\t Desired: N = %.3f, E = %.3f [m]\n", ned4_des.n, ned4_des.e);
    timer = get_time();
    getCourseVector(&desiredCourse, &ned4_cur, &ned4_des);
    timer = get_time() - timer;
    printf("\t Distance = %.3f [m], Heading = %.3f [deg]\n",
        desiredCourse.d, desiredCourse.yaw);
    const char *result4 = ( (desiredCourse.d < (82.761854f + 0.003)
            && desiredCourse.d > (82.761854f - 0.003))
        && (desiredCourse.yaw < (154.784f +  0.003)
            && desiredCourse.yaw > (154.784f -  0.003)) )?
            "Passed" : "Failed";
    printf("\t %s -- Elapsed: %d [ms]\n\n", result4, timer);

    printf("\nFinished testing GPS library functions.\n");

    return SUCCESS;
}

#endif