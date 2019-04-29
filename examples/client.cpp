#include "client.h"
#include "jlog.h"

int main(int argc, char *argv[]) {
  auto log = jlog::logger::new_logger(
      "remote", {udp_client("127.0.0.1", 5000).get_cfd()});
  for (int i = 0; i < strtol(argv[1], nullptr, 10); i++)
    log->info("test from main test %d\n", i);
  return 0;
}
