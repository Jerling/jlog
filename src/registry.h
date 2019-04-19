#ifndef __REGISTRY_H_
#define __REGISTRY_H_

#include "common.h"

template <typename T>
struct registry {
  registry<T>() = default;
  ~registry<T>() = default;
  registry(const registry &) = delete;
  registry &operator=(const registry &) = delete;
  void destory() { delete this; }
  std::shared_ptr<T> get(const std::string &name) {
    if (objs.find(name) == objs.end()) return nullptr;
    return objs[name];
  };
  void delelte(const std::string &name) {
    if (objs.find(name) == objs.end()) return;
    objs.erase(name);
  };
  int count() { return objs.size(); };
  int add(T &&t) {
    if (t.get_name() == "") {
      return -1;
    }
    if (objs.find(t.get_name()) != objs.end()) {
      return -2;
    }
    std::shared_ptr<T> obj(new T(std::move(t)), deleter<T>);
    objs[obj->get_name()] = obj;
  };
  int add(std::initializer_list<std::shared_ptr<T>> list) {
    for (auto t : list) {
      add(std::move(std::move(*t)));
    }
  }
  void print_objs() const {
    std::cout << "exsits loggers as follow:" << std::endl;
    for (auto o : objs) {
      std::cout << "name: " << o.first << std::endl;
    }
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<T>> objs;
};

#endif  // __REGISTRY_H_
