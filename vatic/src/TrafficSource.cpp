#include <thread>
#include <chrono>
#include "TrafficSource.h"
#include "PacketUtils.h"
#include "Utils.h"
#include "Log.h"

LOG_REGISTER_MODULE("TrafficSource")


TrafficSource::TrafficSource(TrafficConf& c)
: m_conf(c) {
  BEG;
  m_pkt.resize(m_conf.m_pktSize, 'v');

  std::uint64_t pktTimeUs = double(m_conf.m_pktSize * 8) / m_conf.m_sendRate;
  std::cout << "Time between packets " << pktTimeUs << " us" << std::endl;
  m_actualBurstTimeUs = TRAFFIC_BURST_US;
  m_pktsPerBurst = 1;
  if (pktTimeUs > m_actualBurstTimeUs) {
    m_actualBurstTimeUs = pktTimeUs;
  } else {
    m_pktsPerBurst = std::min(m_actualBurstTimeUs / pktTimeUs, std::uint64_t(m_conf.m_numPktsBlock));
  }
  END;
}

TrafficSource::~TrafficSource() {
  BEG;
  //  auto elapsedSendUs = DurationUs(m_thput.m_finish, m_thput.m_start);
  //  std::cout << "Send Elapsed time " << elapsedSendUs << " us\n";
  //  std::cout << "Send Number of bytes: " << m_thput.m_nBytes << "\n";
  //  std::cout << "Send Throuhput: " << (double(m_thput.m_nBytes * 8) / double(elapsedSendUs)) << " Mbps\n";
  END;
}

void
TrafficSource::Send() {
  BEG;
  MSG_ASSERT(m_sendCallback, "Vatic send callback not set");
  if (m_conf.m_sendTime > 0) {
    DoSendTime();
  } else {
    DoSendBlocks();
  }


  END;
}

void
TrafficSource::SetCallback(Vatic::TrafficApi::Send_t& vs) {
  BEG;
  m_sendCallback = vs;
  END;
}

void TrafficSource::DoSendTime() {
  BEG;
  auto start = NOW();
  auto bCtr = 0u;
  while (DurationMs(NOW(), start) / 1000.0 < m_conf.m_sendTime && QUIT.load() == false) {
    DoSendBurst();
    ++bCtr;
  }
  m_thput.m_finish = NOW();
  m_thput.m_start = start;
  m_thput.m_nBytes = bCtr * m_pktsPerBurst * m_conf.m_pktSize;
  END;
}

void TrafficSource::DoSendBlocks() {
  BEG;
  m_thput.m_start = NOW();
  for (auto bn = 0u; bn < m_conf.m_numBlocks; ++bn) {
    if (QUIT.load() == true) {
      return;
    }
    DBG("Send Block ", bn);
    DoSendBlock();
    m_thput.m_finish = NOW();
    ThreadSleepMs(m_conf.m_interBlockTime);
  }
  m_thput.m_nBytes = m_conf.m_numBlocks * m_conf.m_numPktsBlock * m_conf.m_pktSize;
  std::cout << "I'm done with sending. Quitting" << std::endl;
  END;
}

void
TrafficSource::DoSendBlock() {
  BEG;
  for (auto npkt = 0u; npkt < m_conf.m_numPktsBlock; npkt += m_pktsPerBurst) {
    DoSendBurst();
  }
  END;
}

void TrafficSource::DoSendBurst() {
  BEG;
  auto startBurst = NOW();
  if (QUIT.load() == true) {
    return;
  }
  for (auto i = 0u; i < m_pktsPerBurst; ++i) {
    m_sendCallback(m_pkt.c_str(), m_pkt.size());
  }
  auto endBurst = NOW();
  ThreadSleepUs(m_actualBurstTimeUs - DurationUs(endBurst, startBurst));
  END;
}

