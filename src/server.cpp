#include "server.h"

udp_server::udp_server(const char *hname, unsigned short port) {
  struct sockaddr_in local;
  local.sin_port = htons(port);
  local.sin_family = AF_INET;
  if (hname == nullptr)
    local.sin_addr.s_addr = INADDR_ANY;
  else {
    EXIT_IF(!inet_aton(hname, &local.sin_addr),
            "Please give a correct ip like:127.0.0.1\n");
  }
  DEBUG("use IP=%s port=%d\n", (char *)inet_ntoa(local.sin_addr), port);
  EXIT_IF(daemon(0, 0) == -1, "Daemon error");
  sfd_ = socket(AF_INET, SOCK_DGRAM, 0);
  EXIT_IF((sfd_ < 0), "udp_server socket failed\n");
  DEBUG("The server socket fd is %d\n", sfd_);
  int re;
  EXIT_IF((re = bind(sfd_, (struct sockaddr *)&local, sizeof(local)) != 0),
          "udp_server bind failed: %d: %s\n", re, strerror(errno));
}

void udp_server::start() {
  DEBUG("jlogd is starting\n");
  /* 客户端地址 */
  struct sockaddr_in cliaddr;
  socklen_t clilen = sizeof(cliaddr);

  /* 消息缓冲 */
  char buff[MAXLINE];

  /* 确定日志输出目录 */
  mkdir(log_dir, RWRWRW | S_IXUSR | S_IXGRP | S_IXOTH);
  logfd = dirfd(opendir(log_dir));
  EXIT_IF(logfd < 0, "logfd at %s get failed\n", log_dir);

  /* 启动后台线程 */
  std::thread t([]() {
    for (;;) {
      DEBUG("handing a packet\n");
      q_data_t data;
      {
        if (buff_q.empty()) {
          std::unique_lock<std::mutex> lk(m);
          cv.wait(lk);
          lk.unlock();
        }
      }
      data = buff_q.front();
      DEBUG("will pop from buff_q\n");
      buff_q.pop();
      int fd;
      EXIT_IF((fd = openat(logfd, data.name, O_CREAT | O_APPEND | O_WRONLY,
                           RWRWRW)) < 0,
              "open logfile failed\n");
      write(fd, data.msg, data.size);
      close(fd);
    }
  });
  t.detach();

  while (true) {
    DEBUG("jlogd is waitting\n");
    auto n = recvfrom(get_fd(), buff, MAXLINE, 0, (struct sockaddr *)&cliaddr,
                      &clilen);
    DEBUG("jlogd recived a messege\n");
    if (buff_q.empty()) {
      DEBUG("buff_q is empty\n");
      buff_q.push(q_data_t((char *)inet_ntoa(cliaddr.sin_addr), buff));
      cv.notify_one();
    } else {
      DEBUG("buff_q is not empty\n");
      buff_q.push(q_data_t((char *)inet_ntoa(cliaddr.sin_addr), buff));
    }
  }
}
