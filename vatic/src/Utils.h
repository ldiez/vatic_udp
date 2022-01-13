#ifndef UTILS_H
#define UTILS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <cerrno>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <unordered_map>
#include <experimental/filesystem>
#include <atomic>

namespace fs = std::experimental::filesystem;
//#define CLOCK std::chrono::steady_clock
#define CLOCK std::chrono::high_resolution_clock
//#define CLOCK std::chrono::system_clock
using TimePoint_t = std::chrono::time_point<CLOCK>;
#define NOW CLOCK::now

inline std::atomic_bool QUIT(false);
#define ON_ERROR -1

#define TUPLE std::make_tuple

struct Thput_t {
  bool m_started = false;
  TimePoint_t m_start;
  TimePoint_t m_finish;
  std::uint64_t m_nBytes = 0u;
};

inline void
DisplayError(const char *what) {
  std::cerr << strerror(errno) << ": " << what << "\n";
  std::cout << "Error \n";
  std::exit(-1);
}

inline void
AssertCode(int n, int err, const char *what) {
  if (n == err) {
    DisplayError(what);
  }
}

inline void
SignalHandler(int signal) {
  switch (signal) {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
    case SIGHUP:
      std::cout << "Stopping program pid " << getpid() << "\n"
              << "Signal raised: " << strsignal(signal) << "(" << signal << ")" << std::endl;
      QUIT.store(true);
      break;
    default:
      std::cout << "Ignoring signal " << strsignal(signal) << std::endl;
  }
}

inline void
ThreadSleepMs(std::uint64_t ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void
ThreadSleepUs(std::uint64_t us) {
  std::this_thread::sleep_for(std::chrono::microseconds(us));
}

inline void
ThreadSleepS(std::uint32_t s) {
  std::this_thread::sleep_for(std::chrono::seconds(s));
}

inline std::uint64_t
DurationUs (TimePoint_t end, TimePoint_t start){
  return std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
}

inline std::uint64_t
DurationMs (TimePoint_t end, TimePoint_t start){
  return std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
}

inline std::uint64_t
TimeStampMs (void){
  return std::chrono::duration_cast<std::chrono::milliseconds>(NOW().time_since_epoch()).count();
}

inline std::uint64_t
TimeStampMs (TimePoint_t tp){
  return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

inline std::uint64_t
TimeStampUs (void){
  return std::chrono::duration_cast<std::chrono::microseconds>(NOW().time_since_epoch()).count();
}

inline std::uint64_t
TimeStampUs (TimePoint_t tp){
  return std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count();
}

inline void
PrintSockaddr(const sockaddr_in& sa) {
  std::cout << "Address " << inet_ntoa(sa.sin_addr) << "\n";
  std::cout << "Port: " << be16toh(sa.sin_port) << "\n";
}

template <class M, class V>
bool MapExist(const M& m, const V& v) {
  if (m.find(v) != m.end()) {
    return true;
  }
  return false;
}

#endif /* UTILS_H */
