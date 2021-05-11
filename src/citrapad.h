#ifndef CITRAPAD_H
#define CITRAPAD_H

#include <stdint.h>

#define CITRAPAD_MAX_PORTS 4

typedef struct PacketHeader {
	uint32_t magic;
	uint16_t protocol_version;
	uint16_t payload_length;
	uint32_t crc;
	uint32_t id;
	uint32_t type;
} PacketHeader;

// Requests
typedef struct RequestPortInfo {
	uint8_t pad_count;
	uint8_t port[CITRAPAD_MAX_PORTS];
} RequestPortInfo;

typedef enum RequestPadDataFlags {
	ALL_PORTS,
	ID,
	MAC
} RequestPadDataFlags;
typedef struct RequestPadData {
	RequestPadDataFlags flags;
	uint8_t port_id;
	uint8_t mac[6];
};

// Responses
typedef struct PortInfoResponse {
	uint8_t id;
	uint8_t state;
	uint8_t model;
	uint8_t connection_type;
	uint8_t mac[6];
	uint8_t battery;
	uint8_t is_pad_active;
} PortInfoResponse;

typedef struct PadDataResponse {
	PortInfoResponse info;
	uint32_t packet_counter;
	uint16_t digital_button;
	uint8_t home;
	uint8_t touch_hard_press;
	uint8_t left_stick_x;
	uint8_t left_stick_y;
	uint8_t right_stick_x;
	uint8_t right_stick_y;
	struct AnalogButton {
		uint8_t button_8;
		uint8_t button_7;
		uint8_t button_6;
		uint8_t button_5;
		uint8_t button_12;
		uint8_t button_11;
		uint8_t button_10;
		uint8_t button_9;
		uint8_t button_16;
		uint8_t button_15;
		uint8_t button_14;
		uint8_t button_13;
	} analog_button;
	struct TouchPad {
		uint8_t is_active;
		uint8_t id;
		uint16_t x;
		uint16_t y;

	} touch_1, touch_2;
	uint64_t motion_timestamp;
	struct Accelerometer {
		float x;
		float y;
		float z;
	} accel;
	struct Gyroscope {
		float pitch;
		float yaw;
		float roll;
	} gyro;
} PadDataResponse;

typedef struct Response {
	PacketHeader header;
	union {
		uint16_t version;
		PortInfoResponse port_info;
		PadDataResponse pad_data;
	};
} Response;

static const uint16_t kCitraPort = 26760;

static const uint32_t kCitrapadPacketTypeVersion = 0x00100000;
static const uint32_t kCitrapadPacketTypePortInfo = 0x00100001;
static const uint32_t kCitrapadPacketTypePadData = 0x00100002;

void citrapad_init(void);
void citrapad_loop(void);
void citrapad_quit(void);

int citrapad_validate_packet(PacketHeader *header);

void citrapad_response_init(uint32_t id, Response *response);
void citrapad_handle_version(Response *response);
void citrapad_handle_port_info(uint8_t pad_id, uint8_t *pad_mac, Response *response);
void citrapad_handle_pad_data(uint8_t pad_id, uint8_t *pad_mac, Response *response);

#endif // CITRAPAD_H
