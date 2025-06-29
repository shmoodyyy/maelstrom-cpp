#ifndef COMMON_MESSAGE_HEADER
#define COMMON_MESSAGE_HEADER
#include "../ext/nlohmann/json.hpp"
#include <atomic>
#include <string_view>

enum MessageType
{
  INVALID,

  INIT_REQ,
  INIT_RES,

  ECHO_REQ,
  ECHO_RES,
};
auto message_type_from_string(std::string_view raw) -> const MessageType;
auto message_type_to_string(MessageType type) -> const std::string_view;

class Message {
  using json = nlohmann::json;
public:
  static auto parse(std::string_view raw) -> std::optional<Message>;
  static auto from_json(const json& json_msg) -> std::optional<Message>;
  static auto next_id() -> const int;
private:
  static std::atomic_int naive_next_id;

public:
  Message();
  Message(MessageType type, int id, std::string_view from, std::string_view to);
  Message(MessageType type, int id, int reply_id, std::string_view from, std::string_view to);

  auto create_response() const -> Message;
  auto as_json() const -> json;
private:

public:
  const MessageType type;
  const int id;
  const int reply_id;
  const std::string from;
  const std::string to;
  json body;
private:
};

#endif
