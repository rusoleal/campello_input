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

#include "inc/campello_input/mouse_linux.hpp"
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>
#include <linux/input.h>
#include <linux/input-event-codes.h>

using namespace systems::leal::campello_input;

static MouseButton linuxToMouseButton(int btn) {
    switch (btn) {
        case BTN_LEFT: return MouseButton::left;
        case BTN_RIGHT: return MouseButton::right;
        case BTN_MIDDLE: return MouseButton::middle;
        case BTN_SIDE: return MouseButton::extra_1;
        case BTN_EXTRA: return MouseButton::extra_2;
        default: return MouseButton::left;
    }
}

LinuxMouse::LinuxMouse(uint32_t id, const char* name, const char* devicePath)
    : _id(id)
    , _name(name ? name : "Unknown Mouse")
    , _devicePath(devicePath ? devicePath : "")
{
}

LinuxMouse::~LinuxMouse() {
    if (_evdev) {
        libevdev_free(_evdev);
        _evdev = nullptr;
    }
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
    }
}

bool LinuxMouse::initialize() {
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

    // Check if this is a pointer device
    if (!libevdev_has_property(_evdev, INPUT_PROP_POINTER)) {
        // Not a pointer, but might still be a mouse without that property set
        // Check for relative X axis
        if (!libevdev_has_event_code(_evdev, EV_REL, REL_X)) {
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

    return true;
}

void LinuxMouse::updateState() {
    if (!_evdev) return;

    struct input_event ev;
    int rc;

    // Accumulate relative movement for this frame
    int32_t dx = 0;
    int32_t dy = 0;

    while ((rc = libevdev_next_event(_evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) == 0) {
        if (ev.type == EV_REL) {
            switch (ev.code) {
                case REL_X:
                    dx += ev.value;
                    break;
                case REL_Y:
                    dy += ev.value;
                    break;
                case REL_WHEEL:
                    _state.scroll_delta_y += static_cast<float>(ev.value);
                    break;
                case REL_HWHEEL:
                    _state.scroll_delta_x += static_cast<float>(ev.value);
                    break;
            }
        } else if (ev.type == EV_ABS) {
            // Handle absolute positioning (some trackpads use this)
            if (ev.code == ABS_X) {
                // Calculate delta from last known position or set absolute
                dx = ev.value - _lastAbsX;
                _lastAbsX = ev.value;
                _hasAbsX = true;
            } else if (ev.code == ABS_Y) {
                dy = ev.value - _lastAbsY;
                _lastAbsY = ev.value;
                _hasAbsY = true;
            }
        } else if (ev.type == EV_KEY) {
            if (ev.code >= BTN_MOUSE && ev.code < BTN_JOYSTICK) {
                MouseButton btn = linuxToMouseButton(ev.code);
                if (ev.value) {
                    _state.buttons |= static_cast<uint32_t>(btn);
                } else {
                    _state.buttons &= ~static_cast<uint32_t>(btn);
                }
            }
        }
    }

    // Apply accumulated deltas
    _state.delta_x = dx;
    _state.delta_y = dy;

    // Update position (relative to previous)
    _state.pos_x += dx;
    _state.pos_y += dy;

    // Update timestamp
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _state.timestamp = static_cast<uint64_t>(ts.tv_sec) * 1000000 + ts.tv_nsec / 1000;
}

bool LinuxMouse::getState(MouseState& outState) const {
    outState = _state;
    return true;
}

void LinuxMouse::resetDeltas() noexcept {
    _state.delta_x = 0;
    _state.delta_y = 0;
    _state.scroll_delta_x = 0.0f;
    _state.scroll_delta_y = 0.0f;
}

bool LinuxMouse::isButtonDown(MouseButton button) const noexcept {
    return (_state.buttons & static_cast<uint32_t>(button)) != 0;
}

void LinuxMouse::setCursorLocked(bool locked) {
    _locked = locked;
    // Note: On Linux, cursor locking usually requires display server integration
    // (X11 or Wayland). This implementation just tracks the state.
    // For a complete implementation, you would need to use X11 GrabPointer
    // or Wayland relative pointer protocols.
}
