#ifndef BUFFER_H
#define BUFFER_H

#include "jlog_inner_inc.h"
const size_t JLOG_BUFFER_SZ = 1024 * 8;

namespace jlog {
namespace details {
class buffer {
 public:
  buffer() : len_(0), caplity_(JLOG_BUFFER_SZ) {
    data_ = (char *)malloc(caplity_);
  }
  buffer(size_t caplity) : len_(0), caplity_(caplity) {
    data_ = (char *)malloc(caplity_);
  }
  buffer(const buffer &bf) {
    if (data_ == this->data_) return;
    data_ = bf.data_;
    len_ = bf.len_;
    caplity_ = bf.caplity_;
  }
  buffer(buffer &&bf) {
    if (data_ == this->data_) return;
    free(data_);
    data_ = bf.data_;
    len_ = bf.len_;
    caplity_ = bf.caplity_;
  }
  int append(const char *str, size_t len) {
    if (len + len_ > caplity_) {
      return -1;
    }
    strncpy(data_ + len_, str, len);
    len_ += len;
    return len_;
  }
  void bzero() {
    memset(data_, 0, caplity_);
    len_ = 0;
  }
  char *data() const { return data_; }
  size_t len() const { return len_; }
  size_t caplity() const { return caplity_; }
  ~buffer() { free(data_); }

 private:
  size_t len_;
  size_t caplity_;
  char *data_;
};

}  // namespace details

struct buffer_node {
  using buffer = details::buffer;
  buffer_node() : buf(), next(nullptr), prev(nullptr) {}
  buffer_node(size_t cap) : buf(cap), next(nullptr), prev(nullptr) {}
  buffer buf;
  std::shared_ptr<buffer_node> next, prev;
};

class buffer_lists {
 public:
  buffer_lists() : size_(0) {
    head_ = std::make_shared<buffer_node>();
    head_->prev = head_;
    head_->next = head_;
    cur_ = head_->next;
  }
  size_t size() const { return size_; }
  void push_back(std::shared_ptr<buffer_node> new_node) {
    new_node->next = head_->next;
    head_->next->prev = new_node;
    new_node->prev = head_;
    head_->next = new_node;
  };
  bool empty() { return cur_ == head_->next; }
  std::shared_ptr<buffer_node> begin() { return head_->next; }
  std::shared_ptr<buffer_node> end() { return head_; }
  void reset() { cur_ = head_->next; }

 private:
  size_t size_;
  std::shared_ptr<buffer_node> head_;
  std::shared_ptr<buffer_node> cur_;
};
}  // namespace jlog

#endif  //  BUFFER_H
