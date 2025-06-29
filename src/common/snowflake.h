#ifndef COMMON_SNOWFLAKE_HEADER
#define COMMON_SNOWFLAKE_HEADER
#include "ext/nlohmann/json.hpp"
#include <cstdint>
#include <mutex>

// thread-safe not-xitter-compliant snowflake impl
class Snowflake
{
public:
  static void init_snowflakes(uint8_t node);
  Snowflake();
  Snowflake(uint64_t id);

  auto id() const         -> const uint64_t { return m_id; }
  auto good() const       -> const bool     { return (id() >> 63) == 0; }
  auto timestamp() const  -> const uint64_t { return (id() >> 32) & 0xEFFFFFFF; }
  auto node() const       -> const uint8_t  { return id() >> 24; }
  auto increment() const  -> const uint32_t { return id() << 8 >> 8; }

  operator uint64_t()                   const { return id(); }
  operator nlohmann::json::value_type() const { return nlohmann::json::value_type(id()); }
private:
  uint64_t m_id;
  static auto gen_snowflake_id() -> const uint64_t;
  static std::mutex gen_mutex;
  static Snowflake  last_snowflake;
  static uint8_t   node_id;
};

#endif
