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

#pragma once

// Touch is only available on iOS/iPadOS/tvOS.
// On macOS there is no UITouch; trackpad gestures come via NSResponder, not a device API.
#if TARGET_OS_IOS || TARGET_OS_TV

#include <campello_input/touch.hpp>
#include <mutex>

namespace systems::leal::campello_input {

    // AppleTouch is fed UITouch events from the app layer (UIView or GameActivity).
    // The app must call processTouch* methods from its UIResponder overrides.
    class AppleTouch : public TouchDevice {
        mutable std::mutex stateMutex;
        TouchState currentState{};

    public:
        AppleTouch() = default;

        uint32_t         id()               const noexcept override;
        const char*      name()             const noexcept override;
        ConnectionStatus connectionStatus() const noexcept override;

        uint32_t maxTouchPoints() const noexcept override;
        void     getState(TouchState& outState) const override;
        bool     triggerHapticPulse(float intensity) override;

        // Called from the app's UIView/UIViewController touch callbacks.
        // touches: NSSet<UITouch *>*, event: UIEvent*
        void processTouchesBegan(void *touches, void *event);
        void processTouchesMoved(void *touches, void *event);
        void processTouchesEnded(void *touches, void *event);
        void processTouchesCancelled(void *touches, void *event);
    };

}

#endif // TARGET_OS_IOS || TARGET_OS_TV
