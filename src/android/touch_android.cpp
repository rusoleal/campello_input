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

#include <android/log.h>
#include "inc/campello_input/touch_android.hpp"

using namespace systems::leal::campello_input;

AndroidTouch::AndroidTouch() = default;

uint32_t AndroidTouch::id() const noexcept {
    return 0;  // Single touch device
}

const char* AndroidTouch::name() const noexcept {
    return "Android Touchscreen";
}

ConnectionStatus AndroidTouch::connectionStatus() const noexcept {
    return ConnectionStatus::connected;
}

uint32_t AndroidTouch::maxTouchPoints() const noexcept {
    // Android supports up to kMaxTouchPoints (10) in our implementation
    return kMaxTouchPoints;
}

void AndroidTouch::getState(TouchState& outState) const {
    std::lock_guard<std::mutex> lock(stateMutex);
    outState = currentState;
}

bool AndroidTouch::triggerHapticPulse(float intensity) {
    // TODO: Implement using Android Vibrator
    // For now, just log
    __android_log_print(ANDROID_LOG_DEBUG, "campello_input",
        "Haptic pulse requested: %.2f", intensity);
    (void)intensity;
    return false;
}

void AndroidTouch::updateState(const TouchState& state) {
    std::lock_guard<std::mutex> lock(stateMutex);
    currentState = state;
}
