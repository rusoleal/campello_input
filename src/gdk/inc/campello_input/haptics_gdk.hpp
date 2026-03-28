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

#include <campello_input/haptics.hpp>

// Forward declarations for GameInput
struct IGameInputDevice;

namespace systems::leal::campello_input {

    // GdkHaptics - Windows/Xbox GDK GameInput haptics implementation
    //
    // Note: This is currently a stub implementation. The GameInput API in the
    // current Windows SDK doesn't expose the haptics/rumble interfaces that
    // were used in the original implementation.
    //
    // Future implementations could use:
    // - RawDeviceReport API to send rumble commands directly
    // - Force Feedback API (IGameInputForceFeedbackDevice) if available
    class GdkHaptics : public Haptics {
        IGameInputDevice* _device = nullptr;

    public:
        explicit GdkHaptics(IGameInputDevice* device);
        ~GdkHaptics() override;

        // Disable copy/move
        GdkHaptics(const GdkHaptics&) = delete;
        GdkHaptics& operator=(const GdkHaptics&) = delete;

        // Initialize haptics for the device
        bool initialize();

        // Haptics interface - all return "not supported"
        [[nodiscard]] HapticsCapability capabilities() const noexcept override;
        
        [[nodiscard]] bool setRumble(const RumbleEffect& effect) override;
        [[nodiscard]] bool stopRumble() override;
        
        [[nodiscard]] bool setTriggerEffect(bool leftTrigger, const TriggerEffect& effect) override;
        [[nodiscard]] bool stopTriggerEffect(bool leftTrigger) override;
    };

} // namespace systems::leal::campello_input
