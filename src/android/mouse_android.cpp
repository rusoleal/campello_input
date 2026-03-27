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

#include "inc/campello_input/mouse_android.hpp"
#include <android/input.h>
#include <android/log.h>

using namespace systems::leal::campello_input;

AndroidMouse::AndroidMouse(uint32_t id) : _id(id) {}

void AndroidMouse::getState(MouseState& outState) const {
    std::lock_guard<std::mutex> lock(stateMutex);
    outState = currentState;
    // Deltas are already in currentState, they'll be reset by resetAccumulators()
}

bool AndroidMouse::isButtonDown(MouseButton button) const noexcept {
    std::lock_guard<std::mutex> lock(stateMutex);
    return (currentState.buttons & static_cast<uint32_t>(button)) != 0;
}

void AndroidMouse::setCursorLocked(bool locked) {
    _locked = locked;
    // Note: Cursor locking on Android requires platform-specific handling
    // via View.requestPointerCapture() on API 26+ (Android 8.0)
    // This is a simplified implementation that just tracks the state
    __android_log_print(ANDROID_LOG_DEBUG, "campello_input",
        "Mouse cursor locked: %s", locked ? "true" : "false");
}

void AndroidMouse::onMotionEvent(float deltaX, float deltaY, float posX, float posY) {
    std::lock_guard<std::mutex> lock(stateMutex);
    
    _accumDeltaX += deltaX;
    _accumDeltaY += deltaY;
    
    currentState.delta_x = static_cast<int32_t>(_accumDeltaX);
    currentState.delta_y = static_cast<int32_t>(_accumDeltaY);
    currentState.pos_x = static_cast<int32_t>(posX);
    currentState.pos_y = static_cast<int32_t>(posY);
}

void AndroidMouse::onScrollEvent(float deltaX, float deltaY) {
    std::lock_guard<std::mutex> lock(stateMutex);
    
    _accumScrollX += deltaX;
    _accumScrollY += deltaY;
    
    currentState.scroll_delta_x = _accumScrollX;
    currentState.scroll_delta_y = _accumScrollY;
}

void AndroidMouse::onButtonEvent(MouseButton button, bool pressed) {
    std::lock_guard<std::mutex> lock(stateMutex);
    
    if (pressed) {
        currentState.buttons |= static_cast<uint32_t>(button);
    } else {
        currentState.buttons &= ~static_cast<uint32_t>(button);
    }
}

void AndroidMouse::resetAccumulators() {
    std::lock_guard<std::mutex> lock(stateMutex);
    
    // Reset accumulated deltas after they've been read
    _accumDeltaX = 0.0f;
    _accumDeltaY = 0.0f;
    _accumScrollX = 0.0f;
    _accumScrollY = 0.0f;
    
    currentState.delta_x = 0;
    currentState.delta_y = 0;
    currentState.scroll_delta_x = 0.0f;
    currentState.scroll_delta_y = 0.0f;
}
