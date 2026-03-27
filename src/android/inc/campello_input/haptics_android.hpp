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
#include <android/native_activity.h>
#include <jni.h>

namespace systems::leal::campello_input {

    // AndroidHaptics - implements Haptics for Android using the Vibrator service
    //
    // This is an internal class. Access via GamepadDevice::haptics() on Android.
    //
    // Supports:
    // - Simple vibration (rumble) using Vibrator.vibrate()
    // - Android 8.0+ (API 26+): VibrationEffect for better control
    // - Android 10+ (API 29+): HapticGenerator for audio-based haptics
    //
    // Note: Android haptics capabilities vary widely by device. Some devices
    // support rich haptics, others only simple on/off vibration.
    class AndroidHaptics : public Haptics {
        ANativeActivity* _activity = nullptr;
        jobject _vibrator = nullptr;
        jmethodID _vibrateMethod = nullptr;
        jmethodID _hasVibratorMethod = nullptr;
        jmethodID _hasAmplitudeControlMethod = nullptr;
        
        // API 26+ (Android 8.0) support
        jclass _vibrationEffectClass = nullptr;
        jmethodID _createOneShotMethod = nullptr;
        jmethodID _createWaveformMethod = nullptr;
        bool _hasVibrationEffect = false;
        
        // Device capabilities
        HapticsCapability _capabilities = HapticsCapability::none;
        bool _initialized = false;

    public:
        AndroidHaptics();
        ~AndroidHaptics() override;

        // Disable copy/move
        AndroidHaptics(const AndroidHaptics&) = delete;
        AndroidHaptics& operator=(const AndroidHaptics&) = delete;

        // Initialize with the native activity
        bool initialize(ANativeActivity* activity);

        // Haptics interface
        bool supports(HapticsCapability capability) const noexcept override;
        
        bool playRumble(float lowFreqIntensity, float highFreqIntensity, 
                       uint32_t durationMs) override;
        bool stopRumble() override;
        
        bool playTriggerEffect(TriggerEffect effect, TriggerIntensity intensity,
                              uint32_t durationMs) override;
        bool stopTriggerEffect(bool isLeftTrigger) override;

    private:
        void cleanup();
        bool checkCapabilities();
        bool vibrateSimple(uint32_t durationMs);
        bool vibrateWithAmplitude(uint32_t durationMs, int amplitude);
    };

} // namespace systems::leal::campello_input
