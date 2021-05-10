#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <SDL.h>

static const uint32_t kCitrapadClientMagic = 0x43555344;
static const uint32_t kCitrapadServerMagic = 0x53555344;
static const uint16_t kCitrapadProtocolVersion = 1001;

static const uint16_t kCitraPort = 26760;
#define MAX_PORTS 4

static const kCitrapadAxisX = 3;
static const kCitrapadAxisY = 4;

static const uint32_t kCitrapadPacketTypeVersion = 0x00100000;
static const uint32_t kCitrapadPacketTypePortInfo = 0x00100001;
static const uint32_t kCitrapadPacketTypePadData = 0x00100002;

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
	uint8_t port[MAX_PORTS];
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

int main(void)
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	// Server address
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(kCitraPort);
	size_t fromlen = sizeof(sa);

	// Server socket
#ifdef _WIN32
	SOCKET sock;
#else
	int sock;
#endif
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == -1) {
		fputs("ERROR: Network error.", stderr);
		perror("socket");
		fprintf(stderr, "%d\n", WSAGetLastError());
		return EXIT_FAILURE;
	}
	if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		fputs("ERROR: Network error.", stderr);
		perror("bind");
		close(sock);
		return EXIT_FAILURE;
	}

	// Get joystick
	if (SDL_Init(SDL_INIT_JOYSTICK)) {
		fputs("ERROR: Error initializing SDL.", stderr);
		fprintf("ERROR: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (SDL_NumJoysticks() == 0) {
		fputs("ERROR: Joystick not found.", stderr);
		return EXIT_FAILURE;
	}

	SDL_Joystick* joystick = NULL;
	joystick = SDL_JoystickOpen(0);
	if (joystick == NULL) {
		fputs("ERROR: Error accessing joystick.", stderr);
		fprintf(stderr, "ERROR: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	uint8_t pad_id = 0;
	uint8_t pad_mac[6] = { 0x9c, 0xaa, 0x1b, 0xbb, 0xd4, 0x22 };
	uint32_t counter = 1;

	uint8_t buffer[1024];
	for (;;) {
		int recsize = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&sa, &fromlen);
		if (recsize < 0) {
			fputs("ERROR: Networking error.", stderr);
			perror("recvfrom");
			fprintf(stderr, "%d\n", WSAGetLastError());
		}

		PacketHeader header;
		memcpy(&header, buffer, sizeof(header));
		if (header.magic != kCitrapadClientMagic) {
			fprintf(stderr, "INFO: Rejecting packet (Invalid magic number %x != %x).\n", header.magic, kCitrapadClientMagic);
			continue;
		}

		if (header.protocol_version != kCitrapadProtocolVersion) {
			fprintf(stderr, "INFO: Rejecting packet (Unmatching protocol version: %d != %d)\n", header.protocol_version, kCitrapadProtocolVersion);
			continue;
		}

		// Construct response
		Response response;
		response.header.magic = kCitrapadServerMagic;
		response.header.protocol_version = kCitrapadProtocolVersion;
		response.header.id = header.id;

		if (header.type == kCitrapadPacketTypeVersion) {
			fprintf(stderr, "INFO: Client requested version\n");
			response.header.payload_length = sizeof(uint16_t) - sizeof(uint32_t);
			response.header.crc = 0;
			response.header.type = kCitrapadPacketTypeVersion;
			response.version = kCitrapadProtocolVersion;
			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, &response, sizeof(PacketHeader) + sizeof(uint16_t));
			sendto(sock, buffer, sizeof(PacketHeader) + sizeof(uint16_t), 0, (struct sockaddr*)&sa, fromlen);
		}
		else if (header.type == kCitrapadPacketTypePortInfo) {

			RequestPortInfo req = { 0 };
			memcpy(&req, &buffer[sizeof(header)], sizeof(req));

			fprintf(stderr, "INFO: Requesting port info for %d pads\n", req.pad_count);

			// Update state
			pad_id = req.port[0];

			response.header.payload_length = sizeof(PortInfoResponse) + sizeof(uint32_t);
			response.header.crc = 0;
			response.header.type = kCitrapadPacketTypePortInfo;
			response.port_info.id = req.port[0];
			response.port_info.state = 0;
			response.port_info.model = 0;
			response.port_info.connection_type = 0;
			memcpy(&response.port_info.mac, pad_mac, sizeof(pad_mac));
			response.port_info.battery = 0xf0;
			response.port_info.is_pad_active = 1;

			memset(buffer, 0, sizeof(buffer));
			memcpy(buffer, &response, sizeof(PacketHeader) + sizeof(PortInfoResponse));
			sendto(sock, buffer, sizeof(PacketHeader) + sizeof(PortInfoResponse), 0, (struct sockaddr*)&sa, fromlen);
		}
		else if (header.type == kCitrapadPacketTypePadData) {
			fprintf(stderr, "INFO: Client requested pad data\n");

			response.header.payload_length = sizeof(PadDataResponse) + sizeof(uint32_t);
			response.header.crc = 0;
			response.header.type = kCitrapadPacketTypePadData;
			memset(&response.pad_data, 0, sizeof(PadDataResponse));
			response.pad_data.info.id = pad_id;
			response.pad_data.info.state = 0;
			response.pad_data.info.model = 0;
			response.pad_data.info.connection_type = 0;
			memcpy(&response.pad_data.info.mac, pad_mac, sizeof(pad_mac));
			response.pad_data.info.battery = 0xf0;
			response.pad_data.info.is_pad_active = 1;


			for (int i = 0; i < 8 * 60; ++i) {
				response.pad_data.packet_counter = counter++;

				SDL_JoystickUpdate();
				response.pad_data.touch_1.id = 0;
				response.pad_data.touch_1.is_active = !SDL_JoystickGetButton(joystick, 9);
				response.pad_data.touch_1.x = ((1<<14) + SDL_JoystickGetAxis(joystick, kCitrapadAxisX)) >> 1;
				response.pad_data.touch_1.y = ((1<<14) + SDL_JoystickGetAxis(joystick, kCitrapadAxisY)) >> 1;

				memset(buffer, 0, sizeof(buffer));
				memcpy(buffer, &response, sizeof(PacketHeader));
				memcpy(&buffer[sizeof(PacketHeader)], &response.pad_data, sizeof(PadDataResponse)); // Split in 2 ops to prevent padding bytes from being copied :-(
				sendto(sock, buffer, sizeof(PacketHeader) + sizeof(PadDataResponse), 0, (struct sockaddr*)&sa, fromlen);

				fprintf(stderr, "INFO: Sending packet %d\n", counter);

				Sleep(16);
			}
		}
		else {
			fprintf(stderr, "INFO: Rejecting packet (Unknown packet type %x)", header.type);
			continue;
		}
	}

	SDL_JoystickClose(joystick);

	SDL_Quit();

#ifndef _WIN32
	close(fd);
#endif

	return EXIT_SUCCESS;
}
