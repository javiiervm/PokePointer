#include "protocol/InputDecoder.h"

namespace pokepointer {
DecodeResult decodeInput(std::span<const std::uint8_t> packet)
{
    // Conservative observed format; other lengths stay available as raw diagnostics.
    if (packet.size() != 17)
        return {{}, DecodeError::UnsupportedLength};
    if ((packet[1] & 0xfcU) != 0)
        return {{}, DecodeError::UnsupportedButtonBits};

    // Source-backed high eight bits only. See docs/protocol.md for the two sources.
    const auto x = static_cast<std::uint8_t>(((packet[3] & 0x0fU) << 4U) | (packet[2] >> 4U));
    return {InputState{x, packet[4], (packet[1] & 1U) != 0, (packet[1] & 2U) != 0},
            DecodeError::None};
}

const char* decodeErrorText(DecodeError error)
{
    switch (error) {
    case DecodeError::None: return "none";
    case DecodeError::UnsupportedLength: return "unsupported report length (expected 17 bytes)";
    case DecodeError::UnsupportedButtonBits: return "unknown button bits; report left undecoded";
    }
    return "unknown decoder error";
}
} // namespace pokepointer
