#ifndef COMMON_MESSAGE_HEADER
#define COMMON_MESSAGE_HEADER
#include "../ext/nlohmann/json.hpp"
#include "snowflake.h"
#include <atomic>
#include <string_view>

enum MessageType
{
  INVALID,

  INIT_REQ,
  INIT_RES,

  ECHO_REQ,
  ECHO_RES,

  GENERATE_REQ,
  GENERATE_RES,
};
auto message_type_from_string(std::string_view raw) -> const MessageType;
auto message_type_to_string(MessageType type) -> const std::string_view;

class Message {
  using json = nlohmann::json;
public:
  static auto parse(std::string_view raw) -> std::optional<Message>;
  static auto from_json(const json& json_msg) -> std::optional<Message>;
private:

public:
  Message();
  Message(MessageType type, Snowflake id, std::string_view from, std::string_view to);
  Message(MessageType type, Snowflake id, Snowflake reply_id, std::string_view from, std::string_view to);

  auto create_response() const -> Message;
  auto as_json() const -> json;
private:

public:
  const MessageType type;
  const Snowflake id;
  const Snowflake reply_id;
  const std::string from;
  const std::string to;
  json body;
private:
};

#endif
