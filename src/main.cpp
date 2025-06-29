#include "common/message.h"
#include "common/node.h"
#include <iostream>

int main() {
  Node node;

  node.register_handler(INIT_REQ, [&](const Message& msg) -> Message {
    // TODO: add in returning error responses
    
    if (!msg.body.contains("node_id") || !msg.body["node_id"].is_string()
      || !msg.body.contains("node_ids") || !msg.body["node_ids"].is_array()) {
      std::cerr << "bleghh\n";
    }
    node.init(msg)

    return msg.create_response();
  });

  node.register_handler(ECHO_REQ, [](const Message& msg) -> Message {
    Message response = msg.create_response();
    response.body["echo"] = msg.body["echo"];
    return msg.create_response();
  });

  node.run();
}
