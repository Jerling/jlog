#ifndef __LOGGER_H_
#define __LOGGER_H_

#include "logger_impl.h"

#define LOG_DTL_DF(log, ...)                                           \
  do {                                                                 \
    log->log(jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, \
                       __func__, __LINE__),                            \
             ##__VA_ARGS__);                                           \
  } while (0)
#define LOG_DTL(log, level, ...)                                       \
  do {                                                                 \
    log->log(level,                                                    \
             jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, \
                       __func__, __LINE__),                            \
             ##__VA_ARGS__);                                           \
  } while (0)
#define INFO_DTL(log, ...)                                              \
  do {                                                                  \
    log->info(jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, \
                        __func__, __LINE__),                            \
              ##__VA_ARGS__);                                           \
  } while (0)
#define DEBUG_DTL(log, ...)                                              \
  do {                                                                   \
    log->debug(jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, \
                         __func__, __LINE__),                            \
               ##__VA_ARGS__);                                           \
  } while (0)
#define ERROR_DTL(log, ...)                                              \
  do {                                                                   \
    log->error(jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, \
                         __func__, __LINE__),                            \
               ##__VA_ARGS__);                                           \
  } while (0)

namespace jlog {
struct logger {
  logger() : impl(details::logger_impl::new_logger_impl()) {}
  logger(std::shared_ptr<details::logger_impl> log_i) { impl = log_i; }
  void destory() { delete this; }
  /* 生成智能指针 */
  static std::shared_ptr<logger> new_logger(
      std::string&& name = "", std::unordered_set<int> fds = {},
      log_level_t lv = log_level_t::debug) {
    return std::shared_ptr<logger>(
        new logger(details::logger_impl::new_logger_impl(
            std::forward<std::string>(name),
            std::forward<std::unordered_set<int>>(fds),
            std::forward<log_level_t>(lv))));
  };

  template <typename... Args>
  void log(Args... args);
  template <typename... Args>
  void debug(Args... args);
  template <typename... Args>
  void info(Args... args);
  template <typename... Args>
  void error(Args... args);
  const std::string get_name() const { return impl->get_name(); }
  void set_name(std::string&& name) {
    impl->set_name(std::forward<std::string>(name));
  }
  const std::unordered_set<int> get_fds() const { return impl->get_fds(); }
  void set_fds(std::unordered_set<int>&& fds) {
    impl->set_fds(std::forward<std::unordered_set<int>>(fds));
  }
  const log_level_t get_level() const { return impl->get_level(); }
  void set_level(log_level_t&& lv) {
    impl->set_level(std::forward<log_level_t>(lv));
  }
  void set_attr(std::string&& name = "", std::unordered_set<int>&& fds = {},
                log_level_t lv = log_level_t::nops) {
    if (!name.empty()) {
      set_name(std::forward<std::string>(name));
    }
    if (!fds.empty()) {
      set_fds(std::forward<std::unordered_set<int>>(fds));
    }
    if (lv < log_level_t::nops) {
      set_level(std::forward<log_level_t>(lv));
    }
  }

  void add_to_regs();
  void unadd_to_regs();

 private:
  std::shared_ptr<details::logger_impl> impl;
};

template <typename... Args>
void logger::log(Args... args) {
  impl->log(std::forward<Args>(args)...);
}

template <typename... Args>
void logger::debug(Args... args) {
  impl->debug(std::forward<Args>(args)...);
}

template <typename... Args>
void logger::info(Args... args) {
  impl->info(std::forward<Args>(args)...);
}
template <typename... Args>
void logger::error(Args... args) {
  impl->error(std::forward<Args>(args)...);
}

void logger::add_to_regs() {
  auto tmp = logger(impl);
  singleton<registry<logger>>::get_instance()->add(std::move(tmp));
}

void logger::unadd_to_regs() {
  singleton<registry<logger>>::get_instance()->delelte(impl->get_name());
}

}  // namespace jlog

#endif  // __LOGGER_H_
