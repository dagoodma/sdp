// MESSAGE GPS_ECEF PACKING

#define MAVLINK_MSG_ID_GPS_ECEF 241

typedef struct __mavlink_gps_ecef_t
{
 float x; ///< Geocentric x position in meters
 float y; ///< Geocentric y position in meters
 float z; ///< Geocentric z position in meters
 uint8_t ack; ///< TRUE or FALSE if acknowledgement required.
 uint8_t status; ///< Command center's origin (0x1), or GPS error (0x2)
} mavlink_gps_ecef_t;

#define MAVLINK_MSG_ID_GPS_ECEF_LEN 14
#define MAVLINK_MSG_ID_241_LEN 14



#define MAVLINK_MESSAGE_INFO_GPS_ECEF { \
	"GPS_ECEF", \
	5, \
	{  { "x", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_ecef_t, x) }, \
         { "y", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_ecef_t, y) }, \
         { "z", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_gps_ecef_t, z) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 12, offsetof(mavlink_gps_ecef_t, ack) }, \
         { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 13, offsetof(mavlink_gps_ecef_t, status) }, \
         } \
}


/**
 * @brief Pack a gps_ecef message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Command center's origin (0x1), or GPS error (0x2)
 * @param x Geocentric x position in meters
 * @param y Geocentric y position in meters
 * @param z Geocentric z position in meters
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ecef_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t status, float x, float y, float z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, x);
	_mav_put_float(buf, 4, y);
	_mav_put_float(buf, 8, z);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 14);
#else
	mavlink_gps_ecef_t packet;
	packet.x = x;
	packet.y = y;
	packet.z = z;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 14);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_ECEF;
	return mavlink_finalize_message(msg, system_id, component_id, 14, 222);
}

/**
 * @brief Pack a gps_ecef message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Command center's origin (0x1), or GPS error (0x2)
 * @param x Geocentric x position in meters
 * @param y Geocentric y position in meters
 * @param z Geocentric z position in meters
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ecef_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t status,float x,float y,float z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, x);
	_mav_put_float(buf, 4, y);
	_mav_put_float(buf, 8, z);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 14);
#else
	mavlink_gps_ecef_t packet;
	packet.x = x;
	packet.y = y;
	packet.z = z;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 14);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_ECEF;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 14, 222);
}

/**
 * @brief Encode a gps_ecef struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param gps_ecef C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_gps_ecef_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_gps_ecef_t* gps_ecef)
{
	return mavlink_msg_gps_ecef_pack(system_id, component_id, msg, gps_ecef->ack, gps_ecef->status, gps_ecef->x, gps_ecef->y, gps_ecef->z);
}

/**
 * @brief Send a gps_ecef message
 * @param chan MAVLink channel to send the message
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Command center's origin (0x1), or GPS error (0x2)
 * @param x Geocentric x position in meters
 * @param y Geocentric y position in meters
 * @param z Geocentric z position in meters
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_ecef_send(mavlink_channel_t chan, uint8_t ack, uint8_t status, float x, float y, float z)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, x);
	_mav_put_float(buf, 4, y);
	_mav_put_float(buf, 8, z);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_ECEF, buf, 14, 222);
#else
	mavlink_gps_ecef_t packet;
	packet.x = x;
	packet.y = y;
	packet.z = z;
	packet.ack = ack;
	packet.status = status;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_ECEF, (const char *)&packet, 14, 222);
#endif
}

#endif

// MESSAGE GPS_ECEF UNPACKING


/**
 * @brief Get field ack from gps_ecef message
 *
 * @return TRUE or FALSE if acknowledgement required.
 */
static inline uint8_t mavlink_msg_gps_ecef_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  12);
}

/**
 * @brief Get field status from gps_ecef message
 *
 * @return Command center's origin (0x1), or GPS error (0x2)
 */
static inline uint8_t mavlink_msg_gps_ecef_get_status(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  13);
}

/**
 * @brief Get field x from gps_ecef message
 *
 * @return Geocentric x position in meters
 */
static inline float mavlink_msg_gps_ecef_get_x(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field y from gps_ecef message
 *
 * @return Geocentric y position in meters
 */
static inline float mavlink_msg_gps_ecef_get_y(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field z from gps_ecef message
 *
 * @return Geocentric z position in meters
 */
static inline float mavlink_msg_gps_ecef_get_z(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Decode a gps_ecef message into a struct
 *
 * @param msg The message to decode
 * @param gps_ecef C-struct to decode the message contents into
 */
static inline void mavlink_msg_gps_ecef_decode(const mavlink_message_t* msg, mavlink_gps_ecef_t* gps_ecef)
{
#if MAVLINK_NEED_BYTE_SWAP
	gps_ecef->x = mavlink_msg_gps_ecef_get_x(msg);
	gps_ecef->y = mavlink_msg_gps_ecef_get_y(msg);
	gps_ecef->z = mavlink_msg_gps_ecef_get_z(msg);
	gps_ecef->ack = mavlink_msg_gps_ecef_get_ack(msg);
	gps_ecef->status = mavlink_msg_gps_ecef_get_status(msg);
#else
	memcpy(gps_ecef, _MAV_PAYLOAD(msg), 14);
#endif
}
