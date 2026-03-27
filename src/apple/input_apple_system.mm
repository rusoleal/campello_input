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

#import <Foundation/Foundation.h>
#import <GameController/GameController.h>

#include "inc/campello_input/input_apple_system.hpp"
#include "inc/campello_input/gamepad_apple.hpp"
#include "inc/campello_input/keyboard_apple.hpp"
#include "inc/campello_input/mouse_apple.hpp"

#include <algorithm>
#include <memory>
#include <vector>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// ObjC helper — owns device instances and fires DeviceObserver callbacks.
// All methods run on the main queue (or are dispatched there).
// ---------------------------------------------------------------------------

@interface CIAppleInputHelper : NSObject

- (void)cleanup;
- (void)addObserver:(DeviceObserver *)obs;
- (void)removeObserver:(DeviceObserver *)obs;

- (int)gamepadCount;
- (GamepadDevice *)gamepadAt:(int)index;
- (KeyboardDevice *)keyboard;
- (MouseDevice *)mouse;
- (bool)hasKeyboard;
- (bool)hasMouse;

@end

@implementation CIAppleInputHelper {
    std::vector<DeviceObserver *> _observers;

    struct GamepadEntry {
        void *gcCtrlPtr;   // raw __bridge pointer for identity (not retained)
        std::unique_ptr<AppleGamepad> device;
    };
    std::vector<GamepadEntry> _gamepads;
    uint32_t                  _nextGamepadId;

    std::unique_ptr<AppleKeyboard> _keyboard;
    std::unique_ptr<AppleMouse>    _mouse;
}

- (instancetype)init {
    if (!(self = [super init])) return nil;
    _nextGamepadId = 0;

    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

    [nc addObserver:self selector:@selector(controllerDidConnect:)
               name:GCControllerDidConnectNotification    object:nil];
    [nc addObserver:self selector:@selector(controllerDidDisconnect:)
               name:GCControllerDidDisconnectNotification object:nil];

    for (GCController *ctrl in GCController.controllers) {
        [self addController:ctrl];
    }

    if (@available(macOS 11.0, iOS 14.0, *)) {
        [nc addObserver:self selector:@selector(keyboardDidConnect:)
                   name:GCKeyboardDidConnectNotification    object:nil];
        [nc addObserver:self selector:@selector(keyboardDidDisconnect:)
                   name:GCKeyboardDidDisconnectNotification object:nil];
        [nc addObserver:self selector:@selector(mouseDidConnect:)
                   name:GCMouseDidConnectNotification       object:nil];
        [nc addObserver:self selector:@selector(mouseDidDisconnect:)
                   name:GCMouseDidDisconnectNotification    object:nil];

        if (GCKeyboard.coalescedKeyboard) {
            [self connectKeyboard:GCKeyboard.coalescedKeyboard];
        }
        if (GCMouse.current) {
            [self connectMouse:GCMouse.current];
        }
    }

    return self;
}

