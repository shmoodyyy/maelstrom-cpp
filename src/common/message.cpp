#include "message.h"
#include <atomic>
#include <optional>
#include <iostream>


auto message_type_from_string(std::string_view raw) -> const MessageType
{
  // is case-sensitivity okay if not good in the case of RPC?
  const std::unordered_map<std::string_view, MessageType> conversion_map = {
    { "init",     INIT_REQ },
    { "init_ok",  INIT_RES },

    { "echo",     ECHO_REQ },
    { "echo_ok",  ECHO_RES },
  };
  auto found = conversion_map.find(raw);
  return found == conversion_map.end() ? INVALID : found->second;
}

auto message_type_to_string(MessageType type) -> const std::string_view
{
  // this is actually awful, heavily hardcoded
  // adding one type forces me to touch 3 different spots at a minimum of my capacity to remember rn
  using namespace std::string_view_literals;
  switch (type) {
    case INVALID: break;

    case INIT_REQ: return "init"sv;
    case INIT_RES: return "init_ok"sv;

    case ECHO_REQ: return "echo"sv;
    case ECHO_RES: return "echo_ok"sv;
  }
  // should probably be very pissy and throwing an error instead / aborting, garbage in system panic out
  return std::string_view();
}

auto Message::parse(std::string_view raw) -> std::optional<Message>
{
  return from_json(raw);
}

auto Message::from_json(const json& json_msg) -> std::optional<Message> {
  if (!json_msg.contains("src") || !json_msg["src"].is_string()
    || !json_msg.contains("dest") || !json_msg["dest"].is_string()
    || !json_msg.contains("body") || !json_msg["body"].is_object()
    || !json_msg.at("body").contains("type") || !json_msg.at("body").at("type").is_string())  {
    return std::nullopt;
  }

  MessageType parsed_type = message_type_from_string(json_msg["body"]["type"].get<std::string_view>());
  const int msg_id =
    json_msg["body"].contains("msg_id") 
      ? json_msg["body"]["msg_id"].get<int>()
      : -1;
  const int reply_id =
    json_msg["body"].contains("is_reply_to")
      ? json_msg["body"]["is_reply_to"].get<int>()
      : -1;

  return 
    parsed_type == INVALID
      ? std::nullopt
      : std::make_optional(
          Message(
            parsed_type,
            msg_id,
            reply_id,
            json_msg["src"].get<std::string_view>(),
            json_msg["dest"].get<std::string_view>()));
}

Message::Message() 
  : type(INVALID)
  , id(-1)
  , reply_id(-1)
{}

Message::Message(MessageType type, int id, std::string_view from, std::string_view to)
  : Message(type, id, -1, from, to)
{}

Message::Message(MessageType type, int id, int reply_id, std::string_view from, std::string_view to)
  : type(type)
  , id(type == INVALID ? -1 : next_id())
  , reply_id(type == INVALID ? -1 : reply_id)
  , from(from)
  , to(to)
{}

auto Message::create_response() const -> Message
{
  MessageType response_type = INVALID;
  switch (type)
  {
    case INIT_REQ: response_type = INIT_RES; break;
    case ECHO_REQ: response_type = ECHO_RES; break;

    default:
      std::cerr << "unimplemented response for message of type '" << message_type_to_string(type) << "'\n";
      return Message();
  }
  return Message(response_type, id, to, from);
}

auto Message::as_json() const -> json
{
  json as_json = {
    { "src",  from },
    { "dest", to },
    { "body", body }
  };
  if (id != -1)
    as_json["body"]["msg_id"] = id;
  if (reply_id != -1)
    as_json["body"]["in_reply_to"] = reply_id;

  return as_json;
}

auto Message::next_id() -> const int
{
  return naive_next_id.fetch_add(1, std::memory_order_seq_cst);
}

std::atomic_int Message::naive_next_id = 1;
