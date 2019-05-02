#ifndef __LOGGER_IMPL_H_
#define __LOGGER_IMPL_H_

#include "buffer.h"
#include "jlog_inner_inc.h"
#include "registry.h"
#include "singleton.h"

#define LOG_DF(...)                                                         \
  {                                                                         \
    singleton<jlog::details::logger_impl>::get_instance()->log(             \
        {STDOUT_FILENO},                                                    \
        jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, __func__, \
                  __LINE__),                                                \
        ##__VA_ARGS__);                                                     \
  }
#define LOG(level, ...)                                                     \
  {                                                                         \
    singleton<jlog::details::logger_impl>::get_instance()->log(             \
        level, {STDOUT_FILENO},                                             \
        jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, __func__, \
                  __LINE__),                                                \
        ##__VA_ARGS__);                                                     \
  }

#define INFO(...)                                                           \
  {                                                                         \
    singleton<jlog::details::logger_impl>::get_instance()->info(            \
        {STDOUT_FILENO},                                                    \
        jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, __func__, \
                  __LINE__),                                                \
        ##__VA_ARGS__);                                                     \
  }
#define DEBUG(...)                                                          \
  {                                                                         \
    singleton<jlog::details::logger_impl>::get_instance()->debug(           \
        {STDOUT_FILENO},                                                    \
        jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, __func__, \
                  __LINE__),                                                \
        ##__VA_ARGS__);                                                     \
  }
