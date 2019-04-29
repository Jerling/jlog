#ifndef __COMMON_H_
#define __COMMON_H_

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "gperftools/tcmalloc.h"

template <typename T>
static void deleter(T* obj) {
  if (obj) {
    obj->destory();
    obj = nullptr;
  }
}

namespace jlog {

enum log_level_t {
  emerg = 0,
  alert,
  crit,
  error,
  warning,
  notice,
  info,
  debug,
  nops
};

inline std::string now() {
  auto t_now = std::chrono::system_clock::now();
  auto t_time = std::chrono::system_clock::to_time_t(t_now);
  auto t_info = localtime(&t_time);
  char buffer[128];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t_info);
  return buffer;
};

const std::string note_msg =
    "[" + now() + "]" +
    "[NOTE] ^v^v^ The level or fds been ignored when use "
    "xxx_DTL , maybe you can setting logger's attrs :)\n";

inline std::string fmt(const char* fmt, ...) {
  char buf[1024];
  va_list vl;
  va_start(vl, fmt);
  vsprintf(buf, fmt, vl);
  va_end(vl);
  size_t len = 0;
  return buf;
}

template <typename... Args>
inline std::string fmt(log_level_t, Args... args) {
  std::string str = fmt(std::forward<Args>(args)...);
  if (str.back() != '\n') str += '\n';
  return str + note_msg;
}

template <typename... Args>
inline std::string fmt(std::unordered_set<int>, Args... args) {
  std::string str = fmt(std::forward<Args>(args)...);
  if (str.back() != '\n') str += '\n';
  return str + note_msg;
}
template <typename... Args>
inline std::string fmt(log_level_t, std::unordered_set<int>, Args... args) {
  std::string str = fmt(std::forward<Args>(args)...);
  if (str.back() != '\n') str += '\n';
  return str + note_msg;
}

}  // namespace jlog
#endif  // __COMMON_H_
