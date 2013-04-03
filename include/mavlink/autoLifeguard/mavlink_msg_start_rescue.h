// MESSAGE START_RESCUE PACKING

#define MAVLINK_MSG_ID_START_RESCUE 241

typedef struct __mavlink_start_rescue_t
{
 float north; ///< Latitude data for the boat to travel to
 float east; ///< Longitude data for the boat to travel to
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
 uint8_t status; ///< Holds status informatiom for the boat
} mavlink_start_rescue_t;

#define MAVLINK_MSG_ID_START_RESCUE_LEN 10
#define MAVLINK_MSG_ID_241_LEN 10



#define MAVLINK_MESSAGE_INFO_START_RESCUE { \
	"START_RESCUE", \
	4, \
	{  { "north", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_start_rescue_t, north) }, \
         { "east", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_start_rescue_t, east) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_start_rescue_t, ack) }, \
         { "status", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_start_rescue_t, status) }, \
         } \
}


/**
 * @brief Pack a start_rescue message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Holds status informatiom for the boat
 * @param north Latitude data for the boat to travel to
 * @param east Longitude data for the boat to travel to
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_start_rescue_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, uint8_t status, float north, float east)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[10];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_uint8_t(buf, 8, ack);
	_mav_put_uint8_t(buf, 9, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 10);
#else
	mavlink_start_rescue_t packet;
	packet.north = north;
	packet.east = east;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 10);
#endif

	msg->msgid = MAVLINK_MSG_ID_START_RESCUE;
	return mavlink_finalize_message(msg, system_id, component_id, 10, 111);
}

/**
 * @brief Pack a start_rescue message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Holds status informatiom for the boat
 * @param north Latitude data for the boat to travel to
 * @param east Longitude data for the boat to travel to
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_start_rescue_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,uint8_t status,float north,float east)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[10];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_uint8_t(buf, 8, ack);
	_mav_put_uint8_t(buf, 9, status);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 10);
#else
	mavlink_start_rescue_t packet;
	packet.north = north;
	packet.east = east;
	packet.ack = ack;
	packet.status = status;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 10);
#endif

	msg->msgid = MAVLINK_MSG_ID_START_RESCUE;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 10, 111);
}

/**
 * @brief Encode a start_rescue struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param start_rescue C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_start_rescue_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_start_rescue_t* start_rescue)
{
	return mavlink_msg_start_rescue_pack(system_id, component_id, msg, start_rescue->ack, start_rescue->status, start_rescue->north, start_rescue->east);
}

/**
 * @brief Send a start_rescue message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param status Holds status informatiom for the boat
 * @param north Latitude data for the boat to travel to
 * @param east Longitude data for the boat to travel to
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_start_rescue_send(mavlink_channel_t chan, uint8_t ack, uint8_t status, float north, float east)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[10];
	_mav_put_float(buf, 0, north);
	_mav_put_float(buf, 4, east);
	_mav_put_uint8_t(buf, 8, ack);
	_mav_put_uint8_t(buf, 9, status);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_START_RESCUE, buf, 10, 111);
#else
	mavlink_start_rescue_t packet;
	packet.north = north;
	packet.east = east;
	packet.ack = ack;
	packet.status = status;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_START_RESCUE, (const char *)&packet, 10, 111);
#endif
}

#endif

// MESSAGE START_RESCUE UNPACKING


/**
 * @brief Get field ack from start_rescue message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_start_rescue_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field status from start_rescue message
 *
 * @return Holds status informatiom for the boat
 */
static inline uint8_t mavlink_msg_start_rescue_get_status(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  9);
}

/**
 * @brief Get field north from start_rescue message
 *
 * @return Latitude data for the boat to travel to
 */
static inline float mavlink_msg_start_rescue_get_north(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field east from start_rescue message
 *
 * @return Longitude data for the boat to travel to
 */
static inline float mavlink_msg_start_rescue_get_east(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Decode a start_rescue message into a struct
 *
 * @param msg The message to decode
 * @param start_rescue C-struct to decode the message contents into
 */
static inline void mavlink_msg_start_rescue_decode(const mavlink_message_t* msg, mavlink_start_rescue_t* start_rescue)
{
#if MAVLINK_NEED_BYTE_SWAP
	start_rescue->north = mavlink_msg_start_rescue_get_north(msg);
	start_rescue->east = mavlink_msg_start_rescue_get_east(msg);
	start_rescue->ack = mavlink_msg_start_rescue_get_ack(msg);
	start_rescue->status = mavlink_msg_start_rescue_get_status(msg);
#else
	memcpy(start_rescue, _MAV_PAYLOAD(msg), 10);
#endif
}
