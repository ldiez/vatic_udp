#include <thread>
#include <iomanip>

#include "Vatic.h"
#include "PacketUtils.h"
#include "Log.h"

LOG_REGISTER_MODULE("Vatic")

static constexpr int LOW_CONG_LABEL = 0;
static constexpr int HIGH_CONG_LABEL = 1;
static constexpr int INVALID_CENTROID = -1;
static constexpr int INVALID_LABEL = -1;

Vatic::Vatic(VaticConf vc, sockaddr_in& addrSend, int socketSend)
: m_conf(vc)
, m_addrSend(addrSend)
, m_socketSend(socketSend) {
  BEG;
  m_buffer = (char*) calloc(m_conf.m_bufferSize, sizeof (char));
  m_bufferOffset = 0;
  m_sendingBuffer = (char*) calloc(MAX_PKT_LEN, sizeof (char));

  m_cwnd.m_cwndSize = m_conf.m_initCwnd;
  m_cwnd.m_cwndFree = m_cwnd.m_cwndSize;
  m_cwnd.m_cwndUsed = 0;
  m_logCwnds.push_back(TUPLE(NOW(), m_cwnd.m_cwndSize, m_cwnd.m_cwndUsed, m_cwnd.m_cwndFree));

  m_alpha = 0.5;
  m_logAlpha.push_back(TUPLE(NOW(), m_alpha));

  m_currPktId = 1u;

  m_centroids = {INVALID_CENTROID, INVALID_CENTROID};
  m_maxCentDist = std::abs(m_centroids.first - m_centroids.second);

  END;
}

Vatic::~Vatic() {
  BEG;

  free(m_buffer);
  free(m_sendingBuffer);
  //  auto elapsedRecvUs = DurationUs(m_recvThput.m_finish, m_recvThput.m_start);
  //  std::cout << "App Recv Elapsed time " << elapsedRecvUs << " us\n";
  //  std::cout << "App Recv Number of bytes: " << m_recvThput.m_nBytes << "\n";
  //  std::cout << "App Recv Throuhput: " << (double(m_recvThput.m_nBytes * 8) / double(elapsedRecvUs)) << " Mbps\n";

  auto elapsedSendUs = DurationUs(m_sendThput.m_finish, m_sendThput.m_start);
  std::cout << "Server Send Elapsed time " << elapsedSendUs << " us\n";
  std::cout << "Server Send Number of bytes: " << m_sendThput.m_nBytes << "\n";
  std::cout << "Server Send Throuhput: " << (double(m_sendThput.m_nBytes * 8) / double(elapsedSendUs)) << " Mbps\n";

  DBG("Creating folder: ", m_conf.m_logPath);
  fs::create_directories(m_conf.m_logPath);
  PrintRtts();
  PrintCwnd();
  PrintAlpha();
  PrintCentroids();
  END;
}

void
Vatic::PrintRtts() {
  BEG;
  if (m_logRtts.size() == 0) {
    WARN("Server did not record any RTT!");
    return;
  }
  std::ofstream ofs(m_conf.m_logPath + "/" + m_conf.m_logName + "_rtts.dat");
  MSG_ASSERT(ofs.is_open(), "Cannot open client log file");
  for (auto& item : m_logRtts) {
    ofs << TimeStampMs(std::get<0>(item)) << "\t"
            << std::get<1>(item) << "\t"
            << std::get<2>(item) << "\n";
  }
  ofs.close();
  END;
}

void
Vatic::PrintCwnd() {
  BEG;
  if (m_logCwnds.size() == 0) {
    WARN("Server did not record any CWND change!");
    return;
  }
  std::ofstream ofs(m_conf.m_logPath + "/" + m_conf.m_logName + "_cwnd.dat");
  MSG_ASSERT(ofs.is_open(), "Cannot open client log file");
  for (auto& item : m_logCwnds) {
    ofs << TimeStampMs(std::get<0>(item)) << "\t"
            << std::get<1>(item) << "\t"
            << std::get<2>(item) << "\t"
            << std::get<3>(item) << "\n";
  }
  ofs.close();
  END;
}

