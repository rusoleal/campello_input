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
#if TARGET_OS_OSX
#  import <AppKit/AppKit.h>
#endif
#include "inc/campello_input/mouse_apple.hpp"

using namespace systems::leal::campello_input;

AppleMouse::AppleMouse(void *gcMouse)
    : mouse(nullptr), _locked(false)
{
    GCMouse *m = (__bridge GCMouse *)gcMouse;
    mouse = (void *)CFBridgingRetain(m);

    if (@available(macOS 11.0, iOS 14.0, *)) {
        // Accumulate raw deltas via the moved handler so no movement is lost
        // between getState() calls. Captured pointer is cleared in the destructor
        // before this object is destroyed.
        AppleMouse *self_ptr = this;
        m.mouseInput.mouseMovedHandler =
            ^(GCMouseInput * /*input*/, float dx, float dy) {
                std::lock_guard<std::mutex> lock(self_ptr->stateMutex);
                self_ptr->accumDeltaX += dx;
                self_ptr->accumDeltaY -= dy;  // GC +Y = up; we normalise to +Y = down
            };
    }
}

AppleMouse::~AppleMouse() {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCMouse *m = (__bridge GCMouse *)mouse;
        m.mouseInput.mouseMovedHandler = nil;  // clear before this object dies
    }
    if (mouse) {
        CFBridgingRelease(mouse);
        mouse = nullptr;
    }
}

uint32_t AppleMouse::id() const noexcept {
    return 0;
}

const char* AppleMouse::name() const noexcept {
    return "Mouse";
}

ConnectionStatus AppleMouse::connectionStatus() const noexcept {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCMouse *m = (__bridge GCMouse *)mouse;
        return m ? ConnectionStatus::connected : ConnectionStatus::disconnected;
    }
    return ConnectionStatus::disconnected;
}

void AppleMouse::getState(MouseState& outState) const {
    outState = {};

    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCMouse *m = (__bridge GCMouse *)mouse;
        GCMouseInput *input = m.mouseInput;
        if (!input) return;

        float dx, dy;
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            dx = accumDeltaX;
            dy = accumDeltaY;
            accumDeltaX = 0.0f;
            accumDeltaY = 0.0f;
        }

        outState.deltaX = dx;
        outState.deltaY = dy;

        if (!_locked) {
            absX += dx;
            absY += dy;
        }
        outState.x = absX;
        outState.y = absY;

        // scroll — GCDeviceCursor extends GCControllerDirectionPad, so xAxis/yAxis work.
        // GC scroll +Y = scroll up; invert to natural (downward scroll = positive).
        outState.scrollX =  input.scroll.xAxis.value;
        outState.scrollY = -input.scroll.yAxis.value;

        // Buttons — cast array elements to the typed class to access .pressed
        uint8_t buttons = 0;
        if (input.leftButton   && input.leftButton.pressed)   buttons |= 1 << static_cast<uint8_t>(MouseButton::left);
        if (input.rightButton  && input.rightButton.pressed)  buttons |= 1 << static_cast<uint8_t>(MouseButton::right);
        if (input.middleButton && input.middleButton.pressed) buttons |= 1 << static_cast<uint8_t>(MouseButton::middle);
        if (input.auxiliaryButtons.count > 0) {
            GCControllerButtonInput *b = input.auxiliaryButtons[0];
            if (b.pressed) buttons |= 1 << static_cast<uint8_t>(MouseButton::extra1);
        }
        if (input.auxiliaryButtons.count > 1) {
            GCControllerButtonInput *b = input.auxiliaryButtons[1];
            if (b.pressed) buttons |= 1 << static_cast<uint8_t>(MouseButton::extra2);
        }
        outState.buttons = buttons;

        outState.timestamp = static_cast<uint64_t>([[NSProcessInfo processInfo] systemUptime] * 1e6);
    }
}

bool AppleMouse::isButtonDown(MouseButton button) const noexcept {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCMouse *m = (__bridge GCMouse *)mouse;
        GCMouseInput *input = m.mouseInput;
        if (!input) return false;

        switch (button) {
            case MouseButton::left:   return input.leftButton   && input.leftButton.pressed;
            case MouseButton::right:  return input.rightButton  && input.rightButton.pressed;
            case MouseButton::middle: return input.middleButton && input.middleButton.pressed;
            case MouseButton::extra1:
                if (input.auxiliaryButtons.count > 0) {
                    GCControllerButtonInput *b = input.auxiliaryButtons[0];
                    return b.pressed;
                }
                return false;
            case MouseButton::extra2:
                if (input.auxiliaryButtons.count > 1) {
                    GCControllerButtonInput *b = input.auxiliaryButtons[1];
                    return b.pressed;
                }
                return false;
            default: return false;
        }
    }
    return false;
}

void AppleMouse::setCursorLocked(bool locked) {
    _locked = locked;
#if TARGET_OS_OSX
    if (locked) {
        CGAssociateMouseAndMouseCursorPosition(false);
        [NSCursor hide];
    } else {
        CGAssociateMouseAndMouseCursorPosition(true);
        [NSCursor unhide];
    }
#endif
}

bool AppleMouse::isCursorLocked() const noexcept {
    return _locked;
}
