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
struct IGameInputRumbleDevice;
struct IGameInputDevice;

namespace systems::leal::campello_input {

    // GdkHaptics - Windows/Xbox GDK GameInput haptics implementation
    //
    // This is an internal class. Access via GamepadDevice::haptics() on Windows/Xbox.
    //
    // Supports:
    // - Simple rumble (low/high frequency motors)
    // - Trigger rumble (Xbox Series X|S controllers)
    //
    // Note: Haptics capabilities depend on the controller hardware.
    // Xbox controllers support full rumble + trigger feedback.
    // Standard PC gamepads may only support simple rumble.
    class GdkHaptics : public Haptics {
        IGameInputRumbleDevice* _rumbleDevice = nullptr;
        IGameInputDevice* _device = nullptr;
        HapticsCapability _capabilities = HapticsCapability::none;
        bool _initialized = false;

        // Current rumble state
        float _currentLowFreq = 0.0f;
        float _currentHighFreq = 0.0f;

    public:
        explicit GdkHaptics(IGameInputDevice* device);
        ~GdkHaptics() override;

        // Disable copy/move
        GdkHaptics(const GdkHaptics&) = delete;
        GdkHaptics& operator=(const GdkHaptics&) = delete;

        // Initialize haptics for the device
        bool initialize();

        // Haptics interface
        bool supports(HapticsCapability capability) const noexcept override;
        
        bool playRumble(float lowFreqIntensity, float highFreqIntensity, 
                       uint32_t durationMs) override;
        bool stopRumble() override;
        
        bool playTriggerEffect(TriggerEffect effect, TriggerIntensity intensity,
                              uint32_t durationMs) override;
        bool stopTriggerEffect(bool isLeftTrigger) override;

    private:
        void checkCapabilities();
    };

} // namespace systems::leal::campello_input