void
Vatic::PrintAlpha() {
  BEG;
  if (m_logAlpha.size() == 0) {
    WARN("Server did not record any alpha change!");
    return;
  }
  std::ofstream ofs(m_conf.m_logPath + "/" + m_conf.m_logName + "_alpha.dat");
  MSG_ASSERT(ofs.is_open(), "Cannot open client log file");
  for (auto& item : m_logAlpha) {
    ofs << TimeStampMs(std::get<0>(item)) << "\t"
            << std::get<1>(item) << "\n";
  }
  ofs.close();
  END;
}

void
Vatic::PrintCentroids() {
  BEG;
  if (m_logCentroids.size() == 0) {
    WARN("Server did not record any centroids change!");
    return;
  }
  std::ofstream ofs(m_conf.m_logPath + "/" + m_conf.m_logName + "_centroids.dat");
  MSG_ASSERT(ofs.is_open(), "Cannot open client log file");
  for (auto& item : m_logCentroids) {
    ofs << TimeStampMs(std::get<0>(item)) << "\t"
            << std::get<1>(item) << "\t"
            << std::get<2>(item) << "\n";
  }
  ofs.close();
  END;
}

void
Vatic::Send() {
  BEG;
  DBG("In RUN with thread ID ", std::this_thread::get_id());
  std::uint32_t burstTimeUs = TRAFFIC_BURST_US;
  while (QUIT.load() == false) {
    auto startBurst = NOW();
    SendPdus();
    auto endBurst = NOW();
    ThreadSleepUs(burstTimeUs - DurationUs(endBurst, startBurst));
  }
  DBG("Sender finished!");
  END;
}

std::uint32_t
Vatic::ReceiveSdu(const char* buff, std::uint32_t n) {
  BEG;
  std::lock_guard<std::mutex> guard(m_bufferMtx);
  std::uint32_t remaining = m_conf.m_bufferSize - m_bufferOffset;
  std::uint32_t toInsert = std::min(n, remaining);
  if (toInsert <= 0) {
    return 0;
  }
  memcpy(&m_buffer[m_bufferOffset], buff, toInsert);
  m_bufferOffset += toInsert;
  if (m_recvThput.m_started == false) {
    m_recvThput.m_started = true;
    m_recvThput.m_start = NOW();
    m_sendThput.m_start = NOW();
  }
  m_recvThput.m_finish = NOW();
  m_recvThput.m_nBytes += n;
  END;
  return toInsert;
}

void
Vatic::SendPdus() {
  BEG;
  std::lock_guard<std::mutex> bufferGuard(m_bufferMtx);
  std::lock_guard<std::mutex> cwndGuard(m_cwndMtx);
  //  auto dataToSend = std::min<std::uint32_t>(m_cwnd.m_cwndFree, m_bufferOffset);
  auto dataToSend = m_cwnd.m_cwndFree;
  if (dataToSend <= 0) {
    return;
  }
  if (m_sendThput.m_started == false) {
    m_sendThput.m_started = true;
    m_sendThput.m_start = NOW();
  }
  m_sendThput.m_finish = NOW();
  m_sendThput.m_nBytes += dataToSend;
  while (dataToSend > 0) {
    // Take out bytes from buffer
    auto payloadLen = std::min<std::uint32_t>(dataToSend, MAX_PAYLOAD_LEN);
    // Prepare sending buffer
    PktHdr hdr;
    hdr.m_id = htobe64(m_currPktId);
    std::uint32_t offset = 0;
    memcpy(&m_sendingBuffer[offset], &hdr, sizeof (PktHdr));
    offset += sizeof (PktHdr);
    //    m_bufferOffset -= payloadLen;
    //    memcpy(&m_sendingBuffer[offset], &m_buffer[m_bufferOffset], payloadLen);
    offset += payloadLen;
    //    MSG_ASSERT(payloadLen + HDR_LEN == offset,
    //            "Error payload calculation: payload " + std::to_string(payloadLen)
    //            + " offset " + std::to_string(offset));
    ssize_t s = sendto(m_socketSend, m_sendingBuffer, offset, 0, (sockaddr *) & m_addrSend, sizeof (sockaddr));
    MSG_ASSERT(s == offset, "Server sent error");
    dataToSend -= payloadLen;
    DBG("Sending packet ", m_currPktId);
    m_cwnd.m_timeSentInflight.insert({m_currPktId, TUPLE(NOW(), payloadLen)});
    ++m_currPktId;
  }
  auto usedBytes = 0;
  for (auto& item : m_cwnd.m_timeSentInflight) {
    usedBytes += std::get<1>(item.second);
  }
  m_cwnd.m_cwndUsed = usedBytes;
  m_cwnd.m_cwndFree = m_cwnd.m_cwndUsed < m_cwnd.m_cwndSize ? m_cwnd.m_cwndSize - m_cwnd.m_cwndUsed : 0u;
  m_logCwnds.push_back(TUPLE(NOW(), m_cwnd.m_cwndSize, m_cwnd.m_cwndUsed, m_cwnd.m_cwndFree));
  if (m_currPktId % 1000 == 0) {
    std::cout << "-- Send up to " << m_currPktId << std::endl;
  }
  END;
}

