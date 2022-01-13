#include <cstdlib>

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Utils.h"
#include "PacketUtils.h"
#include "Client.h"
#include "Log.h"

void ReceiveCb(Client *cl, int socketRecv, sockaddr_in *addrRecv) {
  char buff[MAX_PKT_LEN];
  unsigned int lenInet = sizeof (sockaddr_in);
  while (QUIT.load() == false) {
    int pktLen = recvfrom(socketRecv, buff, sizeof (buff), 0, (sockaddr *) addrRecv, &lenInet);
    AssertCode(pktLen, ON_ERROR, "Error sendto()");
    if (pktLen > 0) {
      cl->Receive(buff, pktLen);
    }
  }
}

int main(int argc, char **argv) {

  LOG_SET_LEVEL("Client", LogLevel::DEBUG);

  std::signal(SIGINT, SignalHandler);
  std::signal(SIGQUIT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
  std::signal(SIGHUP, SignalHandler);

  po::options_description desc("Client general options");
  po::options_description clientDesc("Client options");

  std::uint16_t sendPort;
  std::uint16_t recvPort;
  std::string sendAddr("127.0.0.1");

  po::variables_map vm;
  desc.add_options()
          ("help", "The UDP client is configured by setting ")
          ("send_port", po::value<>(&sendPort)->default_value(8888), "")
          ("recv_port", po::value<>(&recvPort)->default_value(9999), "")
          ("send_addr", po::value<>(&sendAddr)->default_value("127.0.0.1"), "");
  clientDesc.add_options()
          ("log_path", po::value<std::string>()->default_value("./results"), "")
          ("log_name", po::value<std::string>()->default_value("client"), "");

  po::options_description all("Allowed options");
  all.add(desc);
  all.add(clientDesc);

  // parse arguments
  po::store(po::parse_command_line(argc, argv, all), vm);

  try {
    po::notify(vm);
  } catch (std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    std::cout << desc << std::endl;
    std::cout << clientDesc << std::endl;
    return 1;
  }

  ClientConf tConf = LoadClientConf(vm);

  sockaddr_in addrSend;
  sockaddr_in addrRecv;
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

  auto check = bind(socketRecv, (sockaddr *) & addrRecv, sizeof (sockaddr));
  AssertCode(check, ON_ERROR, "Error bind()");

  { // Establish the connection and adapt sockaddr information
    //    PktConnection sendPkt;
    //    sendPkt.m_hdr.m_id = htobe64(0);
    //    sendPkt.m_hdr.m_pktType = PKT_TYPE::CONNECTION;
    //    sendPkt.m_listeningPort = htobe16(recvPort);
    //    char buff[MAX_PKT_SIZE];
    //    unsigned int lenInet = sizeof (sockaddr_in);
    //
    //    UINFO("Server listening at ", inet_ntoa(addrSend.sin_addr), "::", be16toh(addrSend.sin_port));
    //    ssize_t s = sendto(socketSend, (char*) &sendPkt, sizeof (PktConnection), 0, (sockaddr*) & addrSend, sizeof (sockaddr));
    //    ssize_t pktLen = recvfrom(socketRecv, buff, sizeof (buff), 0, (sockaddr *) & addrSend, &lenInet);
    //    UINFO("Server sending from ", inet_ntoa(addrSend.sin_addr), "::", be16toh(addrSend.sin_port));
    //    //
    //    AssertCode(pktLen, ON_ERROR, "Error recvfrom()");
    //    PktConnection* recvkt = (PktConnection*) buff;
    //    //
    //    if (recvkt->m_hdr.m_pktType != PKT_TYPE::CONNECTION) {
    //      UERROR("First received packet is not a connection");
    //      UINFO(int(recvkt->m_hdr.m_pktType));
    //      std::exit(0);
    //    }
    //    addrSend.sin_port = recvkt->m_listeningPort;
  }
  std::cout << "====================================" << std::endl;
  std::cout << "Client ready to  send to " << std::endl;
  PrintSockaddr(addrSend);
  std::cout << tConf;
  std::cout << "====================================" << std::endl;

  Client cl(tConf, addrSend, socketSend);
//  auto sendTrhead = std::thread(&Client::Send, &cl);

  auto recvTrhead = std::thread(ReceiveCb, &cl, socketRecv, &addrRecv);
  while (QUIT.load() == false) {
    //    UINFO("here I am");
    ThreadSleepS(1);
  }
  // This is to unblock receiving threads
  shutdown(socketRecv, SHUT_RDWR);
  recvTrhead.join();
//  sendTrhead.join();
  close(socketRecv);
  close(socketSend);
  return 0;
}
