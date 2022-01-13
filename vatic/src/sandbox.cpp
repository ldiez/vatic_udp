#include <cstdlib>
#include <queue>

#include "ConfUtils.h"
#include "Vatic.h"
#include "Log.h"
#include "TrafficSource.h"

using namespace std;

int main(int argc, char** argv) {

  LOG_SET_LEVEL("Vatic", LogLevel::ALL);
  LOG_SET_LEVEL("TrafficSource", LogLevel::ALL);
  VaticConf vConf;
  vConf.m_alphaThres = 0.5;
  vConf.m_backlog = 10;
  vConf.m_bufferSize = 1e6;
  vConf.m_cwndMax = 1e6;
  vConf.m_preprocess = "NONE";
  vConf.m_vaticLogs = "./results";

  TrafficConf tConf;
  tConf.m_numPktsBlock = 3;
  tConf.m_interBlockTime = 2000;
  tConf.m_numBlocks = 2;
  tConf.m_sendRate =7.8125;
  tConf.m_pktSize = 1000;

//  Vatic v(vConf);
//  std::thread vThread = std::thread(&Vatic::Run, &v);
//  
//  TrafficSource ts(tConf);
//  
//  Vatic::API::Send_t sf = std::bind(&Vatic::ReceiveSdu, &v, std::placeholders::_1, std::placeholders::_2);
//  ts.SetCallback(sf);
//  ts.Run();
//
//  vThread.join();


  return 0;
}

