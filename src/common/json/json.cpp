#include "json.h"

auto json::type_as_str(Type type) -> const std::string_view {
  switch (type) {
    case NULL_TYPE:     return "null";
    case BOOLEAN_TYPE:  return "boolean";
    case NUMBER_TYPE:   return "number";
    case STRING_TYPE:   return "string";
    case OBJECT_TYPE:   return "object";
    case ARRAY_TYPE:    return "array";
  }
}

auto json::JsonVariable::unwrap_null() -> Null {
}