- (void)cleanup {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

// ---------------------------------------------------------------------------
// Observer registration
// ---------------------------------------------------------------------------

- (void)addObserver:(DeviceObserver *)obs {
    _observers.push_back(obs);
    // Immediately notify about already-connected devices.
    for (auto &entry : _gamepads) {
        obs->onDeviceConnected(*entry.device);
    }
    if (_keyboard) obs->onDeviceConnected(*_keyboard);
    if (_mouse)    obs->onDeviceConnected(*_mouse);
}

- (void)removeObserver:(DeviceObserver *)obs {
    auto it = std::find(_observers.begin(), _observers.end(), obs);
    if (it != _observers.end()) _observers.erase(it);
}

// ---------------------------------------------------------------------------
// Device access
// ---------------------------------------------------------------------------

- (int)gamepadCount {
    return (int)_gamepads.size();
}

- (GamepadDevice *)gamepadAt:(int)index {
    if (index < 0 || index >= (int)_gamepads.size()) return nullptr;
    return _gamepads[index].device.get();
}

- (KeyboardDevice *)keyboard {
    return _keyboard.get();
}

- (MouseDevice *)mouse {
    return _mouse.get();
}

- (bool)hasKeyboard {
    return _keyboard != nullptr;
}

- (bool)hasMouse {
    return _mouse != nullptr;
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

- (void)fireConnected:(Device *)device {
    for (auto *obs : _observers) obs->onDeviceConnected(*device);
}

- (void)fireDisconnected:(Device *)device {
    for (auto *obs : _observers) obs->onDeviceDisconnected(*device);
}

- (void)addController:(GCController *)ctrl {
    if (!ctrl.extendedGamepad) return;
    void *ptr = (__bridge void *)ctrl;
    for (auto &e : _gamepads) {
        if (e.gcCtrlPtr == ptr) return;   // already tracked
    }
    GamepadEntry entry;
    entry.gcCtrlPtr = ptr;
    entry.device = std::make_unique<AppleGamepad>(_nextGamepadId++, (__bridge void *)ctrl);
    auto *dev = entry.device.get();
    _gamepads.push_back(std::move(entry));
    [self fireConnected:dev];
}

- (void)removeController:(GCController *)ctrl {
    void *ptr = (__bridge void *)ctrl;
    auto it = std::find_if(_gamepads.begin(), _gamepads.end(),
        [ptr](const GamepadEntry &e) { return e.gcCtrlPtr == ptr; });
    if (it == _gamepads.end()) return;
    [self fireDisconnected:it->device.get()];
    _gamepads.erase(it);
}

- (void)connectKeyboard:(GCKeyboard *)kb API_AVAILABLE(macos(11.0), ios(14.0)) {
    _keyboard = std::make_unique<AppleKeyboard>((__bridge void *)kb);
    [self fireConnected:_keyboard.get()];
}

- (void)connectMouse:(GCMouse *)m API_AVAILABLE(macos(11.0), ios(14.0)) {
    _mouse = std::make_unique<AppleMouse>((__bridge void *)m);
    [self fireConnected:_mouse.get()];
}

// ---------------------------------------------------------------------------
// Notification handlers
// ---------------------------------------------------------------------------

- (void)controllerDidConnect:(NSNotification *)note {
    GCController *ctrl = note.object;
    dispatch_async(dispatch_get_main_queue(), ^{ [self addController:ctrl]; });
}

- (void)controllerDidDisconnect:(NSNotification *)note {
    GCController *ctrl = note.object;
    dispatch_async(dispatch_get_main_queue(), ^{ [self removeController:ctrl]; });
}

- (void)keyboardDidConnect:(NSNotification *)note {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCKeyboard *kb = note.object;
        dispatch_async(dispatch_get_main_queue(), ^{ [self connectKeyboard:kb]; });
    }
}

- (void)keyboardDidDisconnect:(NSNotification *)note {
    (void)note;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self->_keyboard) {
            [self fireDisconnected:self->_keyboard.get()];
            self->_keyboard.reset();
        }
    });
}

- (void)mouseDidConnect:(NSNotification *)note {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCMouse *m = note.object;
        dispatch_async(dispatch_get_main_queue(), ^{ [self connectMouse:m]; });
    }
}

- (void)mouseDidDisconnect:(NSNotification *)note {
    (void)note;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self->_mouse) {
            [self fireDisconnected:self->_mouse.get()];
            self->_mouse.reset();
        }
    });
}

@end

// ---------------------------------------------------------------------------
// AppleInputSystem — C++ implementation
// ---------------------------------------------------------------------------

namespace systems::leal::campello_input {

AppleInputSystem::AppleInputSystem(const PlatformContext& /*ctx*/) {
    impl = (void *)[[CIAppleInputHelper alloc] init];

#if TARGET_OS_IOS || TARGET_OS_TV
    _touch = std::make_unique<AppleTouch>();
#endif
}

AppleInputSystem::~AppleInputSystem() {
    CIAppleInputHelper *h = (CIAppleInputHelper *)impl;
    [h cleanup];
    [h release];
    impl = nullptr;
}

void AppleInputSystem::update() {
    // No-op on Apple platforms - events are delivered asynchronously
    // via NSNotificationCenter. The helper handles everything.
}

void AppleInputSystem::addObserver(DeviceObserver *observer) {
    [(__bridge CIAppleInputHelper *)impl addObserver:observer];
#if TARGET_OS_IOS || TARGET_OS_TV
    if (_touch) observer->onDeviceConnected(*_touch);
#endif
}

void AppleInputSystem::removeObserver(DeviceObserver *observer) {
    [(__bridge CIAppleInputHelper *)impl removeObserver:observer];
}

int AppleInputSystem::gamepadCount() const {
    return [(__bridge CIAppleInputHelper *)impl gamepadCount];
}

GamepadDevice* AppleInputSystem::gamepadAt(int index) {
    return [(__bridge CIAppleInputHelper *)impl gamepadAt:index];
}

bool AppleInputSystem::hasKeyboard() const {
    return [(__bridge CIAppleInputHelper *)impl hasKeyboard];
}

KeyboardDevice* AppleInputSystem::keyboard() {
    return [(__bridge CIAppleInputHelper *)impl keyboard];
}

bool AppleInputSystem::hasMouse() const {
    return [(__bridge CIAppleInputHelper *)impl hasMouse];
}

MouseDevice* AppleInputSystem::mouse() {
    return [(__bridge CIAppleInputHelper *)impl mouse];
}

bool AppleInputSystem::hasTouch() const {
#if TARGET_OS_IOS || TARGET_OS_TV
    return true;
#else
    return false;
#endif
}

TouchDevice* AppleInputSystem::touch() {
#if TARGET_OS_IOS || TARGET_OS_TV
    return _touch.get();
#else
    return nullptr;
#endif
}

} // namespace systems::leal::campello_input
