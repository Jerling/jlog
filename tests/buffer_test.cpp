#include "../jlog.h"

int main(int argc, char *argv[]) {
  jlog::buffer_node n;
  n.buf.append("test", 5);
  DEBUG("%s", n.buf.data());

  std::shared_ptr<jlog::buffer_node> pn(new jlog::buffer_node());
  pn->buf.append("test", 5);
  DEBUG("%s", pn->buf.data());

  std::unordered_map<std::string, std::shared_ptr<jlog::buffer_node>> mb;
  mb["test"] = std::make_shared<jlog::buffer_node>();
  mb["test"]->buf.append("mb test", 8);
  DEBUG("%s", mb["test"]->buf.data());
  return 0;
}
