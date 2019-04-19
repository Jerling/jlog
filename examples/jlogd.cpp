#include "jlog.h"

int main(int argc, char *argv[]) {
  unsigned short port = 0;
  char *hname = nullptr;
  arg_parse(argc, argv, hname, port);
  if (port == 0) {
    port = 5000; /* 默认端口 */
  }

  /* 创建日志服务器 */
  udp_server us(hname, port);
  DEBUG("constuct udp_server finisth\n");
  us.start();

  return 0;
}
