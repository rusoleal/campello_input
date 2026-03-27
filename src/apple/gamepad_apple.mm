// MIT License
//
// Copyright (c) 2025 Ruben Leal Mirete
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#import <GameController/GameController.h>
#include "inc/campello_input/gamepad_apple.hpp"

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static uint64_t buttonMask(GCExtendedGamepad *gp) {
    uint64_t mask = 0;

    auto set = [&](GCControllerButtonInput *btn, GamepadButton bit) {
        if (btn && btn.pressed) mask |= static_cast<uint64_t>(bit);
    };

    set(gp.buttonA,              GamepadButton::face_south);
    set(gp.buttonB,              GamepadButton::face_east);
    set(gp.buttonX,              GamepadButton::face_west);
    set(gp.buttonY,              GamepadButton::face_north);
    set(gp.leftShoulder,         GamepadButton::shoulder_left);
    set(gp.rightShoulder,        GamepadButton::shoulder_right);
    set(gp.leftThumbstickButton, GamepadButton::thumb_left);
    set(gp.rightThumbstickButton,GamepadButton::thumb_right);
    set(gp.dpad.up,              GamepadButton::dpad_up);
    set(gp.dpad.down,            GamepadButton::dpad_down);
    set(gp.dpad.left,            GamepadButton::dpad_left);
    set(gp.dpad.right,           GamepadButton::dpad_right);

    if (@available(macOS 10.15, iOS 13.0, *)) {
        set(gp.buttonMenu,    GamepadButton::start);
        set(gp.buttonOptions, GamepadButton::select);
    }
    if (@available(macOS 11.0, iOS 14.0, *)) {
        set(gp.buttonHome, GamepadButton::home);
    }

    return mask;
}

// ---------------------------------------------------------------------------
// AppleGamepad
// ---------------------------------------------------------------------------

AppleGamepad::AppleGamepad(uint32_t id, void *gcController)
    : _id(id)
{
    GCController *ctrl = (__bridge GCController *)gcController;
    controller = (void *)CFBridgingRetain(ctrl);

    _name = ctrl.vendorName ? [ctrl.vendorName UTF8String] : "Unknown Controller";

    if (@available(macOS 11.0, iOS 14.0, *)) {
        if (ctrl.haptics) {
            _haptics = std::make_unique<AppleHaptics>((__bridge void *)ctrl.haptics);
        }
    }
}

AppleGamepad::~AppleGamepad() {
    _haptics.reset();
    if (controller) {
        CFBridgingRelease(controller);
        controller = nullptr;
    }
}

uint32_t AppleGamepad::id() const noexcept {
    return _id;
}

const char* AppleGamepad::name() const noexcept {
    return _name.c_str();
}

ConnectionStatus AppleGamepad::connectionStatus() const noexcept {
    GCController *ctrl = (__bridge GCController *)controller;
    // A GCController object is only alive while connected; if the underlying
    // object has been released by the system, the pointer will dangle. The
    // manager is responsible for destroying AppleGamepad on disconnect.
    return ctrl ? ConnectionStatus::connected : ConnectionStatus::disconnected;
}

bool AppleGamepad::getState(GamepadState& outState) const {
    GCController *ctrl = (__bridge GCController *)controller;
    GCExtendedGamepad *gp = ctrl.extendedGamepad;
    if (!gp) return false;

    // Timestamp: GC reports seconds since boot; convert to microseconds.
    outState.timestamp = static_cast<uint64_t>(gp.lastEventTimestamp * 1e6);

    outState.buttons = buttonMask(gp);

    // Axes — GC convention: left stick +Y = up. We normalise to +Y = down.
    auto &axes = outState.axes;
    axes[static_cast<uint8_t>(GamepadAxis::left_x)].value        =  gp.leftThumbstick.xAxis.value;
    axes[static_cast<uint8_t>(GamepadAxis::left_y)].value        = -gp.leftThumbstick.yAxis.value;
    axes[static_cast<uint8_t>(GamepadAxis::right_x)].value       =  gp.rightThumbstick.xAxis.value;
    axes[static_cast<uint8_t>(GamepadAxis::right_y)].value       = -gp.rightThumbstick.yAxis.value;
    axes[static_cast<uint8_t>(GamepadAxis::trigger_left)].value  =  gp.leftTrigger.value;
    axes[static_cast<uint8_t>(GamepadAxis::trigger_right)].value =  gp.rightTrigger.value;

    // GC framework applies its own dead-zone; report 0 (unknown to us).
    for (auto &a : axes) a.deadzone = 0.0f;

    return true;
}

Haptics* AppleGamepad::haptics() noexcept {
    return _haptics.get();
}
