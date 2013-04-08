// MESSAGE GPS_NED PACKING

#define MAVLINK_MSG_ID_GPS_NED 241

typedef struct __mavlink_gps_ned_t
{
 float n; ///< North component in meters
 float e; ///< Geocentric y position in meters
 float d; ///< Geocentric z position in meters
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t status; ///< Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
} mavlink_gps_ned_t;

#define MAVLINK_MSG_ID_GPS_NED_LEN 14
#define MAVLINK_MSG_ID_241_LEN 14



#define MAVLINK_MESSAGE_INFO_GPS_NED { \
	"GPS_NED", \
	5, \
	{  { "n", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_ned_t, n) }, \
         { "e", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_ned_t, e) }, \
         { "d", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_gps_ned_t, d) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 12, offsetof(mavlink_gps_ned_t, ack) }, \
         { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 13, offsetof(mavlink_gps_ned_t, status) }, \
         } \
}


/**
 * @brief Pack a gps_ned message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 * @param n North component in meters
 * @param e Geocentric y position in meters
 * @param d Geocentric z position in meters
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t status, float n, float e, float d)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, n);
	_mav_put_float(buf, 4, e);
	_mav_put_float(buf, 8, d);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 14);
#else
	mavlink_gps_ned_t packet;
	packet.n = n;
	packet.e = e;
	packet.d = d;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 14);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED;
	return mavlink_finalize_message(msg, system_id, component_id, 14, 122);
}

/**
 * @brief Pack a gps_ned message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 * @param n North component in meters
 * @param e Geocentric y position in meters
 * @param d Geocentric z position in meters
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t status,float n,float e,float d)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, n);
	_mav_put_float(buf, 4, e);
	_mav_put_float(buf, 8, d);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 14);
#else
	mavlink_gps_ned_t packet;
	packet.n = n;
	packet.e = e;
	packet.d = d;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 14);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 14, 122);
}

/**
 * @brief Encode a gps_ned struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param gps_ned C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_gps_ned_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_gps_ned_t* gps_ned)
{
	return mavlink_msg_gps_ned_pack(system_id, component_id, msg, gps_ned->ack, gps_ned->status, gps_ned->n, gps_ned->e, gps_ned->d);
}

/**
 * @brief Send a gps_ned message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 * @param n North component in meters
 * @param e Geocentric y position in meters
 * @param d Geocentric z position in meters
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_ned_send(mavlink_channel_t chan, uint8_t ack, uint8_t status, float n, float e, float d)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, n);
	_mav_put_float(buf, 4, e);
	_mav_put_float(buf, 8, d);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED, buf, 14, 122);
#else
	mavlink_gps_ned_t packet;
	packet.n = n;
	packet.e = e;
	packet.d = d;
	packet.ack = ack;
	packet.status = status;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED, (const char *)&packet, 14, 122);
#endif
}

#endif

// MESSAGE GPS_NED UNPACKING


/**
 * @brief Get field ack from gps_ned message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_gps_ned_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  12);
}

/**
 * @brief Get field status from gps_ned message
 *
 * @return Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 */
static inline uint8_t mavlink_msg_gps_ned_get_status(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  13);
}

/**
 * @brief Get field n from gps_ned message
 *
 * @return North component in meters
 */
static inline float mavlink_msg_gps_ned_get_n(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field e from gps_ned message
 *
 * @return Geocentric y position in meters
 */
static inline float mavlink_msg_gps_ned_get_e(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field d from gps_ned message
 *
 * @return Geocentric z position in meters
 */
static inline float mavlink_msg_gps_ned_get_d(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Decode a gps_ned message into a struct
 *
 * @param msg The message to decode
 * @param gps_ned C-struct to decode the message contents into
 */
static inline void mavlink_msg_gps_ned_decode(const mavlink_message_t* msg, mavlink_gps_ned_t* gps_ned)
{
#if MAVLINK_NEED_BYTE_SWAP
	gps_ned->n = mavlink_msg_gps_ned_get_n(msg);
	gps_ned->e = mavlink_msg_gps_ned_get_e(msg);
	gps_ned->d = mavlink_msg_gps_ned_get_d(msg);
	gps_ned->ack = mavlink_msg_gps_ned_get_ack(msg);
	gps_ned->status = mavlink_msg_gps_ned_get_status(msg);
#else
	memcpy(gps_ned, _MAV_PAYLOAD(msg), 14);
#endif
}
