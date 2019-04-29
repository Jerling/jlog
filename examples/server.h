#ifndef __SERVER_H_
#define __SERVER_H_

#include "jlog.h"

/* 新建文件权限屏蔽子 */
#define RWRWRW (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

/* 一条日志的缓冲大小 */
const int MAXLINE = 1024;
/* 多长时间刷新一次 buffers */
const int INTERVAL = 1;
/* 日志目录 */
const char log_dir[] = "/var/jlog";

/* 包数计数 */
int count = 0;
/* 刷新计数 */
int flush = 0;

/* buffers 锁 */
std::mutex m;
/* buffers 条件变量 */
std::condition_variable cv;
/* 当前的前端日志 buffer */
std::unordered_map<std::string, std::mutex> mcur;
/* 前端日志的容器锁 */
std::unordered_map<std::string, std::mutex> mbufs;
/* 前端日志容器 */
std::unordered_map<std::string, std::shared_ptr<jlog::buffer_lists>> buffers;
/* 后端日志容器 */
std::unordered_map<std::string, std::shared_ptr<jlog::buffer_lists>> outputs;
/* 前端日志 buffer */
std::unordered_map<std::string, std::shared_ptr<jlog::buffer_node>> curent_;

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

/* udp 服务器 */
class udp_server {
 public:
  udp_server(const char *hname, unsigned short port) {
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
    // EXIT_IF(daemon(0, 0) == -1, "Daemon error");
    sfd_ = socket(AF_INET, SOCK_DGRAM, 0);
    EXIT_IF((sfd_ < 0), "udp_server socket failed\n");
    DEBUG("The server socket fd is %d\n", sfd_);
    int re;
    EXIT_IF((re = bind(sfd_, (struct sockaddr *)&local, sizeof(local)) != 0),
            "udp_server bind failed: %d: %s\n", re, strerror(errno));
  }

  void start() {
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
      DEBUG("Starting a thread...");
      for (;;) {
        bool recved = false;
        auto start = std::chrono::system_clock::now();
        {
          {
            std::unique_lock<std::mutex> lk(m);
            if (buffers.empty()) {
              DEBUG("wait for %d seconds", INTERVAL);
              /* 等待前端日志，间隔默认为 1s */
              cv.wait_until(lk, std::chrono::system_clock::now() +
                                    std::chrono::seconds(INTERVAL));
            }
            if (!buffers.empty()) {
              recved = true;
              DEBUG("buffers is not empty");
              buffers.swap(outputs);
            }
          }
          /* buffer 中没有日志,但 curent_ 中有 */
          if (!recved && !curent_.empty()) {
            for (auto &c : curent_) {
              /* 加锁防止前端正在写日志 */
              std::unique_lock<std::mutex> lk(mcur[c.first]);
              if (c.second->buf.len() == 0) {
                continue;
              }
              if (outputs[c.first] == nullptr) {
                outputs[c.first] = std::make_shared<jlog::buffer_lists>();
              }
              recved = true;
              DEBUG("curent_ is not empty");
              outputs[c.first]->push_back(
                  c.second); /* 将当前的 buffer 添加到 outputs */
              c.second.reset(new jlog::buffer_node); /* 新开一个 buffer */
            }
            if (!recved) curent_.clear();
          }
        }
        if (recved) {
          for (auto o : outputs) {
            if (o.second == nullptr || o.second->empty()) continue;
            int fd = openat(logfd, o.first.data(),
                            O_CREAT | O_APPEND | O_WRONLY, RWRWRW);
            auto node = o.second->begin();
            while (node != node->next && node != o.second->end()) {
              DEBUG("%d flush", flush++);
              write(fd, node->buf.data(), node->buf.len());
              node = node->next;
            }
            close(fd);
          }
          outputs.clear();
        }
      }
    });
    t.detach();

    while (true) {
      DEBUG("jlogd is waitting\n");
      auto n = recvfrom(sfd_, buff, MAXLINE, 0, (struct sockaddr *)&cliaddr,
                        &clilen);
      char *caddr = (char *)inet_ntoa(cliaddr.sin_addr);
      DEBUG("jlogd recived %d messege\n", count++);
      bool over = false;
      {
        std::unique_lock<std::mutex> lk(mcur[caddr]);
        if (curent_[caddr] == nullptr) {
          curent_[caddr] = std::make_shared<jlog::buffer_node>();
        }
        if (curent_[caddr]->buf.len() + n < curent_[caddr]->buf.caplity()) {
          curent_[caddr]->buf.append(buff, n);
        } else {
          over = true; /* 前端日志 buffer 写满 */
        }
      }
      if (over) {
        std::shared_ptr<jlog::buffer_lists> buffer;
        {
          std::unique_lock<std::mutex> lk(mbufs[caddr]);
          buffer = buffers[caddr];
          if (buffer == nullptr) {
            buffer = std::make_shared<jlog::buffer_lists>();
            buffers[caddr] = buffer;
          }
          std::shared_ptr<jlog::buffer_node> cur_tmp;
          {
            std::unique_lock<std::mutex> lk(mcur[caddr]);
            cur_tmp = curent_[caddr]; /* 延后 push */
            curent_[caddr].reset(new jlog::buffer_node);
            curent_[caddr]->buf.append(buff, n); /* 将超过部份写入新的 buffer */
          }
          buffer->push_back(cur_tmp);
        }
        cv.notify_one(); /* 写满一个 buffer */
      }
    }
  }

  ~udp_server() { close(sfd_); }

 private:
  int sfd_;
};

#endif  // __SERVER_H_
