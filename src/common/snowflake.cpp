#include "snowflake.h"
#include "common/encoding/base64.h"
#include "ext/nlohmann/json.hpp"
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>

Snowflake Snowflake::invalid_snowflake(0, 0);

Snowflake::Snowflake()
{
  *this = invalid_snowflake;
}

Snowflake::Snowflake(uint64_t most_sig, uint64_t least_sig)
  : m_most_sig(most_sig)
  , m_least_sig(least_sig)
{}

auto Snowflake::generate() -> Self
{
  Snowflake out;
  std::mt19937_64 gen{ std::random_device()() };
  std::uniform_int_distribution<uint64_t> dist;
  out.m_most_sig = dist(gen);
  out.m_least_sig = dist(gen);
  return out;
}

auto Snowflake::generate_64() -> Self
{
  Snowflake out;
  std::mt19937_64 gen{ std::random_device()() };
  std::uniform_int_distribution<uint64_t> dist;
  out.m_most_sig = dist(gen);
  return out;
}

auto Snowflake::invalid() -> Self
{
  return invalid_snowflake;
}

auto Snowflake::from_json(json::value_type json_value) -> std::optional<Self>
{
  switch (json_value.type()) {
    case nlohmann::detail::value_t::array:          return from_json_array(json_value);
    case nlohmann::detail::value_t::string:         return from_json_string(json_value);

    case nlohmann::detail::value_t::number_unsigned:
    case nlohmann::detail::value_t::number_integer: return from_json_number(json_value);

    default:                                        break;
  }
  std::clog << "[❓][UID] couldnt parse msg_id: " << json_value << '\n';
  return std::nullopt;
}

auto Snowflake::from_json_array(json::array_t array) -> std::optional<Self>
{
  constexpr int json_integral_size = sizeof(json::number_unsigned_t);
  static_assert(sizeof(json::number_integer_t) == sizeof(json::number_unsigned_t), "nlohmann json integers are not 64bit");
  static_assert(16 / json_integral_size == 2);

  if (array.size() != (16 / json_integral_size)) {
    return std::nullopt;
  }

  Snowflake out;
  uint64_t* fields[] = {
    &out.m_most_sig,
    &out.m_least_sig
  };
  static_assert(sizeof(fields) == 16);
  for (int i = 0; i < json_integral_size; ++i) {
    if (array[i].is_number())
      return std::nullopt;
    *fields[i] = array[i].get<uint64_t>();
  }

  return std::nullopt;
}

auto Snowflake::from_json_string(json::string_t string) -> std::optional<Self>
{
  Snowflake out;
  static_assert(sizeof(out.m_most_sig) == 8);
  static_assert(sizeof(out.m_least_sig) == 8);
  std::optional<std::string> decoded = encoding::decode_base64(string);
  if (!decoded.has_value())
    return std::nullopt;
  string = decoded.value();

  uint8_t* bytes[] = {
    (reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 0),
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 1,
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 2,
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 3,
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 4,
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 5,
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 6,
    reinterpret_cast<uint8_t*>(&out.m_most_sig)   + 7,

    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 0,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 1,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 2,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 3,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 4,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 5,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 6,
    reinterpret_cast<uint8_t*>(&out.m_least_sig)  + 7,
  };
  static_assert(sizeof(bytes) == (16 * sizeof(void*)));
  static_assert(sizeof(*bytes[1]) == 1);
  if (string.size() == 16) {
    for (int i = 0; i < 16; ++i) {
      *bytes[i] = string[i];
    }
    return out;
  }

  if (std::optional<std::string> decoded_64 = encoding::decode_base64(string); decoded_64.has_value()) {
    if (decoded_64.value().size() == 16) {
      for (int i = 0; i < 16; ++i) {
        *bytes[i] = decoded_64.value()[i];
      }
      return out;
    }
  }
  return std::nullopt;
}

auto Snowflake::as_json() const -> json::value_type
{
  if (m_most_sig != 0 && m_least_sig == 0) {
    return m_most_sig;
  }

  std::string out;
  static_assert(sizeof(m_most_sig) == 8);
  static_assert(sizeof(m_least_sig) == 8);
  out.resize(16);

  const uint8_t* bytes[] = {
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 0,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 1,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 2,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 3,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 4,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 5,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 6,
    reinterpret_cast<const uint8_t*>(&m_most_sig)   + 7,

    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 0,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 1,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 2,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 3,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 4,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 5,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 6,
    reinterpret_cast<const uint8_t*>(&m_least_sig)  + 7,
  };
  static_assert(sizeof(bytes) == (16 * sizeof(void*)));
  static_assert(sizeof(*bytes[1]) == 1);

  for (int i = 0; i < sizeof(bytes) / sizeof(void*); ++i)
    out[i] = *bytes[i];
  return encoding::encode_base64url(out).value_or("");
}

auto Snowflake::from_json_number(json::number_integer_t id) -> std::optional<Self>
{
  return Snowflake(id, 0);
}
