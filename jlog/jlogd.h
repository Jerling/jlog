#ifndef __JLOGD_H_
#define __JLOGD_H_

#include "logger.h"

/* 新建文件权限屏蔽子 */
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/* 一条日志的缓冲大小 */
const int MAXLINE = 1024;
/* 日志目录 */
const char log_dir[] = "/var/jlog";

/* 地址（用于日志文件命名）和消息 */
struct q_data_t {
  q_data_t() = default;
  q_data_t(const char *n, char *m) {
    strcpy(name, n);
    char *p = m;
    size = 0;
    while (*p != '\0' && *p++ != '\n') size++;
    strncpy(msg, m, size + 1);
    if (msg[size] != '\n') msg[size++] = '\n';
    msg[++size] = '\0';
  }
  /* 保存 ip  */
  char name[16] = "";
  char msg[MAXLINE] = "";
  uint16_t size = 0;
};

/* buff_q 暂时未考虑是否会竞争问题，原因如下：
 * 对于读： 只有一个线程读，不管读到空或不空，都可以继续
 * 对于写： 因为只有一个写线程，为处理的消息只会在 udp 缓冲区等待
 * 因此不管是读或写都可以保证只有一个线程在处理，如果同时进行，那那也只是两端
 * 的操作，因此应该不存在竞争问题 */
static std::queue<q_data_t, std::list<q_data_t>> buff_q;
/* 当队列为空的时候转入睡眠 */
std::mutex m;
std::condition_variable cv;

/* 日志目录 fd */
static int logfd;

void help() { std::cout << "Usage:\n-h[ip addr]\n-p[port]" << std::endl; }

/* 参数解析： ip or port */
void arg_parse(int argc, char *argv[], char *&name, unsigned short &port) {
  const char *opts = "h:p:";
  int opt;
  while ((opt = getopt(argc, argv, opts)) != -1) {
    switch (opt) {
      case 'p':
        if (optarg != nullptr) port = strtol(argv[optind - 1], nullptr, 10);
        break;
      case 'h':
        name = optarg;
        break;
      default:
        help();
        exit(-1);
    }
  }
}

/* 处理公共队列的消息 */
void handle_queue() {
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
};

#endif  // __JLOGD_H_