void
Vatic::Receive(const char* pkt, std::uint32_t len) {
  BEG;
  PktHdr* hdr;
  hdr = (PktHdr*) pkt;
  if (hdr->m_pktType != PKT_TYPE::ACK) {
    WARN("Received DATA from client");
    return;
  }
  std::uint64_t id = be64toh(hdr->m_id);
  auto now = NOW();

  std::uint64_t rttMs = 1e6;
  bool losses = false;
  {// this is to unlock the mutexes while recomputing cwnd and centroids
    std::lock_guard<std::mutex> cwndGuard(m_cwndMtx);
    MSG_ASSERT(MapExist(m_cwnd.m_timeSentInflight, id), "Packet " + std::to_string(id) + " is not in inflight packet times");

    rttMs = DurationMs(now, std::get<0>(m_cwnd.m_timeSentInflight.at(id)));
    m_logRtts.push_back(TUPLE(now, rttMs, id));
    m_cwnd.m_timeSentInflight.erase(id);

    auto usedBytes = 0;
    losses = false;
    for (auto it = m_cwnd.m_timeSentInflight.cbegin(); it != m_cwnd.m_timeSentInflight.cend();) {
      if (it->first >= id) {
        usedBytes += std::get<1>(it->second);
        ++it;
      } else {
        losses = true;
        m_cwnd.m_timeSentInflight.erase(it++);
      }
    }
    m_cwnd.m_cwndUsed = usedBytes;
    m_cwnd.m_cwndFree = m_cwnd.m_cwndUsed < m_cwnd.m_cwndSize ? m_cwnd.m_cwndSize - m_cwnd.m_cwndUsed : 0u;
    m_logCwnds.push_back(TUPLE(NOW(), m_cwnd.m_cwndSize, m_cwnd.m_cwndUsed, m_cwnd.m_cwndFree));
  }
  ReportAck(rttMs, losses);
  END;
}

