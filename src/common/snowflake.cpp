#include "snowflake.h"
#include <cmath>
#include <iostream>
#include <chrono>
#include <mutex>
#include <random>

Snowflake Snowflake::last_snowflake = Snowflake(-1);
std::mutex Snowflake::gen_mutex;
uint8_t Snowflake::node_id = -1;

void Snowflake::init_snowflakes(uint8_t node)
{
  std::unique_lock lock(gen_mutex);
  if (last_snowflake.good()) {
    std::clog << "[🦀][GID] attempting to reset snowflakes on initialized node\n";
    std::abort();
  }
  node_id = node;
}

Snowflake::Snowflake()
{
  std::unique_lock lock(gen_mutex);
  m_id = gen_snowflake_id();
  last_snowflake = *this;
}

Snowflake::Snowflake(uint64_t id)
  : m_id(id)
{}

auto Snowflake::gen_snowflake_id() -> const uint64_t
{
  if (node_id == (0xFF)) {
    std::clog << "[🦀][GID] generating snowflakes on uninitialized node\n";
    std::abort();
  }

  std::random_device rd;
  std::mt19937_64 e2(rd());
  std::uniform_int_distribution<uint64_t> dist;
  return dist(e2);

  std::chrono::milliseconds timestamp =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

  uint64_t timestamp_cast = timestamp.count();
  uint64_t timestamp_trunc = (timestamp_cast & 0xEFFFFFFF);
  return
    timestamp_trunc == last_snowflake.timestamp()
      ? last_snowflake.id()+1
      : (timestamp_trunc << 32) | (uint64_t)(last_snowflake.node()) << 16;
}

