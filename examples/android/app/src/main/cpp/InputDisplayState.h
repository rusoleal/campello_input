#pragma once

#include <campello_input/gamepad.hpp>
#include <campello_input/touch.hpp>
#include <deque>
#include <string>

// ---------------------------------------------------------------------------
// InputDisplayState — snapshot built each frame from AndroidInput
// ---------------------------------------------------------------------------

struct InputDisplayState {
    static constexpr int kMaxGamepads = 4;

    struct GamepadEntry {
        bool    active   = false;
        int32_t deviceId = -1;
        char    name[64] = {};
        systems::leal::campello_input::GamepadState state{};
    } gamepads[kMaxGamepads];
    int gamepadCount = 0;

    systems::leal::campello_input::TouchState touch{};

    std::deque<std::string> recentKeys;  // newest first, max 10
};
