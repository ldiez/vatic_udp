#include <thread>
#include <iomanip>

#include "Client.h"
#include "PacketUtils.h"
#include "Log.h"

LOG_REGISTER_MODULE("Client")

Client::Client(ClientConf c, sockaddr_in& sendAddr, int sendSocket)
: m_conf(c)
, m_addrSend(sendAddr)
, m_socketSend(sendSocket) {
  BEG;
  m_ackCount.m_lastAcked = 0;
  m_ackCount.m_lastNacked = 0;

  END;
}

Client::~Client() {
  BEG;
  auto elapsedRecvUs = DurationUs(m_recvThput.m_finish, m_recvThput.m_start);
  std::cout << "Client Recv Elapsed time " << elapsedRecvUs << " us\n";
  std::cout << "Client Recv Number of bytes: " << m_recvThput.m_nBytes << "\n";
  std::cout << "Client Recv Throuhput: " << (double(m_recvThput.m_nBytes * 8) / double(elapsedRecvUs)) << " Mbps\n";

  DBG("Creating folder: ", m_conf.m_logPath);
  fs::create_directories(m_conf.m_logPath);
  PrintRecvTimes();
  END;
}

void
Client::PrintRecvTimes() {
  BEG;
  if (m_recvTimes.size() == 0) {
    WARN("Client did not record any reception!");
    return;
  }
  std::ofstream ofs(m_conf.m_logPath + "/" + m_conf.m_logName + "_recv.dat");
  MSG_ASSERT(ofs.is_open(), "Cannot open client log file");
  for (auto& item : m_recvTimes) {
    ofs << std::get<0>(item) << "\t"
            << TimeStampMs(std::get<1>(item)) << "\t"
            << std::get<2>(item) << "\n"
            ;
  }
  ofs.close();
  END;
}

void
Client::Send() {
  BEG;
  PktHdr hdr;
  hdr.m_pktType = PKT_TYPE::ACK;
  std::uint32_t burstTimeUs = TRAFFIC_BURST_US;
  while (QUIT.load() == false) {
    auto startBurst = NOW();
    std::lock_guard<std::mutex> guard(m_ackCountMtx);
    if (m_ackCount.m_lastNacked > 0 && m_ackCount.m_lastAcked < m_ackCount.m_lastNacked) {
      hdr.m_id = htobe64(m_ackCount.m_lastNacked);
      DBG ("Sending ACK ")
      auto s = sendto(m_socketSend, (char*) &hdr, sizeof (hdr), 0, (sockaddr*) & m_addrSend, sizeof (sockaddr));
      AssertCode(s, ON_ERROR, "Error sendto()");
      m_ackCount.m_lastAcked = m_ackCount.m_lastNacked;
      m_ackCount.m_lastNacked = 0;
    }
    auto endBurst = NOW();
    ThreadSleepUs(burstTimeUs - DurationUs(endBurst, startBurst));
  }
  DBG("Quiting client sending");
  END;
}

void
Client::SendWithId(std::uint64_t pktId) {
  BEG;
  PktHdr hdr;
  hdr.m_pktType = PKT_TYPE::ACK;
  hdr.m_id = htobe64(pktId);
  auto s = sendto(m_socketSend, (char*) &hdr, sizeof (hdr), 0, (sockaddr*) & m_addrSend, sizeof (sockaddr));
//  std::cout << TimeStampMs() << " | Sent ACK of " << pktId << std::endl;
  END;
}

void
Client::Receive(const char* pkt, std::uint32_t len) {
  BEG;
  std::lock_guard<std::mutex> guard(m_ackCountMtx);
  PktHdr* hdr;
  hdr = (PktHdr*) pkt;
  if (hdr->m_pktType == PKT_TYPE::ACK) {
    WARN("Received ACK from client");
  } else {
    auto pktId = be64toh(hdr->m_id);
//    std::cout << TimeStampMs() << " | Received PKT " << pktId << std::endl;
    SendWithId(pktId);
    auto payloadLen = len - HDR_LEN;
    m_ackCount.m_lastNacked = pktId;
    m_recvTimes.push_back(TUPLE(pktId, NOW(), payloadLen));
    if (m_recvThput.m_started == false) {
      m_recvThput.m_started = true;
      m_recvThput.m_start = NOW();
    }
    m_recvThput.m_finish = NOW();
    m_recvThput.m_nBytes += payloadLen;
  }
  END;
}