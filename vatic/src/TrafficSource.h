#ifndef TESTSTREAM_H
#define TESTSTREAM_H

#include <netinet/in.h>

#include "Vatic.h"

class TrafficSource {
public:
  
  TrafficSource(TrafficConf& c);
  ~TrafficSource();
  void Send ();
  void SetCallback (Vatic::TrafficApi::Send_t& vs);
private:
  void DoSendTime ();
  void DoSendBlocks ();
  void DoSendBlock ();
  void DoSendBurst ();
  TrafficConf m_conf;
  Vatic::TrafficApi::Send_t m_sendCallback;
  Thput_t m_thput;
  std::string m_pkt;
  std::uint32_t m_pktsPerBurst;
  std::uint64_t m_actualBurstTimeUs;
};

#endif /* TESTSTREAM_H */

