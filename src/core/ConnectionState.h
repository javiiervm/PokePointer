#pragma once

namespace pokepointer {
enum class ConnectionState {
    Idle, Scanning, Connecting, Discovering, Subscribing, Streaming,
    Reconnecting, ShuttingDown, Stopped
};

class Lifecycle {
public:
    [[nodiscard]] ConnectionState state() const { return state_; }
    // Invalid and repeated transitions are rejected, including attempts after stop.
    bool transition(ConnectionState next);
private:
    ConnectionState state_{ConnectionState::Idle};
};
[[nodiscard]] const char* stateName(ConnectionState state);
} // namespace pokepointer
