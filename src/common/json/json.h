#ifndef COMMON_JSON_HEADER
#define COMMON_JSON_HEADER
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace json {

struct json;

enum Type {
  NULL_TYPE,
  BOOLEAN_TYPE,
  NUMBER_TYPE,
  STRING_TYPE,
  OBJECT_TYPE,
  ARRAY_TYPE,
};
static struct Null {
private:
  const int * const pad = new int(0);
} null_value;

template<typename T>
class Result {
  auto operator!() -> T;
};

auto type_as_str(Type type) -> const std::string_view;
class JsonVariable {
public:
  template<typename V>
  JsonVariable(std::string_view name, V value) 
    : name(name)
  {
    static_assert(!std::is_pointer_v<V>, "attempting to use distinct value JsonVariable constructor for collection");
    if constexpr (std::is_convertible_v<V, double>) {
      m_data.number = value;
    } else if (std::is_convertible_v<V, std::string>) {
      m_data.string = value;
    } else if (std::is_convertible_v<V, const char*>) {
      m_data.string = std::string(static_cast<const char*>(value));
    } else if (std::is_same_v<V, JsonVariable>) {
      m_data.object = value;
    } else {
    }
  }
  using Object = std::unordered_map<std::string, JsonVariable>;
  using Array = std::vector<JsonVariable>;

  const std::string name;
  auto is_null()      -> bool { return m_type == NULL_TYPE; }
  auto is_boolean()   -> bool { return m_type == BOOLEAN_TYPE; }
  auto is_number()    -> bool { return m_type == NUMBER_TYPE; }
  auto is_string()    -> bool { return m_type == STRING_TYPE; }
  auto is_object()    -> bool { return m_type == OBJECT_TYPE; }
  auto is_array()     -> bool { return m_type == ARRAY_TYPE; }

  auto unwrap_null()    -> Null;
  auto unwrap_boolean() -> bool;
  auto unwrap_number()  -> double;
  auto unwrap_string()  -> std::string;
  auto unwrap_array()   -> Array;

private:
  JsonVariable* m_parent;
  Type m_type;
  union {
    const Null null;
    bool boolean;
    double number;
    std::string string;
    Object object;
    Array array;
  } m_data;
};

}

#endif
