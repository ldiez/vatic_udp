#ifndef PACKETUTILS_H
#define PACKETUTILS_H

#include <cstdint>

constexpr std::uint32_t TRAFFIC_BURST_US = 1e4;

enum PKT_TYPE {
  DATA = 1,
  ACK = 2,
  CONNECTION = 3
};

struct __attribute__ ((packed, aligned(1))) PktHdr {
  std::uint8_t m_pktType;
  std::uint64_t m_id;
  // If packet is data it is the packet id
  // if packet is ACk it is the next expected packet id
};

struct __attribute__ ((packed, aligned(1))) PktConnection {
  PktHdr m_hdr;
  std::uint16_t m_listeningPort;
  // If packet is data it is the packet id
  // if packet is ACk it is the next expected packet id
};

static constexpr std::uint32_t MAX_PKT_LEN = 1009;
static constexpr std::uint32_t HDR_LEN = sizeof(PktHdr);
static constexpr std::uint32_t MAX_PAYLOAD_LEN = MAX_PKT_LEN - HDR_LEN;

#endif /* PACKETUTILS_H */