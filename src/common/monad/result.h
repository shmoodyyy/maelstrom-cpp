#ifndef COMMON_MONAD_RESULT
#define COMMON_MONAD_RESULT

#include <cstring>
#include <ios>
#include <pthread.h>
#include <type_traits>
#include <utility>
#include <cstdlib>
#include <string_view>
#include <string>
#include <source_location>
#include <vector>

template<typename T>
struct Ok
{
  Ok(T value) : value(value) {}
  T value;
};

struct Error {
  std::string what;
  std::vector<std::source_location> where;

  Error()
    : what()
    , where()
  {}

  Error(std::string_view msg, std::source_location&& location)
    : what(msg)
    , where(0)
  {
    where.push_back(location);
  }

  auto expand(std::source_location&& location) -> Error&
  {
    where.push_back(location);
    return *this;
  }
};

#define ERROR_1(msg)                Error(msg, std::source_location::current())
#define ERROR_0()                   ERROR_1("")
#define PICK_ERROR(_1, name, ...)   name
#define ERROR(...)                  __VA_OPT__(ERROR_1(__VA_ARGS__)) ERROR_0()

#define ERROR_PROPAGATE(x)  x.as_err().expand(std::source_location::current())

template<typename T>
class Result
{
public:
  Result(Ok<T> ok)
    : is_valid(true)
    , m_internal(ok.value)
  {}
  Result(T value)
    : is_valid(true)
    , m_internal(value)
  {}
  Result(Error error)
    : is_valid(false)
    , m_internal(error)
  {}
  ~Result()
  {
    if (is_ok()) [[likely]]
      m_internal.valid_value.~T();
    else
      m_internal.err_value.~Error();
  }

  // copy-construct
  Result(const Result& other) = delete;

  // move-construct
  Result(Result&& other) : is_valid(other.is_valid), m_internal()
  {
    other.is_valid = false;
    std::swap(m_internal, other.m_internal);
  }

  // copy-assign
  Result& operator=(const Result& other)
  {
    if (this == &other)
      return *this;

    if (is_valid) {
      m_internal.valid_value.~T();
    } else {
      m_internal.err_value.~Error();
    }

    is_valid = other.is_valid;
    if (other.is_ok()) [[likely]] {
      new (&m_internal.valid_value) T(other.m_internal.valid_value);
    } else {
      new (&m_internal.err_value) Error(other.m_internal.err_value);
    }
    return *this;
  }

  // move-assign
  Result& operator=(Result&& other)
  {
    if (this == &other)
      return *this;

    if (is_valid) {
      m_internal.valid_value.~T();
    } else {
      m_internal.err_value.~Error();
    }

    is_valid = other.is_valid;
    if (other.is_ok()) [[likely]] {
      new (&m_internal.valid_value) T(std::move(other.m_internal.valid_value));
    } else {
      new (&m_internal.err_value) Error(std::move(other.m_internal.err_value));
    }
    return *this;
  }

  inline auto is_ok() const -> bool { return is_valid; }
  inline auto expect() -> T&&
  {
    if (!is_ok()) [[unlikely]] {
      auto it = m_internal.err_value.where.begin();
      std::source_location where = *it;
      ++it;
      printf(
        "[expect() on error type]\n[%s @ %s @ %u:%u]\n",
        where.function_name(),
        where.file_name(),
        where.line(),
        where.column());
      printf("\tError: %s\n\n", m_internal.err_value.what.data());
      if (it != m_internal.err_value.where.end())
        printf("propagated through:");
      while (it != m_internal.err_value.where.end()) {
        std::source_location where = *it;
        printf(
          "\n\t\t[%s @ %s @ %u:%u]",
          where.function_name(),
          where.file_name(),
          where.line(),
          where.column());
        ++it;
      }
      std::exit(-1);
    }
    return std::forward<T>(std::move(m_internal.valid_value));
  }
  inline auto as_err() -> Error&& {
    return std::forward<Error>(std::move(m_internal.err_value));
  }

private:
  bool is_valid;
  union internal_value {
    Error err_value;
    T valid_value;

    internal_value() {}
    internal_value(T value): valid_value(value) {}
    internal_value(Error error): err_value(error) {}
    ~internal_value() {}
  } m_internal;
};

#endif
