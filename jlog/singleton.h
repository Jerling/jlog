#ifndef __SINGLETON_H_
#define __SINGLETON_H_

#include "jlog_inner_inc.h"

template <typename T>
struct singleton {
 private:
  singleton() = delete;
  singleton(const singleton<T>&) = delete;
  singleton<T> operator=(const singleton<T>&) = delete;
  static std::shared_ptr<T> instance;

 public:
  static std::shared_ptr<T> get_instance() { return instance; };
};

template <typename T>
std::shared_ptr<T> singleton<T>::instance(new T, deleter<T>);
#endif  // __SINGLETON_H_
