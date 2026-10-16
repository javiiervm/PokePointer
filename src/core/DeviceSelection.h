#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace pokepointer {
struct DeviceCandidate {
    std::string identifier;
    bool looksLikePokeball{};
};
enum class SelectionStatus { Selected, NotFound, Ambiguous };
struct DeviceSelection {
    SelectionStatus status{SelectionStatus::NotFound};
    std::optional<std::size_t> index;
    bool replacedCachedIdentity{};
};
// Identifiers must already be canonical and candidates deduplicated by the transport.
[[nodiscard]] DeviceSelection selectDevice(std::span<const DeviceCandidate> devices,
    std::string_view explicitId, std::string_view cachedId);
} // namespace pokepointer
