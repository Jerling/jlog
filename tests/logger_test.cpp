#include "../jlog.h"
int main() {
  auto fd = open("/home/jer/test.txt", O_CREAT | O_RDWR, RWRWRW);
  LOG(jlog::log_level_t::debug, "LOG test %d\n", 10);
  auto log = jlog::logger::new_logger("stdout", {fd});
  for (auto i : log->get_fds()) {
    std::cout << i << std::endl;
  }
  log->log(jlog::log_level_t::info, std::unordered_set<int>({fd}), "%s:%d\n",
           __FILE__, __LINE__);
  log->log("%s:%d\n", __FILE__, __LINE__);
  return 0;
}