void
Vatic::ReportAck(std::uint32_t rttms, bool loss) {
  BEG;
  // if losses, reduce cwdn
  if (loss) {
    WARN("Detected loss!!");
    std::cout << "Detected losses!!!" << std::endl;
    std::lock_guard<std::mutex> cwndGuard(m_cwndMtx);
    m_cwnd.m_cwndSize *= (1 - m_alpha / 2);
    m_cwnd.m_cwndSize = std::max(m_cwnd.m_cwndSize, m_conf.m_initCwnd);
    m_cwnd.m_cwndFree = m_cwnd.m_cwndUsed < m_cwnd.m_cwndSize ? m_cwnd.m_cwndSize - m_cwnd.m_cwndUsed : 0u;
    m_logCwnds.push_back(TUPLE(NOW(), m_cwnd.m_cwndSize, m_cwnd.m_cwndUsed, m_cwnd.m_cwndFree));
    return;
  }
  // Process rtt
  auto rtt_feature = rttms;
  if (m_conf.m_preprocess == "EXP") {
    rtt_feature = std::exp(rttms);
  } else if (m_conf.m_preprocess == "TANH") {
    rtt_feature = std::tanh(rttms);
  }
  // Insert in the backlog queue
  if (m_backlog.size() == m_conf.m_backlogLen) {
    m_backlog.pop_back();
  }
  m_backlog.push_front({rtt_feature, INVALID_LABEL});
  // First centroids values
  if (m_centroids.first == INVALID_CENTROID and m_centroids.second == INVALID_CENTROID) {
    m_centroids.first = rtt_feature;
  } else if (m_centroids.first != INVALID_CENTROID and m_centroids.second == INVALID_CENTROID) {
    if (rtt_feature > 1.1 * (m_centroids.first)) {
      m_centroids = {m_centroids.first, rtt_feature};
    } else if (rtt_feature < 0.9 * (m_centroids.first)) {
      m_centroids = {rtt_feature, m_centroids.first};
    }
  } else {// Normal case
    // compute distances to old centroids and assign labels
    for (auto& item : m_backlog) {
      if (!std::isfinite(item.first)) {
        continue;
        std::cout << "Infinite value !!!!" << std::endl;
        std::exit(-1);
      }
      auto distC1 = std::abs(item.first - m_centroids.first);
      auto distC2 = std::abs(item.first - m_centroids.second);
      item.second = distC1 <= distC2 ? LOW_CONG_LABEL : HIGH_CONG_LABEL;
    }
    // compute the new centroids after the new assignment
    RecomputeCentroids();
    m_logAlpha.push_back(TUPLE(NOW(), m_alpha));
    UpdateAlpha();
  }
  m_logCentroids.push_back(TUPLE(NOW(), m_centroids.first, m_centroids.second));

  std::lock_guard<std::mutex> cwndGuard(m_cwndMtx);
  if (m_alpha > m_conf.m_alphaThres) {
    //    std::cout << TimeStampMs() << " | Decreasing cwnd from " << m_cwnd.m_cwndSize << " to " << m_cwnd.m_cwndSize * (1 - m_alpha / 2.0) << std::endl;
    m_cwnd.m_cwndSize *= (1 - m_alpha / 2.0);
    m_cwnd.m_cwndSize = std::max(m_cwnd.m_cwndSize, m_conf.m_initCwnd);
  } else {
    //    std::cout << TimeStampMs() << " | Increasing cwnd from " << m_cwnd.m_cwndSize << " to " << m_cwnd.m_cwndSize + (m_conf.m_cwndMax - m_cwnd.m_cwndSize) / 2.0 << std::endl;
    m_cwnd.m_cwndSize += (m_conf.m_cwndMax - m_cwnd.m_cwndSize) / 2.0;
    m_cwnd.m_cwndSize = std::min(m_cwnd.m_cwndSize, m_conf.m_cwndMax);
  }
  m_cwnd.m_cwndFree = m_cwnd.m_cwndUsed < m_cwnd.m_cwndSize ? m_cwnd.m_cwndSize - m_cwnd.m_cwndUsed : 0u;
  m_logCwnds.push_back(TUPLE(NOW(), m_cwnd.m_cwndSize, m_cwnd.m_cwndUsed, m_cwnd.m_cwndFree));
  END;
}

void
Vatic::RecomputeCentroids() {
  BEG;
  auto accCentroidLow = 0.0;
  auto ctrCentroidLow = 0u;
  auto accCentroidHigh = 0.0;
  auto ctrCentroidHigh = 0u;
  for (auto &item : m_backlog) {
    if (!std::isfinite(item.first)) {
      continue;
    }

    if (item.second == LOW_CONG_LABEL) {
      accCentroidLow += item.first;
      ++ctrCentroidLow;
    } else { // item.second == LOW_CONG_LABEL
      accCentroidHigh += item.first;
      ++ctrCentroidHigh;
    }
  }
  auto aux0 = ctrCentroidLow > 0 ? accCentroidLow / ctrCentroidLow : m_centroids.first;
  auto aux1 = ctrCentroidHigh > 0 ? accCentroidHigh / ctrCentroidHigh : m_centroids.second;

  //if existing dist between centroids is greater, then
  //don't update the cluster centroids, else update the
  //centroids
  auto newDist = std::abs(aux0 - aux1);
  if (m_maxCentDist < newDist) {
    m_centroids = {aux0, aux1};
    m_maxCentDist = std::abs(m_centroids.first - m_centroids.second);
  }
  END;
}

void
Vatic::UpdateAlpha() {
  BEG;
  auto highCtr = 0u;
  auto allCtr = 0u;
  for (auto &item : m_backlog) {
    if (!std::isfinite(item.first)) {
      continue;
    }
    if (item.second == HIGH_CONG_LABEL) {
      ++highCtr;
    }
    ++allCtr;
  }
  m_alpha = double(highCtr) / allCtr;
  END;
}