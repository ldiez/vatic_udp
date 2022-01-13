#ifndef CLIENT_H
#define CLIENT_H

#include <mutex>
#include <arpa/inet.h>
#include <deque>
#include "ConfUtils.h"
#include "Utils.h"

class Client {

  struct AckCount {
    std::uint32_t m_lastAcked;
    std::uint32_t m_lastNacked;
  };
public:
  Client(ClientConf c, sockaddr_in& addrSend, int socketSend);
  ~Client();
  void Send();
  void SendWithId(std::uint64_t pktId);
  // One thread to send back ACKs
  // One send to receive
  void Receive(const char* pkt, std::uint32_t len);

  void PrintRecvTimes();
private:
  
  
  ClientConf m_conf;

  sockaddr_in m_addrSend;
  int m_socketSend;

  std::mutex m_ackCountMtx;
  AckCount m_ackCount;


  std::vector<std::tuple<std::uint64_t, TimePoint_t, std::uint32_t>> m_recvTimes;

  Thput_t m_recvThput;


};

#endif /* CLIENT_H */

