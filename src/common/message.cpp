#include "message.h"
#include "common/snowflake.h"
#include <atomic>
#include <optional>
#include <iostream>


auto message_type_from_string(std::string_view raw) -> const MessageType
{
  // is case-sensitivity okay if not good in the case of RPC?
  const std::unordered_map<std::string_view, MessageType> conversion_map = {
    { "init",         INIT_REQ },
    { "init_ok",      INIT_RES },

    { "echo",         ECHO_REQ },
    { "echo_ok",      ECHO_RES },

    { "generate",     GENERATE_REQ },
    { "generate_ok",  GENERATE_RES },
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

    case GENERATE_REQ: return "generate"sv;
    case GENERATE_RES: return "generate_ok"sv;
  }
  // should probably be very pissy and throwing an error instead / aborting, garbage in system panic out
  // addendum: no? are you stupid? garbage in error message out
  return std::string_view();
}

auto Message::parse(std::string_view raw) -> std::optional<Message>
{
  json parsed = json::parse(raw, nullptr, false);
  if (parsed.is_discarded())
    return std::nullopt;
  return from_json(parsed);
}

auto Message::from_json(const json& json_msg) -> std::optional<Message> {
  bool required_fields_present = true;
  if (!json_msg.contains("src") || !json_msg["src"].is_string()) {
    std::clog << "[❌][MSG] message missing required field 'src'\n";
    required_fields_present = false;
  }
  if (!json_msg.contains("dest") || !json_msg["dest"].is_string()) {
    std::clog << "[❌][MSG] message missing required field 'dest'\n";
    required_fields_present = false;
  }
  if (!json_msg.contains("body") || !json_msg["body"].is_object()) {
    std::clog << "[❌][MSG] body of message is not of JSON type 'object'\n";
    required_fields_present = false;
  } else if (!json_msg.at("body").contains("type") || !json_msg.at("body").at("type").is_string()) {
    std::clog << "[❌][MSG] body of message does not contain 'type'\n";
    required_fields_present = false;
  }
  if (!required_fields_present) {
    std::clog << "[❌][MSG] not all required fields present\n";
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
  
  if (parsed_type == INVALID)
    return std::nullopt;

  Message msg(
    parsed_type,
    msg_id,
    reply_id,
    json_msg["src"].get<std::string_view>(),
    json_msg["dest"].get<std::string_view>()
  );
  msg.body = json_msg["body"];
  return msg;
}

Message::Message() 
  : type(INVALID)
  , id(-1)
  , reply_id(-1)
{}

Message::Message(MessageType type, Snowflake id, std::string_view from, std::string_view to)
  : Message(type, id, -1, from, to)
{}

Message::Message(MessageType type, Snowflake id, Snowflake reply_id, std::string_view from, std::string_view to)
  : type(type)
  , id(type == INVALID ? Snowflake(-1) : id)
  , reply_id(type == INVALID ? Snowflake(-1) : reply_id)
  , from(from)
  , to(to)
{
  if (type != INVALID)
    body["type"] = message_type_to_string(type);
  if (id != -1)
    body["msg_id"] = id;
  if (reply_id != -1)
    body["in_reply_to"] = reply_id;
}

auto Message::create_response() const -> Message
{
  MessageType response_type = INVALID;
  switch (type)
  {
    case INIT_REQ:      response_type = INIT_RES; break;
    case ECHO_REQ:      response_type = ECHO_RES; break;
    case GENERATE_REQ:  response_type = GENERATE_RES; break;

    default:
      std::cerr << "unimplemented response for message of type '" << message_type_to_string(type) << "'\n";
      return Message();
  }
  Message response(response_type, Snowflake(), id, to, from);
  response.body["in_reply_to"] = id;
  return response;
}

auto Message::as_json() const -> json
{
  json as_json = {
    { "src",  from },
    { "dest", to },
    { "body", body }
  };
  if (id.good())
    as_json["body"]["msg_id"] = id;
  if (reply_id.good())
    as_json["body"]["in_reply_to"] = reply_id;
  return as_json;
}
