#include "citrapad.h"

#include <stdio.h>
#include <string.h>

static const uint32_t kCitrapadClientMagic = 0x43555344;
static const uint32_t kCitrapadServerMagic = 0x53555344;
static const uint16_t kCitrapadProtocolVersion = 1001;

int citrapad_validate_packet(PacketHeader *header) {
  if (header->magic != kCitrapadClientMagic) {
    fprintf(stderr, "INFO: Rejecting packet (Invalid magic number %x != %x).\n",
            header->magic, kCitrapadClientMagic);
    return 0;
  }

  if (header->protocol_version != kCitrapadProtocolVersion) {
    fprintf(stderr,
            "INFO: Rejecting packet (Unmatching protocol version: %d != %d)\n",
            header->protocol_version, kCitrapadProtocolVersion);
    return 0;
  }

  return 1;
}

void citrapad_response_init(uint32_t id, Response *response) {
  response->header.magic = kCitrapadServerMagic;
  response->header.protocol_version = kCitrapadProtocolVersion;
  response->header.id = id;
}

void citrapad_handle_version(Response *response) {
  response->header.payload_length = sizeof(uint16_t);
  response->header.crc = 0;
  response->header.type = kCitrapadPacketTypeVersion;
  response->version = kCitrapadProtocolVersion;
}

void citrapad_handle_port_info(uint8_t pad_id, uint8_t *pad_mac,
                               Response *response) {
  response->header.payload_length = sizeof(PortInfoResponse) + sizeof(uint32_t);
  response->header.crc = 0;
  response->header.type = kCitrapadPacketTypePortInfo;
  response->port_info.id = pad_id;
  response->port_info.state = 0;
  response->port_info.model = 0;
  response->port_info.connection_type = 0;
  memcpy(&response->port_info.mac, pad_mac, 6 * sizeof(uint8_t));
  response->port_info.battery = 0xf0;
  response->port_info.is_pad_active = 1;
}

void citrapad_handle_pad_data(uint8_t pad_id, uint8_t *pad_mac,
                              Response *response) {
  response->header.payload_length = sizeof(PadDataResponse) + sizeof(uint32_t);
  response->header.crc = 0;
  response->header.type = kCitrapadPacketTypePadData;
  memset(&response->pad_data, 0, sizeof(PadDataResponse));
  response->pad_data.info.id = pad_id;
  response->pad_data.info.state = 0;
  response->pad_data.info.model = 0;
  response->pad_data.info.connection_type = 0;
  memcpy(&response->pad_data.info.mac, pad_mac, 6 * sizeof(uint8_t));
  response->pad_data.info.battery = 0xf0;
  response->pad_data.info.is_pad_active = 1;
}