#define ERROR(...)                                                          \
  {                                                                         \
    singleton<jlog::details::logger_impl>::get_instance()->error(           \
        {STDOUT_FILENO},                                                    \
        jlog::fmt("[%s][%s:%s:%d]", jlog::now().data(), __FILE__, __func__, \
                  __LINE__),                                                \
        ##__VA_ARGS__);                                                     \
  }

#define EXIT_IF(cond, ...) \
  {                        \
    if (cond) {            \
      ERROR(__VA_ARGS__);  \
    }                      \
  }

namespace jlog {

namespace details {
std::mutex m_buffer_lists_;
std::condition_variable cv_buffer_lists_;
std::unordered_map<int, std::shared_ptr<buffer_lists>> buffer_lists_;
std::unordered_map<int, std::shared_ptr<buffer_lists>> output_lists_;
std::unordered_map<int, std::mutex> m_current_;
std::unordered_map<int, std::shared_ptr<buffer_node>> current_;

struct logger_impl {
  /* 构造函数 */
  logger_impl(const logger_impl&) = delete;
  logger_impl& operator=(logger_impl&) = delete;

  /* 启用移动构造函数移动赋值函数， 使得可以用移动语义 */
  logger_impl() = default;
  logger_impl(logger_impl&& log);
  logger_impl& operator=(logger_impl&& log) {
    logger_impl(std::move(log));
    return *this;
  };

  /* 普通构造函数 */
  logger_impl(std::string&& name)
      : name_(name), fds_({1}), level_(log_level_t::debug) {}
  logger_impl(std::string&& name, std::unordered_set<int>&& fds)
      : name_(name), fds_(fds), level_(log_level_t::debug) {}
  logger_impl(std::string&& name, std::unordered_set<int>&& fds,
              log_level_t&& lv)
      : name_(name), fds_(fds), level_(lv) {}
  void destory();

  /*
   * 静态函数声明（定义时不需要 static 关键字）：
   * */

  /* 日志字符字符串 */
  static const std::string& strlevel(log_level_t lv);

  /* 生成智能指针 */
  static std::shared_ptr<logger_impl> new_logger_impl(
      std::string&& name = "", std::unordered_set<int> fds = {},
      log_level_t lv = log_level_t::debug) {
    return std::shared_ptr<logger_impl>(
        new logger_impl(std::move(name), std::move(fds), std::move(lv)),
        deleter<logger_impl>);
  };

  /*
   * 普通函数声明 */

  /* 将自己注册/取消到注册中心 */
  void add_to_regs();
  void unadd_to_regs();

  /* 输出自己的日志等级 */
  const std::string& strlevel();

  /* 等级操作 */
  const log_level_t& get_level() const;
  void set_level(log_level_t&& lv);

  /* 描述符操作 */
  const std::unordered_set<int>& get_fds() const;
  void set_fds(std::unordered_set<int>&& fds);

  /* 日志名字操作 */
  const std::string& get_name() const;
  void set_name(std::string&& name);

  /* 默认头部 */
  template <typename... Args>
  void log(log_level_t lv, const std::unordered_set<int>& fds, Args...);
  template <typename... Args>
  void log(log_level_t lv, const std::unordered_set<int>& fds,
           const std::string& msg, Args...);

  /* 默认日志等级和描述符 */
  template <typename... Args>
  void log(Args... args);
  /* 默认描述符 */
  template <typename... Args>
  void log(log_level_t lv, Args... args);
  /* 默认等级 */
  template <typename... Args>
  void log(const std::unordered_set<int>& fds, Args... args);

  /**
   * 直接给出日志等级, 并且重载对应的 描述符集合 版本
   */
  template <typename... Args>
  void info(Args... args);
  template <typename... Args>
  void info(const std::unordered_set<int>& fds, Args... args);
  template <typename... Args>
  void debug(Args... args);
  template <typename... Args>
  void debug(const std::unordered_set<int>& fds, Args... args);
  template <typename... Args>
  void error(Args... args);
  template <typename... Args>
  void error(const std::unordered_set<int>& fds, Args... args);
  static void thread_func();

 private:
  /* 只能在堆上生成对象， 便于将对象添加到注册中心 */
  ~logger_impl() = default;

  /* log 的辅助函数
   * fds: 自定义的输出描述符集合， 为空， 则使用默认的集合
   * str: 出错信息的前面的信息， 一般为时间， 行号等
   * ...: 格式化字符串
   * */
  template <typename... Args>
  void _log(const std::unordered_set<int>& fds, const std::string& str,
            Args... args);
  void _log(const std::unordered_set<int>& fds, const std::string& str);
  bool _lv_is_valid(log_level_t lv) { return lv <= level_; }

  /* 日志等级 */
  log_level_t level_ = log_level_t::debug;
  /* 日志名称 */
  std::string name_ = "";
  /* 要打印到的描述符集合 */
  std::unordered_set<int> fds_ = {};
  /* 日志等级到对应字符字符串的描述表 */
  static const std::unordered_map<log_level_t, std::string> level_m;
  static void _write(int fd, const char* msg, size_t len);
  static bool running_;
  static std::mutex lr_;
};

bool logger_impl::running_ = false;
std::mutex logger_impl::lr_;

const std::unordered_map<log_level_t, std::string> logger_impl::level_m{
    {log_level_t::debug, " debug "},  {log_level_t::info, " info  "},
    {log_level_t::notice, "notice "}, {log_level_t::warning, "warning"},
    {log_level_t::error, " error "},  {log_level_t::crit, " crit  "},
    {log_level_t::alert, " alert "},  {log_level_t::emerg, " emerg "}};

logger_impl::logger_impl(logger_impl&& log) {
  name_ = log.get_name();
  fds_ = log.get_fds();
  level_ = log.get_level();
  log.~logger_impl();
}

void logger_impl::destory() {
  sleep(2); /* 等待后台线程完成 */
  for (auto fd : fds_) {
    close(fd);
  }
}

/* 日志字符字符串 */
const std::string& logger_impl::strlevel(log_level_t lv) {
  return level_m.at(lv);
}

void logger_impl::add_to_regs() {
  std::shared_ptr<logger_impl> t(
      new logger_impl(std::move(name_), std::move(fds_), std::move(level_)),
      deleter<logger_impl>);
  singleton<registry<logger_impl>>::get_instance()->add(std::move(*t));
}
void logger_impl::unadd_to_regs() {
  singleton<registry<logger_impl>>::get_instance()->delelte(name_);
}

const std::string& logger_impl::strlevel() { return level_m.at(level_); }

/* 等级操作 */
const log_level_t& logger_impl::get_level() const { return level_; }
void logger_impl::set_level(log_level_t&& lv) { level_ = lv; }
/* 描述符操作 */
const std::unordered_set<int>& logger_impl::get_fds() const { return fds_; }
void logger_impl::set_fds(std::unordered_set<int>&& fds) { fds_ = fds; };

/* 日志名字操作 */
const std::string& logger_impl::get_name() const { return name_; }
void logger_impl::set_name(std::string&& name) { name_ = name; }

/* 默认日志等级和描述符 */
template <typename... Args>
void logger_impl::log(Args... args) {
  log(level_, fds_, std::forward<Args>(args)...);
}
/* 默认描述符 */
template <typename... Args>
void logger_impl::log(log_level_t lv, Args... args) {
  log(lv, fds_, std::forward<Args>(args)...);
}
/* 默认等级 */
template <typename... Args>
void logger_impl::log(const std::unordered_set<int>& fds, Args... args) {
  log(level_, fds, std::forward<Args>(args)...);
}

/**
 * 直接给出日志等级, 并且重载对应的 描述符集合 版本
 */
template <typename... Args>
void logger_impl::info(Args... args) {
  log(log_level_t::info, std::forward<Args>(args)...);
}
template <typename... Args>
void logger_impl::info(const std::unordered_set<int>& fds, Args... args) {
  log(log_level_t::info, fds, std::forward<Args>(args)...);
}
template <typename... Args>
void logger_impl::debug(Args... args) {
  log(log_level_t::debug, std::forward<Args>(args)...);
}
template <typename... Args>
void logger_impl::debug(const std::unordered_set<int>& fds, Args... args) {
  log(log_level_t::debug, fds, std::forward<Args>(args)...);
}
template <typename... Args>
void logger_impl::error(Args... args) {
  log(log_level_t::error, std::forward<Args>(args)...);
}
template <typename... Args>
void logger_impl::error(const std::unordered_set<int>& fds, Args... args) {
  log(log_level_t::error, fds, std::forward<Args>(args)...);
}

template <typename... Args>
void logger_impl::_log(const std::unordered_set<int>& fds,
                       const std::string& str, Args... args) {
  std::string msg = str + " -> " += fmt(std::forward<Args>(args)...);
  if (msg.back() != '\n') msg += "\n";
  for (auto fd : fds_) {
    if (fd == 1 || fd == 2) { /* 控制台消息直接输出 */
      write(fd, msg.c_str(), msg.size());
    } else { /* 文件中日志采用异步方式 */
      _write(fd, msg.c_str(), msg.size());
    }
  }
}

void logger_impl::_log(const std::unordered_set<int>& fds,
                       const std::string& str) {
  std::string msg = str + " -> Nothing !!!\n";
  for (auto fd : fds_) {
    _write(fd, msg.c_str(), msg.size());
  }
}

template <typename... Args>
void logger_impl::log(log_level_t lv, const std::unordered_set<int>& fds,
                      const std::string& msg, Args... args) {
  if (_lv_is_valid(lv)) {
    _log(fds, msg + "[" + strlevel(lv) + "]", std::forward<Args>(args)...);
  }
  if (lv < log_level_t::warning) {
    exit(-1);
  }
}

template <typename... Args>
void logger_impl::log(log_level_t lv, const std::unordered_set<int>& fds,
                      Args... args) {
  log(lv, fds, "[" + now() + "]", std::forward<Args>(args)...);
}

void logger_impl::thread_func() {
  while (true) {
    bool recved = false;
    {
      std::unique_lock<std::mutex> lk(m_buffer_lists_);
      if (buffer_lists_.empty()) {
        cv_buffer_lists_.wait_for(lk, std::chrono::seconds(INTERVAL));
      }
      if (!buffer_lists_.empty()) {
        buffer_lists_.swap(output_lists_);
        recved = true;
      }
    }
    if (!recved && !m_current_.empty()) {
      for (auto& c : current_) {
        std::unique_lock<std::mutex> lk(m_current_[c.first]);
        if (c.second->buf.len() == 0) continue;
        if (output_lists_[c.first] == nullptr) {
          output_lists_[c.first] = std::make_shared<buffer_lists>();
        }
        output_lists_[c.first]->push_back(c.second);
        c.second.reset(new buffer_node);
      }
      if (!output_lists_.empty()) recved = true;
    }
    if (recved) {
      for (auto o : output_lists_) {
        auto node = o.second->begin();
        while (node != node->next && node != o.second->end()) {
          write(o.first, node->buf.data(), node->buf.len());
          node = node->next;
        }
      }
      output_lists_.clear();
    }
  }
}

void logger_impl::_write(int fd, const char* msg, size_t len) {
  {
    if (!running_) {
      std::unique_lock<std::mutex> lk(lr_);
      if (!running_) {
        static std::thread thread_(thread_func);
        thread_.detach();
        running_ = true;
      }
    }
    bool over = false;
    {
      std::unique_lock<std::mutex> lk(m_current_[fd]);
      if (current_[fd] == nullptr) {
        current_[fd] = std::make_shared<jlog::buffer_node>();
      }
      if (current_[fd]->buf.len() + len < current_[fd]->buf.caplity()) {
        current_[fd]->buf.append(msg, len);
      } else {
        over = true; /* 前端日志 buffer 写满 */
      }
    }
    if (over) {
      std::shared_ptr<jlog::buffer_lists> buffer;
      {
        {
          /* 从 buffers 中取出对应的 buffer */
          std::unique_lock<std::mutex> lk(m_buffer_lists_);
          buffer = buffer_lists_[fd];
          buffer_lists_[fd] = nullptr;
        }
        if (buffer == nullptr) {
          buffer = std::make_shared<jlog::buffer_lists>();
        }
        std::shared_ptr<jlog::buffer_node> cur_tmp;
        {
          std::unique_lock<std::mutex> lk(m_current_[fd]);
          cur_tmp = current_[fd]; /* 延后 push */
          current_[fd].reset(new jlog::buffer_node);
          current_[fd]->buf.append(msg, len); /* 将超过部份写入新的 buffer */
        }
        buffer->push_back(cur_tmp);
      }
      {
        /* 将 buffer 插入到 buffers 中 */
        std::unique_lock<std::mutex> lk(m_buffer_lists_);
        buffer_lists_[fd] = buffer;
        cv_buffer_lists_.notify_one(); /* 写满一个 buffer */
      }
    }
  }
}

}  // namespace details
}  // namespace jlog

#endif  // __LOGGER_IMPL_H_
