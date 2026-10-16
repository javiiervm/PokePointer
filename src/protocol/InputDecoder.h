#pragma once

#include "core/InputState.h"
#include <optional>
#include <span>
#include <string_view>

namespace pokepointer {
inline constexpr std::string_view inputServiceUuid = "6675e16c-f36d-4567-bb55-6b51e27a23e5";
inline constexpr std::string_view inputCharacteristicUuid = "6675e16c-f36d-4567-bb55-6b51e27a23e6";
enum class DecodeError { None, UnsupportedLength, UnsupportedButtonBits };
struct DecodeResult {
    std::optional<InputState> state;
    DecodeError error{DecodeError::None};
};

// Accepts only the known characteristic's 17-byte report, without an HID prefix.
[[nodiscard]] DecodeResult decodeInput(std::span<const std::uint8_t> packet);
[[nodiscard]] const char* decodeErrorText(DecodeError error);
} // namespace pokepointer
