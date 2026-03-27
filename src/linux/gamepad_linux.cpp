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

#include "inc/campello_input/gamepad_linux.hpp"
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <linux/input.h>
#include <linux/input-event-codes.h>

using namespace systems::leal::campello_input;

LinuxGamepad::LinuxGamepad(uint32_t id, const char* name, const char* devicePath)
    : _id(id)
    , _name(name ? name : "Unknown Gamepad")
    , _devicePath(devicePath ? devicePath : "")
{
}

LinuxGamepad::~LinuxGamepad() {
    if (_evdev) {
        libevdev_free(_evdev);
        _evdev = nullptr;
    }
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
    }
}

bool LinuxGamepad::initialize() {
    _fd = open(_devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (_fd < 0) {
        return false;
    }

    int rc = libevdev_new_from_fd(_fd, &_evdev);
    if (rc < 0) {
        close(_fd);
        _fd = -1;
        return false;
    }

    // Check if this is actually a gamepad/joystick
    if (!libevdev_has_property(_evdev, INPUT_PROP_JOYSTICK) &&
        !libevdev_has_property(_evdev, INPUT_PROP_BUTTONPAD)) {
        // Not a gamepad, but could still be a joystick
        // Check for typical gamepad axes
        if (!libevdev_has_event_code(_evdev, EV_ABS, ABS_X)) {
            libevdev_free(_evdev);
            _evdev = nullptr;
            close(_fd);
            _fd = -1;
            return false;
        }
    }

    // Update name from device
    const char* devName = libevdev_get_name(_evdev);
    if (devName) {
        _name = devName;
    }

    setupMappings();
    return true;
}

void LinuxGamepad::setupMappings() {
    if (!_evdev) return;

    // Check for absolute axes
    _hasAbsX = libevdev_has_event_code(_evdev, EV_ABS, ABS_X);
    _hasAbsY = libevdev_has_event_code(_evdev, EV_ABS, ABS_Y);
    _hasAbsRX = libevdev_has_event_code(_evdev, EV_ABS, ABS_RX);
    _hasAbsRY = libevdev_has_event_code(_evdev, EV_ABS, ABS_RY);
    _hasAbsZ = libevdev_has_event_code(_evdev, EV_ABS, ABS_Z);
    _hasAbsRZ = libevdev_has_event_code(_evdev, EV_ABS, ABS_RZ);
    _hasAbsHat0X = libevdev_has_event_code(_evdev, EV_ABS, ABS_HAT0X);
    _hasAbsHat0Y = libevdev_has_event_code(_evdev, EV_ABS, ABS_HAT0Y);
    _hasAbsGas = libevdev_has_event_code(_evdev, EV_ABS, ABS_GAS);
    _hasAbsBrake = libevdev_has_event_code(_evdev, EV_ABS, ABS_BRAKE);
}

float LinuxGamepad::normalizeAxis(int code, int value) const {
    if (!_evdev) return 0.0f;

    const struct input_absinfo* abs = libevdev_get_abs_info(_evdev, code);
    if (!abs) return 0.0f;

    int min = abs->minimum;
    int max = abs->maximum;
    if (max == min) return 0.0f;

    // Normalize to [-1, 1] or [0, 1]
    float normalized = (2.0f * (value - min) / (max - min)) - 1.0f;
    
    // Apply deadzone
    float deadzone = abs->fuzz / static_cast<float>(max - min);
    if (std::abs(normalized) < deadzone) {
        normalized = 0.0f;
    }

    return normalized;
}

void LinuxGamepad::updateState() {
    if (!_evdev) return;

    struct input_event ev;
    int rc;

    // Read all pending events
    while ((rc = libevdev_next_event(_evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) == 0) {
        if (ev.type == EV_ABS) {
            float value = normalizeAxis(ev.code, ev.value);
            
            switch (ev.code) {
                case ABS_X:
                    _state.axes[static_cast<uint8_t>(GamepadAxis::left_x)].value = value;
                    break;
                case ABS_Y:
                    // Invert Y axis (Linux uses +up, we use +down)
                    _state.axes[static_cast<uint8_t>(GamepadAxis::left_y)].value = -value;
                    break;
                case ABS_RX:
                    _state.axes[static_cast<uint8_t>(GamepadAxis::right_x)].value = value;
                    break;
                case ABS_RY:
                    // Invert Y axis
                    _state.axes[static_cast<uint8_t>(GamepadAxis::right_y)].value = -value;
                    break;
                case ABS_Z:
                case ABS_BRAKE:
                    // Map to [0, 1]
                    _state.axes[static_cast<uint8_t>(GamepadAxis::trigger_left)].value = (value + 1.0f) / 2.0f;
                    break;
                case ABS_RZ:
                case ABS_GAS:
                    _state.axes[static_cast<uint8_t>(GamepadAxis::trigger_right)].value = (value + 1.0f) / 2.0f;
                    break;
                case ABS_HAT0X:
                    if (ev.value < 0) {
                        _state.buttons |= static_cast<uint64_t>(GamepadButton::dpad_left);
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_right);
                    } else if (ev.value > 0) {
                        _state.buttons |= static_cast<uint64_t>(GamepadButton::dpad_right);
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_left);
                    } else {
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_left);
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_right);
                    }
                    break;
                case ABS_HAT0Y:
                    if (ev.value < 0) {
                        _state.buttons |= static_cast<uint64_t>(GamepadButton::dpad_up);
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_down);
                    } else if (ev.value > 0) {
                        _state.buttons |= static_cast<uint64_t>(GamepadButton::dpad_down);
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_up);
                    } else {
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_up);
                        _state.buttons &= ~static_cast<uint64_t>(GamepadButton::dpad_down);
                    }
                    break;
            }
        } else if (ev.type == EV_KEY) {
            uint64_t buttonBit = 0;
            
            switch (ev.code) {
                case BTN_A: case BTN_GAMEPAD: case BTN_SOUTH:
                    buttonBit = static_cast<uint64_t>(GamepadButton::face_south);
                    break;
                case BTN_B: case BTN_EAST:
                    buttonBit = static_cast<uint64_t>(GamepadButton::face_east);
                    break;
                case BTN_X: case BTN_WEST:
                    buttonBit = static_cast<uint64_t>(GamepadButton::face_west);
                    break;
                case BTN_Y: case BTN_NORTH:
                    buttonBit = static_cast<uint64_t>(GamepadButton::face_north);
                    break;
                case BTN_TL:
                    buttonBit = static_cast<uint64_t>(GamepadButton::shoulder_left);
                    break;
                case BTN_TR:
                    buttonBit = static_cast<uint64_t>(GamepadButton::shoulder_right);
                    break;
                case BTN_TL2:
                    buttonBit = static_cast<uint64_t>(GamepadButton::shoulder_left);
                    break;
                case BTN_TR2:
                    buttonBit = static_cast<uint64_t>(GamepadButton::shoulder_right);
                    break;
                case BTN_SELECT: case BTN_BACK:
                    buttonBit = static_cast<uint64_t>(GamepadButton::select);
                    break;
                case BTN_START: case BTN_FORWARD:
                    buttonBit = static_cast<uint64_t>(GamepadButton::start);
                    break;
                case BTN_MODE:
                    buttonBit = static_cast<uint64_t>(GamepadButton::home);
                    break;
                case BTN_THUMBL:
                    buttonBit = static_cast<uint64_t>(GamepadButton::thumb_left);
                    break;
                case BTN_THUMBR:
                    buttonBit = static_cast<uint64_t>(GamepadButton::thumb_right);
                    break;
                case BTN_DPAD_UP:
                    buttonBit = static_cast<uint64_t>(GamepadButton::dpad_up);
                    break;
                case BTN_DPAD_DOWN:
                    buttonBit = static_cast<uint64_t>(GamepadButton::dpad_down);
                    break;
                case BTN_DPAD_LEFT:
                    buttonBit = static_cast<uint64_t>(GamepadButton::dpad_left);
                    break;
                case BTN_DPAD_RIGHT:
                    buttonBit = static_cast<uint64_t>(GamepadButton::dpad_right);
                    break;
                case BTN_TRIGGER_HAPPY1:  // Some controllers use these for extra buttons
                    buttonBit = static_cast<uint64_t>(GamepadButton::misc1);
                    break;
            }

            if (buttonBit != 0) {
                if (ev.value) {
                    _state.buttons |= buttonBit;
                } else {
                    _state.buttons &= ~buttonBit;
                }
            }
        }
    }

    // Update timestamp (microseconds)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _state.timestamp = static_cast<uint64_t>(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
}

bool LinuxGamepad::getState(GamepadState& outState) const {
    outState = _state;
    return true;
}
