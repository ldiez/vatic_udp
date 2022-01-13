#include <cstdlib>
#include <csignal>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

#include "ConfUtils.h"
#include "Utils.h"
#include "TrafficSource.h"
#include "Log.h"
#include "PacketUtils.h"

void ReceiveCb(Vatic *vatic, int socketRecv, sockaddr_in *addrRecv) {
  char buff[MAX_PKT_LEN];
  unsigned int lenInet = sizeof (sockaddr);
  while (QUIT.load() == false) {
    auto pktLen = recvfrom(socketRecv, buff, sizeof (buff), 0, (sockaddr *) addrRecv, &lenInet);
    AssertCode(pktLen, ON_ERROR, "Error sendto()");
    if (pktLen > 0) {
      vatic->Receive(buff, pktLen);
    }
  }
  UINFO("Quitting receiving thread");
}

int main(int argc, char **argv) {

  //  LOG_SET_LEVEL("TrafficSource", LogLevel::WARNING);
    LOG_SET_LEVEL("Vatic", LogLevel::DEBUG);

  std::signal(SIGINT, SignalHandler);
  std::signal(SIGQUIT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
  std::signal(SIGHUP, SignalHandler);

  po::options_description servDesc("Server general options");
  po::options_description trafficDesc("Traffic options");
  po::options_description vaticDesc("Vatic options");

  std::uint16_t sendPort;
  std::uint16_t recvPort;
  std::string sendAddr;

  po::variables_map vm;
  // General options
  servDesc.add_options()
          ("help", "The UDP server is configured by setting just the port: --port or -p")
          ("send_port", po::value<>(&sendPort)->default_value(9999), "Client listening port")
          ("recv_port", po::value<>(&recvPort)->default_value(8888), "Client listening port")
          ("send_addr", po::value<>(&sendAddr)->default_value("127.0.0.1"), "Client Address");
  //  Vatic options
  vaticDesc.add_options()
          ("alpha_thres", po::value<double>()->default_value(0.5), "Vatic Alpha value")
          ("backlog_len", po::value<std::uint32_t>()->default_value(10u), "")
          ("buffer_size", po::value<std::uint32_t>()->default_value(1e9), "")
          ("cwnd_max", po::value<std::uint32_t>()->default_value(1e6), "Vatic Maximum Congestion window in bytes")
          ("init_cwnd", po::value<std::uint32_t>()->default_value(1e3), "Vatic Maximum Congestion window in bytes")
          ("preprocess", po::value<std::string>()->default_value("NONE"), "RTT preprocessing policy: NONE, TANH, EXP")
          ("log_path", po::value<std::string>()->default_value("./results"), "")
          ("log_name", po::value<std::string>()->default_value("server"), "");
  //  application options
  trafficDesc.add_options()
          ("num_pkts_block", po::value<std::uint32_t>()->default_value(1e6), "Number of packets per block")
          ("inter_block_time", po::value<std::uint32_t>()->default_value(1), "Waiting time since the end of sending a block, until starting sending a new one in seconds")
          ("num_blocks", po::value<std::uint32_t>()->default_value(1), "Number of blocks to send per connection")
          ("send_time", po::value<std::uint32_t>()->default_value(0), "Sending time in seconds. It overwrites the number of blocks and packets")
          ("send_rate", po::value<double>()->default_value(1), "Sending rate in Mbps")
          ("pkt_size", po::value<std::uint32_t>()->default_value(1000), "Size of application packets in bytes");

  po::options_description all("Allowed options");
  all.add(servDesc);
  all.add(vaticDesc);
  all.add(trafficDesc);

  // parse arguments
  po::store(po::parse_command_line(argc, argv, all), vm);

  try {
    po::notify(vm);
  } catch (std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    std::cout << servDesc << std::endl;
    std::cout << vaticDesc << std::endl;
    std::cout << trafficDesc << std::endl;
    return 1;
  }

  TrafficConf tConf = LoadTrafficConf(vm);
  VaticConf vConf = LoadVaticConf(vm);

  sockaddr_in addrRecv;
  sockaddr_in addrSend;
  memset(&addrSend, 0, sizeof (sockaddr_in));
  memset(&addrRecv, 0, sizeof (sockaddr_in));

  addrSend.sin_family = AF_INET;
  addrSend.sin_port = htobe16(sendPort);
  addrSend.sin_addr.s_addr = inet_addr(sendAddr.c_str());
  AssertCode(addrSend.sin_addr.s_addr, INADDR_NONE, "INADDR_NONE");

  addrRecv.sin_family = AF_INET;
  addrRecv.sin_port = htobe16(recvPort);
  addrRecv.sin_addr.s_addr = INADDR_ANY;

  auto socketSend = socket(AF_INET, SOCK_DGRAM, 0);
  auto socketRecv = socket(AF_INET, SOCK_DGRAM, 0);
  AssertCode(socketSend, ON_ERROR, "Error socket()");
  AssertCode(socketRecv, ON_ERROR, "Error socket()");

  int check = bind(socketRecv, (sockaddr *) & addrRecv, sizeof (sockaddr));
  AssertCode(check, ON_ERROR, "Error bind()");

  { // Establish connection and adapt sockaddr information
    //    char buff[MAX_PKT_SIZE];
    //    unsigned int lenInet = sizeof (addrSend);
    //    ssize_t pktLen = recvfrom(socketRecv, buff, sizeof (buff), 0, (sockaddr*) & addrSend, &lenInet);
    //    AssertCode(pktLen, ON_ERROR, "Error recvfrom()");
    //    PktConnection* pkt = (PktConnection*) buff;
    //    if (pkt->m_hdr.m_pktType != PKT_TYPE::CONNECTION) {
    //      UERROR("First received packet from a client is not a connection");
    //      std::exit(0);
    //    }
    //    UINFO("Client sending from at --> ", inet_ntoa(addrSend.sin_addr), "::", be16toh(addrSend.sin_port));
    //    addrSend.sin_port = pkt->m_listeningPort;
    //    pkt->m_listeningPort = addrRecv.sin_port;
    //    UINFO("Client listening at --> ", inet_ntoa(addrSend.sin_addr), "::", be16toh(addrSend.sin_port));
    //    ssize_t s = sendto(socketSend, buff, sizeof (PktConnection), 0, (sockaddr *) & addrSend, sizeof (sockaddr));
  }
  std::cout << "====================================" << std::endl;
  std::cout << "Server ready to  send to " << std::endl;
  PrintSockaddr(addrSend);
  std::cout << tConf;
  std::cout << vConf;
  std::cout << "====================================" << std::endl;
  // Create the Vatic instance
  Vatic vatic(vConf, addrSend, socketSend);
  auto sendThead = std::thread(&Vatic::Send, &vatic);
  //  Create the traffic source(application)
  //  TrafficSource traffSrc(tConf);

  //  Vatic::TrafficApi::Send_t sf = std::bind(&Vatic::ReceiveSdu, &vatic, std::placeholders::_1, std::placeholders::_2);
  //  traffSrc.SetCallback(sf);
  //
  //  auto trafficThread = std::thread(&TrafficSource::Send, &traffSrc);
  //
  auto recvTrhead = std::thread(ReceiveCb, &vatic, socketRecv, &addrRecv);

  //  trafficThread.join();
  
  
  ThreadSleepS(tConf.m_sendTime);
  QUIT.store(true);
  shutdown(socketRecv, SHUT_RDWR);
  shutdown(socketSend, SHUT_RDWR);
  std::cout << "Traffic thread finished!!" << std::endl;
  
  if (sendThead.joinable()) {
    sendThead.join();
  }
  if (recvTrhead.joinable()) {
    recvTrhead.join();
  }

  close(socketRecv);
  close(socketSend);
  return 0;
}
