// MESSAGE GPS_GEO_ORIGIN PACKING

#define MAVLINK_MSG_ID_GPS_GEO_ORIGIN 243

typedef struct __mavlink_gps_geo_origin_t
{
 float latitiude; ///< Holds a Message ID number
 float longitude; ///< Longitude data for the boat to travel to
 uint8_t ack; ///<  TRUE if we want an ACK return FALSE else
} mavlink_gps_geo_origin_t;

#define MAVLINK_MSG_ID_GPS_GEO_ORIGIN_LEN 9
#define MAVLINK_MSG_ID_243_LEN 9



#define MAVLINK_MESSAGE_INFO_GPS_GEO_ORIGIN { \
	"GPS_GEO_ORIGIN", \
	3, \
	{  { "latitiude", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_gps_geo_origin_t, latitiude) }, \
         { "longitude", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_gps_geo_origin_t, longitude) }, \
         { "ack", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_gps_geo_origin_t, ack) }, \
         } \
}


/**
 * @brief Pack a gps_geo_origin message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param latitiude Holds a Message ID number
 * @param longitude Longitude data for the boat to travel to
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_geo_origin_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t ack, float latitiude, float longitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, latitiude);
	_mav_put_float(buf, 4, longitude);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_geo_origin_t packet;
	packet.latitiude = latitiude;
	packet.longitude = longitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_GEO_ORIGIN;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 62);
}

/**
 * @brief Pack a gps_geo_origin message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param latitiude Holds a Message ID number
 * @param longitude Longitude data for the boat to travel to
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_gps_geo_origin_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t ack,float latitiude,float longitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, latitiude);
	_mav_put_float(buf, 4, longitude);
	_mav_put_uint8_t(buf, 8, ack);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_gps_geo_origin_t packet;
	packet.latitiude = latitiude;
	packet.longitude = longitude;
	packet.ack = ack;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_GPS_GEO_ORIGIN;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 62);
}

/**
 * @brief Encode a gps_geo_origin struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param gps_geo_origin C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_gps_geo_origin_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_gps_geo_origin_t* gps_geo_origin)
{
	return mavlink_msg_gps_geo_origin_pack(system_id, component_id, msg, gps_geo_origin->ack, gps_geo_origin->latitiude, gps_geo_origin->longitude);
}

/**
 * @brief Send a gps_geo_origin message
 * @param chan MAVLink channel to send the message
 *
 * @param ack  TRUE if we want an ACK return FALSE else
 * @param latitiude Holds a Message ID number
 * @param longitude Longitude data for the boat to travel to
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_gps_geo_origin_send(mavlink_channel_t chan, uint8_t ack, float latitiude, float longitude)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_float(buf, 0, latitiude);
	_mav_put_float(buf, 4, longitude);
	_mav_put_uint8_t(buf, 8, ack);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_GEO_ORIGIN, buf, 9, 62);
#else
	mavlink_gps_geo_origin_t packet;
	packet.latitiude = latitiude;
	packet.longitude = longitude;
	packet.ack = ack;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_GPS_GEO_ORIGIN, (const char *)&packet, 9, 62);
#endif
}

#endif

// MESSAGE GPS_GEO_ORIGIN UNPACKING


/**
 * @brief Get field ack from gps_geo_origin message
 *
 * @return  TRUE if we want an ACK return FALSE else
 */
static inline uint8_t mavlink_msg_gps_geo_origin_get_ack(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field latitiude from gps_geo_origin message
 *
 * @return Holds a Message ID number
 */
static inline float mavlink_msg_gps_geo_origin_get_latitiude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field longitude from gps_geo_origin message
 *
 * @return Longitude data for the boat to travel to
 */
static inline float mavlink_msg_gps_geo_origin_get_longitude(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Decode a gps_geo_origin message into a struct
 *
 * @param msg The message to decode
 * @param gps_geo_origin C-struct to decode the message contents into
 */
static inline void mavlink_msg_gps_geo_origin_decode(const mavlink_message_t* msg, mavlink_gps_geo_origin_t* gps_geo_origin)
{
#if MAVLINK_NEED_BYTE_SWAP
	gps_geo_origin->latitiude = mavlink_msg_gps_geo_origin_get_latitiude(msg);
	gps_geo_origin->longitude = mavlink_msg_gps_geo_origin_get_longitude(msg);
	gps_geo_origin->ack = mavlink_msg_gps_geo_origin_get_ack(msg);
#else
	memcpy(gps_geo_origin, _MAV_PAYLOAD(msg), 9);
#endif
}
