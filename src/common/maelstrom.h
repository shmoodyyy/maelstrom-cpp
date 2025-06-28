#ifndef COMMON_MAELSTROM_HEADER
#define COMMON_MAELSTROM_HEADER
#include "../ext/nlohmann/json.hpp"
#include <functional>
#include <string_view>
#include <unordered_map>
#include <iostream>

namespace maelstrom {

class Message {
  using json = nlohmann::json;
public:
  Message(const std::string_view message);

  static int naive_message_id;

  std::string source;
  std::string destination;
  json payload;

  using callback = std::function<json(Message&)>;
  static void message_dispatch(std::string_view raw);
  static void add_handler(std::string type, callback fn);
private:
  static std::unordered_map<std::string, callback> handler_map;
};

void run_node();

}

#endif
