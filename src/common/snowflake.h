#ifndef COMMON_SNOWFLAKE_HEADER
#define COMMON_SNOWFLAKE_HEADER
#include "ext/nlohmann/json.hpp"
#include <cstdint>

// thread-safe not-xitter-compliant snowflake impl
class Snowflake
{
  using Self = Snowflake;
  using json = nlohmann::json;
public:
  Snowflake();

  static auto generate()                  -> Self;
  static auto generate_64()               -> Self;
  static auto invalid()                   -> Self;
  static auto from_json(json::value_type) -> std::optional<Self>;

  auto as_json() const -> json::value_type;
  auto is_valid() const   -> bool                   { return m_most_sig != 0 || m_least_sig != 0; }
private:
  Snowflake(uint64_t, uint64_t);

  static auto from_json_array(json::array_t array) -> std::optional<Self>;
  static auto from_json_string(json::string_t string) -> std::optional<Self>;
  static auto from_json_number(json::number_integer_t id) -> std::optional<Self>;
  static Snowflake invalid_snowflake;

  uint64_t m_most_sig;
  uint64_t m_least_sig;
};

#endif
