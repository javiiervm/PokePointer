#pragma once

#include <algorithm>
#include <chrono>

namespace pokepointer {
class Backoff {
public:
    [[nodiscard]] std::chrono::milliseconds next()
    {
        const auto result = delay_;
        delay_ = std::min(delay_ * 2, std::chrono::milliseconds{30000});
        return result;
    }
    void reset() { delay_ = std::chrono::milliseconds{1000}; }
private:
    std::chrono::milliseconds delay_{1000};
};
} // namespace pokepointer
