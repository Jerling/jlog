#ifndef __CLIENT_H_
#define __CLIENT_H_

#include "jlogd.h"

/* udp 客户端 */
class udp_client {
 public:
  udp_client(const char *sname, unsigned short port) {
    cfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    EXIT_IF(cfd_ < 0, "Get client socket failed\n");
    DEBUG("client socket fd is %d\n", cfd_);
    struct sockaddr_in server;
    EXIT_IF(!inet_aton(sname, &server.sin_addr), "Please ust IP addr\n");
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    socklen_t len = sizeof(server);
    EXIT_IF((connect(cfd_, (struct sockaddr *)&server, len)) != 0,
            "connect failed : %s\n", strerror(errno));
  }
  int get_cfd() const { return cfd_; }

 private:
  int cfd_;
};

#endif  // __CLIENT_H_
