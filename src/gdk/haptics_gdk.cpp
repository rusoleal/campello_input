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

#include "inc/campello_input/haptics_gdk.hpp"
#include <GameInput.h>

using namespace systems::leal::campello_input;

GdkHaptics::GdkHaptics(IGameInputDevice* device)
    : _device(device)
{
    if (_device) {
        _device->AddRef();
    }
}

GdkHaptics::~GdkHaptics() {
    // Stop any ongoing rumble
    stopRumble();
    
    if (_rumbleDevice) {
        _rumbleDevice->Release();
        _rumbleDevice = nullptr;
    }
    
    if (_device) {
        _device->Release();
        _device = nullptr;
    }
}

bool GdkHaptics::initialize() {
    if (!_device) return false;
    
    // Query for rumble interface
    HRESULT hr = _device->QueryInterface(&IGameInputRumbleDevice::GetRuntimeClassName(), 
                                          reinterpret_cast<void**>(&_rumbleDevice));
    if (FAILED(hr) || !_rumbleDevice) {
        // Device doesn't support rumble
        return false;
    }
    
    checkCapabilities();
    _initialized = true;
    return true;
}

void GdkHaptics::checkCapabilities() {
    if (!_rumbleDevice) return;
    
    _capabilities = HapticsCapability::none;
    
    // Check for simple rumble support (all rumble devices support this)
    _capabilities = _capabilities | HapticsCapability::rumble;
    
    // Xbox controllers and some PC gamepads support high-frequency rumble
    // We assume all GameInput rumble devices support both frequencies
    _capabilities = _capabilities | HapticsCapability::rumble_high_freq;
    
    // Check for trigger rumble (Xbox Series X|S controllers)
    // GameInput doesn't have a direct API to check this, but we can try
    // to set trigger rumble and see if it succeeds
    GameInputRumbleState testState{};
    testState.leftTrigger = 0.01f;  // Very small test value
    HRESULT hr = _rumbleDevice->SetRumbleState(&testState);
    if (SUCCEEDED(hr)) {
        _capabilities = _capabilities | HapticsCapability::trigger_left;
        _capabilities = _capabilities | HapticsCapability::trigger_right;
        
        // Clear the test rumble
        testState.leftTrigger = 0.0f;
        _rumbleDevice->SetRumbleState(&testState);
    }
}

bool GdkHaptics::supports(HapticsCapability capability) const noexcept {
    return (static_cast<int>(_capabilities) & static_cast<int>(capability)) != 0;
}

bool GdkHaptics::playRumble(float lowFreqIntensity, float highFreqIntensity, uint32_t durationMs) {
    if (!_initialized || !_rumbleDevice) return false;
    
    // Clamp intensities to [0, 1]
    lowFreqIntensity = lowFreqIntensity < 0.0f ? 0.0f : (lowFreqIntensity > 1.0f ? 1.0f : lowFreqIntensity);
    highFreqIntensity = highFreqIntensity < 0.0f ? 0.0f : (highFreqIntensity > 1.0f ? 1.0f : highFreqIntensity);
    
    // Store current values
    _currentLowFreq = lowFreqIntensity;
    _currentHighFreq = highFreqIntensity;
    
    // Set rumble state
    GameInputRumbleState state{};
    state.lowFrequency = lowFreqIntensity;
    state.highFrequency = highFreqIntensity;
    
    HRESULT hr = _rumbleDevice->SetRumbleState(&state);
    
    // Note: GameInput doesn't have a built-in duration mechanism.
    // The caller is responsible for calling stopRumble() after durationMs.
    // Alternatively, we could start a timer thread here.
    
    return SUCCEEDED(hr);
}

bool GdkHaptics::stopRumble() {
    if (!_initialized || !_rumbleDevice) return false;
    
    // Clear all rumble
    GameInputRumbleState state{};
    _currentLowFreq = 0.0f;
    _currentHighFreq = 0.0f;
    
    HRESULT hr = _rumbleDevice->SetRumbleState(&state);
    return SUCCEEDED(hr);
}

bool GdkHaptics::playTriggerEffect(TriggerEffect effect, TriggerIntensity intensity, uint32_t durationMs) {
    if (!_initialized || !_rumbleDevice) return false;
    
    // Check if trigger rumble is supported
    if (!supports(HapticsCapability::trigger_left) && !supports(HapticsCapability::trigger_right)) {
        return false;
    }
    
    // Convert intensity enum to float
    float intensityValue;
    switch (intensity) {
        case TriggerIntensity::light:  intensityValue = 0.3f; break;
        case TriggerIntensity::medium: intensityValue = 0.6f; break;
        case TriggerIntensity::heavy:  intensityValue = 1.0f; break;
        default: intensityValue = 0.5f;
    }
    
    // Convert effect to rumble type
    // On Xbox controllers, trigger effects are just vibration
    float triggerValue = intensityValue;
    
    GameInputRumbleState state{};
    state.lowFrequency = _currentLowFreq;
    state.highFrequency = _currentHighFreq;
    
    switch (effect) {
        case TriggerEffect::vibration:
        case TriggerEffect::weapon:
        case TriggerEffect::feedback:
            // These all map to simple vibration on Xbox triggers
            if (supports(HapticsCapability::trigger_left)) {
                state.leftTrigger = triggerValue;
            }
            if (supports(HapticsCapability::trigger_right)) {
                state.rightTrigger = triggerValue;
            }
            break;
            
        case TriggerEffect::resistance:
        case TriggerEffect::slope:
        case TriggerEffect::soft:
            // These are DualSense-specific and not supported on Xbox
            // Fall back to vibration
            if (supports(HapticsCapability::trigger_left)) {
                state.leftTrigger = triggerValue;
            }
            if (supports(HapticsCapability::trigger_right)) {
                state.rightTrigger = triggerValue;
            }
            break;
    }
    
    HRESULT hr = _rumbleDevice->SetRumbleState(&state);
    
    // Note: No automatic duration - caller must call stopTriggerEffect()
    (void)durationMs;
    
    return SUCCEEDED(hr);
}

bool GdkHaptics::stopTriggerEffect(bool isLeftTrigger) {
    if (!_initialized || !_rumbleDevice) return false;
    
    GameInputRumbleState state{};
    state.lowFrequency = _currentLowFreq;
    state.highFrequency = _currentHighFreq;
    
    if (isLeftTrigger) {
        state.leftTrigger = 0.0f;
    } else {
        state.rightTrigger = 0.0f;
    }
    
    HRESULT hr = _rumbleDevice->SetRumbleState(&state);
    return SUCCEEDED(hr);
}
