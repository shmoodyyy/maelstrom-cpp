#include "common/message.h"
#include "common/node.h"
#include "common/snowflake.h"

int main(int argc, const char** argv) {
  Node node(16);

  node.register_handler(ECHO_REQ, [](const Message& msg) -> Message {
    Message response = msg.create_response();
    response.body["echo"] = msg.body["echo"];
    return response;
  });

  node.register_handler(GENERATE_REQ, [](const Message& msg) -> Message {
    Message response = msg.create_response();
    response.body["id"] = Snowflake();
    return response;
  });

  node.run();
}
