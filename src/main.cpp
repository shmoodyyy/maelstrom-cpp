#include "common/maelstrom.h"
#include <string>

int main() {
  using nlohmann::json;
  maelstrom::Message::add_handler("echo", [](maelstrom::Message& msg) {
      json response;
      response["type"] = "echo_ok";
      response["msg_id"] = msg.naive_message_id++;
      response["in_reply_to"] = msg.payload["msg_id"];
      response["echo"] = msg.payload["echo"];
      return response;
    }
  );
  maelstrom::run_node();
}
