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

using namespace systems::leal::campello_input;

// Note: This is a stub implementation.
// The GameInput API in the current Windows SDK doesn't expose IGameInputRumbleDevice
// or GameInputRumbleState structures. Haptics support would require:
// 1. Using the RawDeviceReport API to send rumble commands directly
// 2. Or using the Force Feedback API (IGameInputForceFeedbackDevice)
// 3. Or waiting for a newer GDK version with haptics support

GdkHaptics::GdkHaptics(IGameInputDevice* device)
    : _device(device)
{
    (void)_device;
}

GdkHaptics::~GdkHaptics() = default;

bool GdkHaptics::initialize() {
    // Haptics not supported with current GameInput API
    return false;
}

HapticsCapability GdkHaptics::capabilities() const noexcept {
    return HapticsCapability::none;
}

bool GdkHaptics::setRumble(const RumbleEffect& effect) {
    (void)effect;
    return false;
}

bool GdkHaptics::stopRumble() {
    return false;
}

bool GdkHaptics::setTriggerEffect(bool leftTrigger, const TriggerEffect& effect) {
    (void)leftTrigger;
    (void)effect;
    return false;
}

bool GdkHaptics::stopTriggerEffect(bool leftTrigger) {
    (void)leftTrigger;
    return false;
}
