#include "core/ConnectionState.h"

namespace pokepointer {
bool Lifecycle::transition(ConnectionState next)
{
    using enum ConnectionState;
    if (state_ == next || state_ == Stopped)
        return false;
    bool allowed = next == ShuttingDown && state_ != ShuttingDown;
    if (next == Reconnecting)
        allowed = state_ == Scanning || state_ == Connecting || state_ == Discovering
               || state_ == Subscribing || state_ == Streaming;
    switch (state_) {
    case Idle: allowed |= next == Scanning; break;
    case Scanning: allowed |= next == Connecting; break;
    case Connecting: allowed |= next == Discovering; break;
    case Discovering: allowed |= next == Subscribing; break;
    case Subscribing: allowed |= next == Streaming; break;
    case Streaming: break;
    case Reconnecting: allowed |= next == Scanning; break;
    case ShuttingDown: allowed = next == Stopped; break;
    case Stopped: break;
    }
    if (allowed)
        state_ = next;
    return allowed;
}

const char* stateName(ConnectionState state)
{
    switch (state) {
    case ConnectionState::Idle: return "idle";
    case ConnectionState::Scanning: return "scanning";
    case ConnectionState::Connecting: return "connecting";
    case ConnectionState::Discovering: return "gatt-discovery";
    case ConnectionState::Subscribing: return "subscribing";
    case ConnectionState::Streaming: return "streaming";
    case ConnectionState::Reconnecting: return "reconnecting";
    case ConnectionState::ShuttingDown: return "shutting-down";
    case ConnectionState::Stopped: return "stopped";
    }
    return "unknown";
}
} // namespace pokepointer
