#include "jlog.h"

int main() {
  auto log = jlog::logger::new_logger(
      "remote", {udp_client("127.0.0.1", 5000).get_cfd()});
  log->info("test from main test\n");
  return 0;
}
