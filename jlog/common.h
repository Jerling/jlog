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

inline std::string now() {
  auto t_now = time(nullptr);
  auto t_info = localtime(&t_now);
  char buffer[128];
  strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", t_info);
  return buffer;
};

#define TIME_INFO "[" << now() << "]"
#define FILE_INFO "[" << __FILE__ << ":" << __LINE__ << "]"

template <typename T>
static void deleter(T* obj) {
  if (obj) {
    obj->destory();
    obj = nullptr;
  }
}

#endif  // __COMMON_H_
