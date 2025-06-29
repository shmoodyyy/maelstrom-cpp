#include "common/message.h"
#include "common/node.h"
#include <iostream>
#include <fstream>

int main(int argc, const char** argv) {
  Node node(16);
  node.register_handler(INIT_REQ, [&](const Message& msg) -> Message {
    // TODO: add in returning error responses
    if (!msg.body.contains("node_id") || !msg.body["node_id"].is_string()
      || !msg.body.contains("node_ids") || !msg.body["node_ids"].is_array()) {
      std::cerr << "bleghh\n" << msg.body << '\n';
    }
    std::string_view self_id = msg.body["node_id"].get<std::string_view>();
    std::vector<std::string> node_ids = msg.body["node_ids"];
    std::size_t idx = 0;
    for (std::string_view id : node_ids) {
      if (self_id.compare(id) == 0)
        break;
      ++idx;
    }
    if (idx == node_ids.size())
      return Message();

    // TODO: get boolean return and respond accordingly
    node.init(std::move(node_ids), idx);
    return msg.create_response();
  });

  node.register_handler(ECHO_REQ, [](const Message& msg) -> Message {
    Message response = msg.create_response();
    response.body["echo"] = msg.body["echo"];
    return response;
  });

  node.run();
}
