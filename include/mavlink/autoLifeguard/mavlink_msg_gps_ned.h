// MESSAGE GPS_NED PACKING

#define MAVLINK_MSG_ID_GPS_NED 242

typedef struct __mavlink_gps_ned_t
{
 float north; ///< North component in meters.
 float east; ///< East component in meters.
 float down; ///< Down component in meters.
 uint8_t ack; ///< TRUE or FALSE if acknowledgement required.
 uint8_t status; ///< Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
} mavlink_gps_ned_t;

#define MAVLINK_MSG_ID_GPS_NED_LEN 14
#define MAVLINK_MSG_ID_242_LEN 14



#define MAVLINK_MESSAGE_INFO_GPS_NED { \
	"GPS_NED", \
	5, \
	{  { "north", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_ned_t, north) }, \
         { "east", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_ned_t, east) }, \
         { "down", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_gps_ned_t, down) }, \
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
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 * @param north North component in meters.
 * @param east East component in meters.
 * @param down Down component in meters.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t status, float north, float east, float down)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_float(buf, 8, down);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 14);
#else
	mavlink_gps_ned_t packet;
	packet.north = north;
	packet.east = east;
	packet.down = down;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 14);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED;
	return mavlink_finalize_message(msg, system_id, component_id, 14, 167);
}

/**
 * @brief Pack a gps_ned message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 * @param north North component in meters.
 * @param east East component in meters.
 * @param down Down component in meters.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_ned_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t status,float north,float east,float down)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_float(buf, 8, down);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 14);
#else
	mavlink_gps_ned_t packet;
	packet.north = north;
	packet.east = east;
	packet.down = down;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 14);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_NED;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 14, 167);
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
	return mavlink_msg_gps_ned_pack(system_id, component_id, msg, gps_ned->ack, gps_ned->status, gps_ned->north, gps_ned->east, gps_ned->down);
}

/**
 * @brief Send a gps_ned message
 * @param chan MAVLink channel to send the message
 *
 * @param ack TRUE or FALSE if acknowledgement required.
 * @param status Set boat's station location (0x1), start a rescue (0x2), or report boat's location (0x3)
 * @param north North component in meters.
 * @param east East component in meters.
 * @param down Down component in meters.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_ned_send(mavlink_channel_t chan, uint8_t ack, uint8_t status, float north, float east, float down)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[14];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_float(buf, 8, down);
	_mav_put_uint8_t(buf, 12, ack);
	_mav_put_uint8_t(buf, 13, status);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED, buf, 14, 167);
#else
	mavlink_gps_ned_t packet;
	packet.north = north;
	packet.east = east;
	packet.down = down;
	packet.ack = ack;
	packet.status = status;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_NED, (const char *)&packet, 14, 167);
#endif
}

#endif

// MESSAGE GPS_NED UNPACKING


/**
 * @brief Get field ack from gps_ned message
 *
 * @return TRUE or FALSE if acknowledgement required.
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
 * @brief Get field north from gps_ned message
 *
 * @return North component in meters.
 */
static inline float mavlink_msg_gps_ned_get_north(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field east from gps_ned message
 *
 * @return East component in meters.
 */
static inline float mavlink_msg_gps_ned_get_east(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field down from gps_ned message
 *
 * @return Down component in meters.
 */
static inline float mavlink_msg_gps_ned_get_down(const mavlink_message_t* msg)
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
	gps_ned->north = mavlink_msg_gps_ned_get_north(msg);
	gps_ned->east = mavlink_msg_gps_ned_get_east(msg);
	gps_ned->down = mavlink_msg_gps_ned_get_down(msg);
	gps_ned->ack = mavlink_msg_gps_ned_get_ack(msg);
	gps_ned->status = mavlink_msg_gps_ned_get_status(msg);
#else
	memcpy(gps_ned, _MAV_PAYLOAD(msg), 14);
#endif
}
