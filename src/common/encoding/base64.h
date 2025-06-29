#ifndef AMBER_ENCODING_BASE64URL
#define AMBER_ENCODING_BASE64URL
#include <string>
#include <optional>

namespace encoding {
  auto encode_base64(const std::string_view data) -> std::optional<std::string>;
  auto decode_base64(const std::string_view data) -> std::optional<std::string>;

  auto encode_base64url(const std::string_view data) -> std::optional<std::string>;
  auto decode_base64url(const std::string_view data) -> std::optional<std::string>;
}

#endif
