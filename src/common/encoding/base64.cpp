#include "base64.h"
#include <cstdint>
#include <optional>

namespace {
    constexpr const char* const base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto to_base64_char(uint8_t byte) -> char {
        return byte < 64 ? base64_chars[byte] : '?';
    }

    auto from_base64(char c) -> std::optional<uint8_t> {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        else if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        else if (c >= '0' && c <= '9') return c - '0' + 52;
        else if (c == '-' || c == '_') return c == '-' ? 62 : 63;
        else return std::nullopt;
    }

    constexpr const char* const base64url_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    auto to_base64url_char(uint8_t byte) -> char {
        return byte < 64 ? base64url_chars[byte] : '?';
    }

    auto from_base64url(char c) -> std::optional<uint8_t> {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        else if (c >= 'a' && c <= 'z') return c - 'a' + 26;
        else if (c >= '0' && c <= '9') return c - '0' + 52;
        else if (c == '-' || c == '_') return c == '-' ? 62 : 63;
        else return std::nullopt;
    }
}

// base64
auto encoding::decode_base64(std::string_view base64) -> std::optional<std::string> {
    const long char_count = (base64.size() * 6) / 8;
    std::string out;
    if (char_count == 0) {
        return out;
    }
    out.resize(char_count);
    long    out_idx = 0;
    bool    carry = false;
    uint8_t bits_to_read = 6;
    uint8_t final_shift = 0;
    for (const char c : base64) {
        std::optional<uint8_t> val;
        switch (bits_to_read) 
        {
        case 6:
            val = from_base64(c);
            if (!val.has_value()) return std::nullopt;
            out[out_idx]    |= (val.value() & 0b00111111) << (carry ? 0 : 2);
            out_idx += carry;
            bits_to_read = carry ? 6 : 2;
            final_shift = carry ? 0 : 2;
            carry = false;
            break;
        case 2:
            val = from_base64(c);
            if (!val.has_value()) return std::nullopt;
            out[out_idx++]  |= (val.value() & 0b00110000) >> 4;
            out[out_idx]    |= (val.value() & 0b00001111) << 4;
            bits_to_read = 4;
            final_shift = 4;
            break;
        case 4:
            val = from_base64(c);
            if (!val.has_value()) return std::nullopt;
            out[out_idx++]  |= (val.value() & 0b00111100) >> 2;
            out[out_idx]    |= (val.value() & 0b00000011) << 6;
            carry = true;
            bits_to_read = 6;
            final_shift = 6;
            break;
        default: return std::nullopt; // garbage
        }
    }
    out[out_idx] >>= final_shift;
    return out;
}

auto encoding::encode_base64(std::string_view data) -> std::optional<std::string> {
    const long char_count = (data.size() * 8 + 5) / 6;
    const long padding = data.size() % 3;
    std::string out;
    if (data.size() == 0) {
        return out;
    }
    out.resize(char_count);
    long    out_idx = 0;
    uint8_t carry = 0;
    uint8_t carry_bits = 0;
    for (long i = 0; i < data.size(); ++i) {
        const uint8_t byte = static_cast<uint8_t>(data[i]);
        switch (carry_bits)
        {
        case 0:
            out[out_idx++] = to_base64_char((byte & 0b11111100) >> 2);
            carry = (byte & 0b00000011) << 4;
            carry_bits = 2;
            break;
        case 2:
            out[out_idx++] = to_base64url_char(carry | ((byte & 0b11110000) >> 4));
            carry = (byte & 0b00001111) << 2;
            carry_bits = 4;
            break;
        case 4:
            out[out_idx++] = to_base64_char(carry | ((byte & 0b11000000) >> 6));
            out[out_idx++] = to_base64_char(byte & 0b00111111);
            carry = 0;
            carry_bits = 0;
            break;

        default: return std::nullopt; // garbage
        }
    }
    while (out_idx < char_count) {
        out[out_idx++] = to_base64_char(carry);
        carry = 0;
    }
    out.append(padding, '=');
    return out;
}


// base64url
auto encoding::decode_base64url(std::string_view base64) -> std::optional<std::string> {
    const long char_count = (base64.size() * 6) / 8;
    std::string out;
    if (char_count == 0) {
        return out;
    }
    out.resize(char_count);
    long    out_idx = 0;
    bool    carry = false;
    uint8_t bits_to_read = 6;
    uint8_t final_shift = 0;
    for (const char c : base64) {
        std::optional<uint8_t> val;
        switch (bits_to_read) 
        {
        case 6:
            val = from_base64url(c);
            if (!val.has_value()) return std::nullopt;
            out[out_idx]    |= (val.value() & 0b00111111) << (carry ? 0 : 2);
            out_idx += carry;
            bits_to_read = carry ? 6 : 2;
            final_shift = carry ? 0 : 2;
            carry = false;
            break;
        case 2:
            val = from_base64url(c);
            if (!val.has_value()) return std::nullopt;
            out[out_idx++]  |= (val.value() & 0b00110000) >> 4;
            out[out_idx]    |= (val.value() & 0b00001111) << 4;
            bits_to_read = 4;
            final_shift = 4;
            break;
        case 4:
            val = from_base64url(c);
            if (!val.has_value()) return std::nullopt;
            out[out_idx++]  |= (val.value() & 0b00111100) >> 2;
            out[out_idx]    |= (val.value() & 0b00000011) << 6;
            carry = true;
            bits_to_read = 6;
            final_shift = 6;
            break;
        default: return std::nullopt; // garbage
        }
    }
    out[out_idx] >>= final_shift;
    return out;
}

auto encoding::encode_base64url(std::string_view data) -> std::optional<std::string> {
    const long char_count = (data.size() * 8 + 5) / 6;
    std::string out;
    if (data.size() == 0) {
        return out;
    }
    out.resize(char_count);
    long    out_idx = 0;
    uint8_t carry = 0;
    uint8_t carry_bits = 0;
    for (long i = 0; i < data.size(); ++i) {
        const uint8_t byte = static_cast<uint8_t>(data[i]);
        switch (carry_bits)
        {
        case 0:
            out[out_idx++] = to_base64url_char((byte & 0b11111100) >> 2);
            carry = (byte & 0b00000011) << 4;
            carry_bits = 2;
            break;
        case 2:
            out[out_idx++] = to_base64url_char(carry | ((byte & 0b11110000) >> 4));
            carry = (byte & 0b00001111) << 2;
            carry_bits = 4;
            break;
        case 4:
            out[out_idx++] = to_base64url_char(carry | ((byte & 0b11000000) >> 6));
            out[out_idx++] = to_base64url_char(byte & 0b00111111);
            carry = 0;
            carry_bits = 0;
            break;

        default: return std::nullopt; // garbage
        }
    }
    while (out_idx < char_count) {
        out[out_idx++] = to_base64url_char(carry);
        carry = 0;
    }
    return out;
}
