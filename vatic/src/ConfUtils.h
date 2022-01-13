#ifndef CONFUTILS_H
#define CONFUTILS_H

#include <cstdint>
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct TrafficConf {
  std::uint32_t m_numPktsBlock; // 
  std::uint32_t m_interBlockTime; // time in milli seconds 
  std::uint32_t m_numBlocks; // 
  std::uint32_t m_sendTime; // in seconds
  double m_sendRate; // in kbps (1E3)
  std::uint32_t m_pktSize; // in bytes  
};

struct VaticConf {
  double m_alphaThres; // between 0 and 1
  std::uint32_t m_backlogLen; // 
  std::uint32_t m_bufferSize; // in bytes
  std::uint32_t m_cwndMax; // in bytes
  std::uint32_t m_initCwnd; // initial cwnd in bytes
  std::string m_preprocess; // NONE, TANH, EXP
  std::string m_vaticLogs; // path
  std::string m_logPath;
  std::string m_logName;
};

struct ClientConf {
  std::string m_logPath;
  std::string m_logName;
};

std::ostream& operator<<(std::ostream& os, const TrafficConf& e);
std::ostream& operator<<(std::ostream& os, const VaticConf& e);
std::ostream& operator<<(std::ostream& os, const ClientConf& e);

TrafficConf LoadTrafficConf(const po::variables_map& desc);
VaticConf LoadVaticConf(const po::variables_map& desc);
ClientConf LoadClientConf(const po::variables_map& desc);

#endif /* CONFUTILS_H */

