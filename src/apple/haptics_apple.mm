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
#import <CoreHaptics/CoreHaptics.h>
#include "inc/campello_input/haptics_apple.hpp"

using namespace systems::leal::campello_input;

API_AVAILABLE(macos(11.0), ios(14.0))
static CHHapticEngine *createEngine(GCDeviceHaptics *deviceHaptics) {
    NSError *error = nil;
    CHHapticEngine *engine = [deviceHaptics createEngineWithLocality:GCHapticsLocalityDefault];
    if (!engine) return nil;
    [engine startAndReturnError:&error];
    return error ? nil : engine;
}

AppleHaptics::AppleHaptics(void *gcDeviceHaptics)
    : engine(nullptr), player(nullptr)
{
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCDeviceHaptics *deviceHaptics = (__bridge GCDeviceHaptics *)gcDeviceHaptics;
        CHHapticEngine *eng = createEngine(deviceHaptics);
        if (eng) {
            engine = (void *)CFBridgingRetain(eng);
        }
    }
}

AppleHaptics::~AppleHaptics() {
    if (player) {
        CFBridgingRelease(player);
        player = nullptr;
    }
    if (engine) {
        if (@available(macOS 11.0, iOS 14.0, *)) {
            CHHapticEngine *eng = (__bridge CHHapticEngine *)engine;
            [eng stopWithCompletionHandler:nil];
        }
        CFBridgingRelease(engine);
        engine = nullptr;
    }
}

HapticsCapability AppleHaptics::capabilities() const noexcept {
    if (!engine) return HapticsCapability::none;
    return HapticsCapability::rumble;
    // Per-trigger haptics (trigger_left | trigger_right) would require
    // additional engines created with GCHapticsLocalityLeftTrigger /
    // GCHapticsLocalityRightTrigger — not yet implemented.
}

bool AppleHaptics::setRumble(const RumbleEffect& effect) {
    if (!engine) return false;

    if (@available(macOS 11.0, iOS 14.0, *)) {
        CHHapticEngine *eng = (__bridge CHHapticEngine *)engine;

        // Stop previous player if any.
        if (player) {
            id<CHHapticPatternPlayer> prev = (__bridge id<CHHapticPatternPlayer>)player;
            NSError *stopErr = nil;
            [prev stopAtTime:0 error:&stopErr];
            CFBridgingRelease(player);
            player = nullptr;
        }

        // Map dual-motor values: intensity = average, sharpness = high-freq ratio.
        float intensity  = (effect.lowFrequency + effect.highFrequency) * 0.5f;
        float sharpness  = effect.highFrequency;
        double duration  = effect.durationMs > 0 ? effect.durationMs / 1000.0 : 30.0;

        NSDictionary *hapticDict = @{
            CHHapticPatternKeyPattern: @[@{
                CHHapticPatternKeyEvent: @{
                    CHHapticPatternKeyEventType:      CHHapticEventTypeHapticContinuous,
                    CHHapticPatternKeyTime:           @0.0,
                    CHHapticPatternKeyEventDuration:  @(duration),
                    CHHapticPatternKeyEventParameters: @[
                        @{ CHHapticPatternKeyParameterID:    CHHapticEventParameterIDHapticIntensity,
                           CHHapticPatternKeyParameterValue: @(intensity) },
                        @{ CHHapticPatternKeyParameterID:    CHHapticEventParameterIDHapticSharpness,
                           CHHapticPatternKeyParameterValue: @(sharpness) },
                    ],
                },
            }],
        };

        NSError *error = nil;
        CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithDictionary:hapticDict
                                                                         error:&error];
        if (error) return false;

        id<CHHapticPatternPlayer> newPlayer = [eng createPlayerWithPattern:pattern error:&error];
        if (error) return false;

        [newPlayer startAtTime:0 error:&error];
        if (error) return false;

        player = (void *)CFBridgingRetain(newPlayer);
        return true;
    }
    return false;
}

bool AppleHaptics::stopRumble() {
    if (!player) return false;

    if (@available(macOS 11.0, iOS 14.0, *)) {
        id<CHHapticPatternPlayer> p = (__bridge id<CHHapticPatternPlayer>)player;
        NSError *error = nil;
        [p stopAtTime:0 error:&error];
        CFBridgingRelease(player);
        player = nullptr;
        return error == nil;
    }
    return false;
}

bool AppleHaptics::setTriggerEffect(bool /*leftTrigger*/, const TriggerEffect& /*effect*/) {
    // TODO: create per-trigger CHHapticEngine via GCHapticsLocalityLeftTrigger /
    // GCHapticsLocalityRightTrigger and play a matching CHHapticPattern.
    return false;
}

bool AppleHaptics::stopTriggerEffect(bool /*leftTrigger*/) {
    return false;
}
