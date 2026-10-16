#include "core/DeviceSelection.h"

namespace pokepointer {
DeviceSelection selectDevice(std::span<const DeviceCandidate> devices,
                             std::string_view explicitId, std::string_view cachedId)
{
    const auto preferred = explicitId.empty() ? cachedId : explicitId;
    if (!preferred.empty()) {
        for (std::size_t i = 0; i < devices.size(); ++i)
            if (devices[i].identifier == preferred)
                return {SelectionStatus::Selected, i, false};
        if (!explicitId.empty())
            return {};
    }
    std::optional<std::size_t> found;
    for (std::size_t i = 0; i < devices.size(); ++i) {
        if (!devices[i].looksLikePokeball)
            continue;
        if (found)
            return {SelectionStatus::Ambiguous, {}, false};
        found = i;
    }
    if (!found)
        return {};
    return {SelectionStatus::Selected, found, !cachedId.empty()};
}
} // namespace pokepointer
