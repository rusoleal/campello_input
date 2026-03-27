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

#include "inc/campello_input/touch_apple.hpp"

#if TARGET_OS_IOS || TARGET_OS_TV

#import <UIKit/UIKit.h>
#import <CoreHaptics/CoreHaptics.h>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static TouchPhase phaseFromUITouchPhase(UITouchPhase phase) {
    switch (phase) {
        case UITouchPhaseBegan:       return TouchPhase::began;
        case UITouchPhaseMoved:       return TouchPhase::moved;
        case UITouchPhaseStationary:  return TouchPhase::stationary;
        case UITouchPhaseEnded:       return TouchPhase::ended;
        case UITouchPhaseCancelled:   return TouchPhase::cancelled;
        default:                      return TouchPhase::cancelled;
    }
}

static void applyTouches(TouchState& state, NSSet<UITouch *> *touches, UIView *view) {
    for (UITouch *touch in touches) {
        // Find an existing slot by touch pointer hash or add a new one.
        uint32_t touchId = static_cast<uint32_t>((uintptr_t)touch & 0xffffffff);

        TouchPoint *slot = nullptr;
        for (uint32_t i = 0; i < state.count; i++) {
            if (state.points[i].id == touchId) { slot = &state.points[i]; break; }
        }

        if (!slot) {
            if (state.count >= kMaxTouchPoints) continue;
            slot = &state.points[state.count++];
            slot->id = touchId;
        }

        CGPoint cur  = [touch locationInView:view];
        CGPoint prev = [touch previousLocationInView:view];

        slot->phase     = phaseFromUITouchPhase(touch.phase);
        slot->x         = static_cast<float>(cur.x);
        slot->y         = static_cast<float>(cur.y);
        slot->deltaX    = static_cast<float>(cur.x - prev.x);
        slot->deltaY    = static_cast<float>(cur.y - prev.y);
        slot->pressure  = static_cast<float>(touch.force > 0 ? touch.force / touch.maximumPossibleForce : 1.0f);
        slot->radius    = static_cast<float>(touch.majorRadius);
        slot->timestamp = static_cast<uint64_t>(touch.timestamp * 1e6);

        // Remove ended/cancelled touches from the active list after copying data.
        if (slot->phase == TouchPhase::ended || slot->phase == TouchPhase::cancelled) {
            uint32_t idx = static_cast<uint32_t>(slot - state.points);
            state.points[idx] = state.points[--state.count];
        }
    }
}

// ---------------------------------------------------------------------------
// AppleTouch
// ---------------------------------------------------------------------------

uint32_t AppleTouch::id() const noexcept { return 0; }

const char* AppleTouch::name() const noexcept { return "Touchscreen"; }

ConnectionStatus AppleTouch::connectionStatus() const noexcept {
    return ConnectionStatus::connected;
}

uint32_t AppleTouch::maxTouchPoints() const noexcept {
    return static_cast<uint32_t>([UIScreen mainScreen].traitCollection.maximumNumberOfTouches);
}

void AppleTouch::getState(TouchState& outState) const {
    std::lock_guard<std::mutex> lock(stateMutex);
    outState = currentState;
}

bool AppleTouch::triggerHapticPulse(float intensity) {
    if (@available(iOS 13.0, *)) {
        if (!CHHapticEngine.capabilitiesForHardware.supportsHaptics) return false;

        NSError *error = nil;
        CHHapticEngine *engine = [[CHHapticEngine alloc] initAndReturnError:&error];
        if (error) return false;

        [engine startAndReturnError:&error];
        if (error) return false;

        NSDictionary *hapticDict = @{
            CHHapticPatternKeyPattern: @[@{
                CHHapticPatternKeyEvent: @{
                    CHHapticPatternKeyEventType:      CHHapticEventTypeHapticTransient,
                    CHHapticPatternKeyTime:           @0.0,
                    CHHapticPatternKeyEventParameters: @[
                        @{ CHHapticPatternKeyParameterID:    CHHapticEventParameterIDHapticIntensity,
                           CHHapticPatternKeyParameterValue: @(intensity) },
                        @{ CHHapticPatternKeyParameterID:    CHHapticEventParameterIDHapticSharpness,
                           CHHapticPatternKeyParameterValue: @0.5 },
                    ],
                },
            }],
        };

        CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithDictionary:hapticDict error:&error];
        if (error) return false;

        id<CHHapticPatternPlayer> p = [engine createPlayerWithPattern:pattern error:&error];
        if (error) return false;

        [p startAtTime:0 error:&error];
        return error == nil;
    }

    // Fallback: UIImpactFeedbackGenerator (iOS 10+)
    UIImpactFeedbackGenerator *gen = [[UIImpactFeedbackGenerator alloc]
        initWithStyle:UIImpactFeedbackStyleMedium];
    [gen impactOccurred];
    return true;
}

void AppleTouch::processTouchesBegan(void *touches, void */*event*/) {
    std::lock_guard<std::mutex> lock(stateMutex);
    applyTouches(currentState, (__bridge NSSet<UITouch *> *)touches, nil);
}

void AppleTouch::processTouchesMoved(void *touches, void */*event*/) {
    std::lock_guard<std::mutex> lock(stateMutex);
    applyTouches(currentState, (__bridge NSSet<UITouch *> *)touches, nil);
}

void AppleTouch::processTouchesEnded(void *touches, void */*event*/) {
    std::lock_guard<std::mutex> lock(stateMutex);
    applyTouches(currentState, (__bridge NSSet<UITouch *> *)touches, nil);
}

void AppleTouch::processTouchesCancelled(void *touches, void */*event*/) {
    std::lock_guard<std::mutex> lock(stateMutex);
    applyTouches(currentState, (__bridge NSSet<UITouch *> *)touches, nil);
}

#endif // TARGET_OS_IOS || TARGET_OS_TV
