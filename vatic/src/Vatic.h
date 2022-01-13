#ifndef VATIC_H
#define VATIC_H

#include <mutex>
#include <arpa/inet.h>
#include <unordered_map>
#include <chrono>
#include <queue>

#include "Utils.h"
#include "ConfUtils.h"

class Vatic {
public:

  struct TrafficApi {
    using Send_t = std::function<void(const char*, std::uint32_t)>;
  };

  Vatic(VaticConf vc, sockaddr_in& addrSend, int socketSend);
  ~Vatic();
  void Send();

  std::uint32_t ReceiveSdu(const char* buff, std::uint32_t n);

  void SendPdus();

  void Receive(const char* pkt, std::uint32_t len); //
private:
  using TimePoint_t = std::chrono::time_point<std::chrono::system_clock>; 

  void PrintRtts(); 
  void PrintCwnd();
  void PrintAlpha();
  void PrintCentroids();

  struct Cwnd {
    std::uint32_t m_cwndSize;
    std::uint32_t m_cwndFree;
    std::uint32_t m_cwndUsed;
    std::map<std::uint64_t, std::tuple<TimePoint_t, std::uint32_t>> m_timeSentInflight;
  };
  //  void ReduceCwnd();
  void ReportAck(std::uint32_t rttms, bool loss = false);
  void ReportLoss ();
  void RecomputeCentroids();
  void UpdateAlpha();
    
  const VaticConf m_conf;

  const sockaddr_in m_addrSend;
  const int m_socketSend;
  
  std::uint64_t m_currPktId;
  
  char* m_buffer;
  std::uint32_t m_bufferOffset;
  char* m_sendingBuffer;
  std::mutex m_bufferMtx;
  
  Cwnd m_cwnd;
  std::mutex m_cwndMtx;


  double m_maxCentDist;
  double m_alpha;

  std::deque<std::pair<double, int16_t>> m_backlog;
  std::pair<double, double> m_centroids;
  
  std::vector<std::tuple<TimePoint_t, std::uint32_t, std::uint64_t>> m_logRtts;
  std::vector<std::tuple<TimePoint_t, std::uint32_t, std::uint32_t, std::uint32_t>> m_logCwnds;
  std::vector<std::tuple<TimePoint_t, double>> m_logAlpha;
  std::vector<std::tuple<TimePoint_t, double, double>> m_logCentroids;
  
  Thput_t m_recvThput;
  Thput_t m_sendThput;

};
 
#endif /* VATIC_H */

