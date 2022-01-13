#include "ConfUtils.h"
#include "Log.h"

LOG_REGISTER_MODULE("ConfUtils")

std::ostream&
operator<<(std::ostream& os, const TrafficConf& e) {
  return os
          << "==== Traffic configuration ====\n"
          << "  Number of Packets per Blocks: " << e.m_numPktsBlock << "\n"
          << "  Inter Block Time: " << e.m_interBlockTime << " ms\n"
          << "  Number of Blocks " << e.m_numBlocks << "\n"
          << "  Sending Rate " << e.m_sendRate << " kbps\n"
          << "  Packet Size: " << e.m_pktSize << " B\n"
          ;
}

std::ostream&
operator<<(std::ostream& os, const VaticConf& e) {
  return os
          << "==== Vatic configuration ====\n"
          << "  Alpha Thresh: " << e.m_alphaThres << "\n"
          << "  Backlog Length: " << e.m_backlogLen << "\n"
          << "  Buffer Size: " << e.m_bufferSize << " B\n"
          << "  CWND Max: " << e.m_cwndMax << " B\n"
          << "  Init CWND: " << e.m_initCwnd << " B\n"
          << "  RTT Preprocess: " << e.m_preprocess << "\n"
          << "  Log Path: " << e.m_logPath << "\n"
          << "  Log Name: " << e.m_logName << "\n"
          ;
}

std::ostream&
operator<<(std::ostream& os, const ClientConf& e) {
  return os
          << "==== Client configuration ====\n"
          << "  Log Path: " << e.m_logPath << "\n"
          << "  Log Name: " << e.m_logName << "\n"
          ;
}

TrafficConf
LoadTrafficConf(const po::variables_map & vm) {
  TrafficConf conf;
  conf.m_numPktsBlock = vm["num_pkts_block"].as<std::uint32_t>();
  conf.m_interBlockTime = vm["inter_block_time"].as<std::uint32_t>();
  conf.m_numBlocks = vm["num_blocks"].as<std::uint32_t>();
  conf.m_sendRate = vm["send_rate"].as<double>();
  conf.m_sendTime = vm["send_time"].as<std::uint32_t>();
  conf.m_pktSize = vm["pkt_size"].as<std::uint32_t>();
  return conf;
}

VaticConf
LoadVaticConf(const po::variables_map & vm) {
  VaticConf conf;
  conf.m_alphaThres = vm["alpha_thres"].as<double>();
  conf.m_backlogLen = vm["backlog_len"].as<std::uint32_t>();
  conf.m_bufferSize = vm["buffer_size"].as<std::uint32_t>();
  conf.m_cwndMax = vm["cwnd_max"].as<std::uint32_t>();
  conf.m_initCwnd = vm["init_cwnd"].as<std::uint32_t>();
  conf.m_preprocess = vm["preprocess"].as<std::string>();
  conf.m_logPath = vm["log_path"].as<std::string>();
  conf.m_logName = vm["log_name"].as<std::string>();
  return conf;
}

ClientConf
LoadClientConf(const po::variables_map& vm) {
  ClientConf conf;
  conf.m_logPath = vm["log_path"].as<std::string>();
  conf.m_logName = vm["log_name"].as<std::string>();
  return conf;
}
