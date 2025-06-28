#include "maelstrom.h"
#include <cstdlib>
#include <unordered_map>
#include <iostream>

std::unordered_map<std::string, maelstrom::Message::callback> maelstrom::Message::handler_map;
int maelstrom::Message::naive_message_id = 0;

maelstrom::Message::Message(const std::string_view message) 
{
  json data = json::parse(message.data());
  source      = data["src"];
  destination = data["dest"];
  payload     = data["body"];
}

void maelstrom::Message::message_dispatch(std::string_view raw) {
  static long naive_msg_id = 1;
  Message message(raw);
  if (auto found = handler_map.find(message.payload["type"]); found != handler_map.end()) {
    json response;
    json body = found->second(message);
    response["src"] = message.destination;
    response["dest"] = message.source;
    response["body"] = body;
    std::cout << response << std::endl;
    return;
  }
  std::abort();
}

void maelstrom::Message::add_handler(std::string type, callback fn) {
  if (auto found = handler_map.find(type); found != handler_map.end())
    std::abort();
  handler_map.emplace(std::move(type), fn);
}

void maelstrom::run_node() {
  using json = nlohmann::ordered_json;
  maelstrom::Message::add_handler("init", [&](Message& msg) {
      json response;
      response["type"] = "init_ok";
      response["msg_id"] = Message::naive_message_id++;
      response["in_reply_to"] = msg.payload["msg_id"];
      return response;
    }
  );

  while (true) {
    std::string buf;
    std::getline(std::cin, buf);
    if (buf.empty())
      break;
    Message::message_dispatch(buf);
  }
}
