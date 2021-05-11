#include "citrapad.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <windows.h>

#include "joystick.h"

static SOCKET sock;

void citrapad_init() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR) {
    fputs("ERROR: Network error. ", stderr);
    fprintf(stderr, "%d\n", WSAGetLastError());
  }

  // Server address
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(kCitraPort);

  // Server socket
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) {
    fputs("ERROR: Network error.", stderr);
    perror("socket");
    fprintf(stderr, "%d\n", WSAGetLastError());
    exit(EXIT_FAILURE);
  }
  if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
    fputs("ERROR: Network error.", stderr);
    perror("bind");
    closesocket(sock);
    exit(EXIT_FAILURE);
  }
}

void citrapad_quit() {
  shutdown(sock, SD_BOTH);
  closesocket(sock);

  if (WSACleanup() == SOCKET_ERROR) {
    fputs("ERROR: Network error.", stderr);
    fprintf(stderr, "%d\n", WSAGetLastError());
  }
}

void citrapad_loop() {
  uint8_t pad_id = 0;
  uint8_t pad_mac[6] = {0x9c, 0xaa, 0x1b, 0xbb, 0xd4, 0x22};
  uint32_t counter = 1;

  struct sockaddr_in sa = {};
  memset(&sa, 0, sizeof(sa));
  size_t fromlen = sizeof(sa);

  uint8_t buffer[1024];
  for (;;) {
    int recsize =
        recvfrom(sock, buffer, 1024, 0, (struct sockaddr *)&sa, &fromlen);
    if (recsize < 0) {
      fputs("ERROR: Networking error.", stderr);
      perror("recvfrom");
      fprintf(stderr, "%d\n", WSAGetLastError());
    }

    PacketHeader header;
    memcpy(&header, buffer, sizeof(header));
    if (!citrapad_validate_packet(&header)) {
      continue;
    }

    // Construct response
    Response response;
    citrapad_response_init(header.id, &response);

    if (header.type == kCitrapadPacketTypeVersion) {
      fprintf(stderr, "INFO: Client requested version\n");

      citrapad_handle_version(&response);

      memset(buffer, 0, sizeof(buffer));
      memcpy(buffer, &response, sizeof(PacketHeader) + sizeof(uint16_t));
      sendto(sock, buffer, sizeof(PacketHeader) + sizeof(uint16_t), 0,
             (struct sockaddr *)&sa, fromlen);
    } else if (header.type == kCitrapadPacketTypePortInfo) {

      RequestPortInfo req = {0};
      memcpy(&req, &buffer[sizeof(header)], sizeof(req));

      fprintf(stderr, "INFO: Requesting port info for %d pads\n",
              req.pad_count);

      // Update state
      pad_id = req.port[0];

      citrapad_handle_port_info(pad_id, pad_mac, &response);

      memset(buffer, 0, sizeof(buffer));
      memcpy(buffer, &response,
             sizeof(PacketHeader) + sizeof(PortInfoResponse));
      sendto(sock, buffer, sizeof(PacketHeader) + sizeof(PortInfoResponse), 0,
             (struct sockaddr *)&sa, fromlen);
    } else if (header.type == kCitrapadPacketTypePadData) {
      fprintf(stderr, "INFO: Client requested pad data\n");

      citrapad_handle_pad_data(pad_id, pad_mac, &response);

      for (int i = 0; i < 8 * 30; ++i) {
        response.pad_data.packet_counter = counter++;

        response.pad_data.touch_1.id = 0;
        joystick_update(&response.pad_data.touch_1.is_active,
                        &response.pad_data.touch_1.x,
                        &response.pad_data.touch_1.y);

        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, &response, sizeof(PacketHeader));
        memcpy(&buffer[sizeof(PacketHeader)], &response.pad_data,
               sizeof(PadDataResponse)); // Split in 2 ops to prevent padding
                                         // bytes from being copied :-(
        sendto(sock, buffer, sizeof(PacketHeader) + sizeof(PadDataResponse), 0,
               (struct sockaddr *)&sa, fromlen);

        fprintf(stderr, "INFO: Sending packet %d\n", counter);

        Sleep(32);
      }
    } else {
      fprintf(stderr, "INFO: Rejecting packet (Unknown packet type %x)",
              header.type);
      continue;
    }
  }
}
