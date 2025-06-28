#include <utility>
#include <cstdlib>
#include <string_view>
#include <string>
#include <source_location>
#include <queue>

template<typename T>
struct Ok
{
  Ok(T value) : value(value) {}
  T value;
};

struct Error {
  std::string what;
  std::queue<std::source_location> where;

  Error(std::string_view msg, std::source_location&& location)
    : what(msg)
  {
    where.push(location);
  }

  auto expand(std::source_location&& location) -> Error&
  {
    where.push(location);
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
    if (is_ok())
      m_internal.valid_value.~T();
    else
      m_internal.err_value.~Error();
  }

  // copy-construct
  Result(const Result& other) : is_valid(other.is_valid), m_internal(0)
  {
    if (is_ok()) {
      new (&m_internal.valid_value) T(other.m_internal.valid_value);
    } else {
      new (&m_internal.err_value) Error(other.m_internal.err_value);
    }
  }

  // move-construct
  Result(Result&& other) : is_valid(other.is_valid), m_internal(other.is_valid ? other.m_internal.valid_value : other.m_internal.err_value)
  {
    if (is_ok()) {
      new (&m_internal.valid_value) T(std::move(other.m_internal.valid_value));
    } else {
      new (&m_internal.err_value) Error(std::move(other.m_internal.err_value));
    }
  }

  // copy-assign
  Result& operator=(const Result& other)
  {
    if (this == &other)
      return *this;

    this->~Result();
    is_valid = other.is_valid;
    if (is_ok()) {
      new (&m_internal.valid_value) T(other.m_internal.valid_value);
    } else {
      new (&m_internal.err_value) Error(other.m_internal.err_value);
    }
  }

  // move-assign
  Result& operator=(Result&& other)
  {
    if (this == &other)
      return *this;

    this->~Result();
    is_valid = other.is_valid;
    if (is_ok()) {
      new (&m_internal.valid_value) T(other.m_internal.valid_value);
    } else {
      new (&m_internal.err_value) Error(other.m_internal.err_value);
    }
  }

  inline auto is_ok()   -> bool { return is_valid; }
  inline auto expect()  -> T&& { return expect("");}
  inline auto expect(std::string_view msg) -> T&&
  {
    if (!is_ok()) {
      std::source_location where = m_internal.err_value.where.front(); m_internal.err_value.where.pop();
      printf(
        "[expect() on error type]\n[%s @ %s @ %i:%i]\n",
        where.function_name(),
        where.file_name(),
        where.line(),
        where.column());
      printf("\t%s\n\tError: %s\n\n", msg.data(), m_internal.err_value.what.data());
      if (!m_internal.err_value.where.empty())
        printf("propagated through:");
      while (!m_internal.err_value.where.empty()) {
        std::source_location where = m_internal.err_value.where.front(); m_internal.err_value.where.pop();
        printf(
          "\n\t\t[%s @ %s @ %i:%i]",
          where.function_name(),
          where.file_name(),
          where.line(),
          where.column());
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

    internal_value(T value): valid_value(value) {}
    internal_value(Error error): err_value(error) {}
    ~internal_value() {}
  } m_internal;
};

Result<int> sum(Result<int> a, Result<int> b)
{
  if (!a.is_ok())
    return ERROR_PROPAGATE(a);
  if (!b.is_ok())
    return ERROR_PROPAGATE(a);
  return a.expect() + b.expect();
}

Result<int> create_int_error() {
  return ERROR();
}

int main() {
    Result<int> a = 5;
    return sum(5, sum(7, a)).expect();
}
