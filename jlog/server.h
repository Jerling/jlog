#ifndef __SERVER_H_
#define __SERVER_H_

#include "jlogd.h"

/* udp 服务器 */
class udp_server {
 public:
  udp_server(const char*, unsigned short);
  int get_fd() const { return sfd_; }
  void start();
  ~udp_server() { close(sfd_); }

 private:
  int sfd_;
};

#endif  // __SERVER_H_
