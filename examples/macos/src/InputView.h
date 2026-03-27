#pragma once

// This header mixes C++ and Objective-C — include only from .mm translation units.

#import <AppKit/AppKit.h>

#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// InputDisplayState — snapshot filled by InputViewController each frame
// ---------------------------------------------------------------------------

struct InputDisplayState {

    struct GamepadInfo {
        bool        connected = false;
        std::string name;
        systems::leal::campello_input::GamepadState state{};
    };
    std::vector<GamepadInfo> gamepads;  // up to 3 displayed

    struct KeyboardInfo {
        bool available = false;
        systems::leal::campello_input::KeyboardState   state{};
        systems::leal::campello_input::KeyModifier     modifiers
            = systems::leal::campello_input::KeyModifier::none;
        std::vector<std::string> recentKeys;  // newest first, max 10
    } keyboard;

    struct MouseInfo {
        bool available = false;
        systems::leal::campello_input::MouseState state{};
    } mouse;
};

// ---------------------------------------------------------------------------

@interface InputView : NSView
- (void)updateWithState:(const InputDisplayState &)state;
@end
