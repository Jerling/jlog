#include "../jlog.h"
int main() {
  LOG(jlog::log_level_t::debug, "LOG test %d\n", 10);
  INFO();
  auto log = jlog::logger::new_logger("stdout", {1});
  log->log(jlog::log_level_t::info, "%s:%d\n", __FILE__, __LINE__);
  log->log("%s:%d\n", __FILE__, __LINE__);
  LOG_DTL(log, jlog::log_level_t::info, "test\n");
  return 0;
}
