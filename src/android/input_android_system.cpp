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

#include <android/input.h>
#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

#include "inc/campello_input/input_android_system.hpp"
#include "inc/campello_input/input_android.hpp"  // For AndroidGamepad
#include "inc/campello_input/touch_android.hpp"
#include "inc/campello_input/keyboard_android.hpp"
#include "inc/campello_input/mouse_android.hpp"
#include "inc/campello_input/haptics_android.hpp"
#include <campello_input/mouse.hpp>  // For MouseButton

#include <algorithm>
#include <cstring>
#include <deque>
#include <map>
#include <memory>
#include <vector>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Key-code helpers
// ---------------------------------------------------------------------------

static uint64_t gamepadButtonFromKeyCode(int32_t keyCode) {
    switch (keyCode) {
        case AKEYCODE_BUTTON_A:      return (uint64_t)GamepadButton::face_south;
        case AKEYCODE_BUTTON_B:      return (uint64_t)GamepadButton::face_east;
        case AKEYCODE_BUTTON_X:      return (uint64_t)GamepadButton::face_west;
        case AKEYCODE_BUTTON_Y:      return (uint64_t)GamepadButton::face_north;
        case AKEYCODE_BUTTON_L1:     return (uint64_t)GamepadButton::shoulder_left;
        case AKEYCODE_BUTTON_R1:     return (uint64_t)GamepadButton::shoulder_right;
        case AKEYCODE_BUTTON_THUMBL: return (uint64_t)GamepadButton::thumb_left;
        case AKEYCODE_BUTTON_THUMBR: return (uint64_t)GamepadButton::thumb_right;
        case AKEYCODE_DPAD_UP:       return (uint64_t)GamepadButton::dpad_up;
        case AKEYCODE_DPAD_DOWN:     return (uint64_t)GamepadButton::dpad_down;
        case AKEYCODE_DPAD_LEFT:     return (uint64_t)GamepadButton::dpad_left;
        case AKEYCODE_DPAD_RIGHT:    return (uint64_t)GamepadButton::dpad_right;
        case AKEYCODE_BUTTON_START:  return (uint64_t)GamepadButton::start;
        case AKEYCODE_BUTTON_SELECT: return (uint64_t)GamepadButton::select;
        case AKEYCODE_BUTTON_MODE:   return (uint64_t)GamepadButton::home;
        case AKEYCODE_BUTTON_C:      return (uint64_t)GamepadButton::misc1;
        default:                     return 0;
    }
}

// ---------------------------------------------------------------------------
// AndroidGamepad implementation (from input_android.cpp)
// ---------------------------------------------------------------------------

AndroidGamepad::AndroidGamepad(uint32_t id, int32_t androidDeviceId, const char *name)
    : _id(id), _androidDeviceId(androidDeviceId) {
    std::snprintf(_name, sizeof(_name), "%s", name ? name : "Gamepad");
}

void AndroidGamepad::updateState(const GamepadState &s) noexcept {
    _state = s;
}

DeviceType AndroidGamepad::type() const noexcept {
    return DeviceType::gamepad;
}

uint32_t AndroidGamepad::id() const noexcept {
    return _id;
}

const char* AndroidGamepad::name() const noexcept {
    return _name;
}

ConnectionStatus AndroidGamepad::connectionStatus() const noexcept {
    return ConnectionStatus::connected;
}

bool AndroidGamepad::getState(GamepadState &outState) const {
    outState = _state;
    return true;
}

Haptics* AndroidGamepad::haptics() noexcept {
    return nullptr;
}

// ---------------------------------------------------------------------------
// AndroidInputSystem::Impl
// ---------------------------------------------------------------------------

struct AndroidInputSystem::Impl {
    std::vector<std::unique_ptr<AndroidGamepad>> gamepads;
    std::map<int32_t, int>                       deviceIdToIndex;
    std::vector<DeviceObserver *>                observers;
    uint32_t                                     nextId = 0;
    TouchState                                   touchState{};
    std::deque<std::string>                      recentKeys;
    
    // Keyboard and mouse devices
    std::unique_ptr<AndroidKeyboard>             keyboard;
    std::unique_ptr<AndroidMouse>                mouse;
    bool                                         keyboardConnected = false;
    bool                                         mouseConnected = false;

    AndroidGamepad* findOrAddGamepad(int32_t deviceId, std::vector<DeviceObserver*>& obs) {
        auto it = deviceIdToIndex.find(deviceId);
        if (it != deviceIdToIndex.end()) {
            return gamepads[it->second].get();
        }
        char name[64];
        std::snprintf(name, sizeof(name), "Gamepad %d", deviceId);
        auto gp = std::make_unique<AndroidGamepad>(nextId++, deviceId, name);
        AndroidGamepad *ptr = gp.get();
        int idx = (int)gamepads.size();
        gamepads.push_back(std::move(gp));
        deviceIdToIndex[deviceId] = idx;
        for (auto *o : obs) o->onDeviceConnected(*ptr);
        return ptr;
    }
    
    void ensureKeyboard(std::vector<DeviceObserver*>& obs) {
        if (!keyboard) {
            keyboard = std::make_unique<AndroidKeyboard>(nextId++);
            keyboardConnected = true;
            for (auto *o : obs) o->onDeviceConnected(*keyboard);
        }
    }
    
    void ensureMouse(std::vector<DeviceObserver*>& obs) {
        if (!mouse) {
            mouse = std::make_unique<AndroidMouse>(nextId++);
            mouseConnected = true;
            for (auto *o : obs) o->onDeviceConnected(*mouse);
        }
    }
};

// ---------------------------------------------------------------------------
// AndroidInputSystem
// ---------------------------------------------------------------------------

AndroidInputSystem::AndroidInputSystem(const PlatformContext& ctx)
    : pImpl(new Impl()),
      app(static_cast<android_app*>(ctx.native)),
      touchDevice(std::make_unique<AndroidTouch>())
{
    if (!app) {
        __android_log_print(ANDROID_LOG_WARN, "campello_input",
            "AndroidInputSystem created without android_app pointer. "
            "Make sure to pass PlatformContext{.native = android_app_ptr}");
    }
    
    // Initialize keyboard and mouse (lazy creation on first input event)
    // They will be reported to observers when first used
}

AndroidInputSystem::~AndroidInputSystem() {
    delete pImpl;
}

void AndroidInputSystem::update() {
    if (!app) return;

    auto *buf = android_app_swap_input_buffers(app);
    if (!buf) return;

    // --- Motion events ---
    for (int i = 0; i < buf->motionEventsCount; ++i) {
        auto &evt    = buf->motionEvents[i];
        int32_t action = evt.action & AMOTION_EVENT_ACTION_MASK;
        int32_t src    = evt.source;

        if (src & AINPUT_SOURCE_JOYSTICK) {
            // Gamepad axes and HAT
            AndroidGamepad *gp = pImpl->findOrAddGamepad(evt.deviceId, pImpl->observers);
            if (gp && evt.pointerCount > 0) {
                GamepadState s{};
                gp->getState(s);

                const auto &ptr = evt.pointers[0];
                auto axv = [&](int32_t axis) {
                    return GameActivityPointerAxes_getAxisValue(&ptr, axis);
                };

                s.axes[(int)GamepadAxis::left_x ].value = axv(AMOTION_EVENT_AXIS_X);
                s.axes[(int)GamepadAxis::left_y ].value = axv(AMOTION_EVENT_AXIS_Y);
                s.axes[(int)GamepadAxis::right_x].value = axv(AMOTION_EVENT_AXIS_Z);
                s.axes[(int)GamepadAxis::right_y].value = axv(AMOTION_EVENT_AXIS_RZ);

                float lt = axv(AMOTION_EVENT_AXIS_BRAKE);
                float rt = axv(AMOTION_EVENT_AXIS_GAS);
                if (lt == 0.f) lt = axv(AMOTION_EVENT_AXIS_LTRIGGER);
                if (rt == 0.f) rt = axv(AMOTION_EVENT_AXIS_RTRIGGER);
                s.axes[(int)GamepadAxis::trigger_left ].value = std::max(0.f, std::min(1.f, lt));
                s.axes[(int)GamepadAxis::trigger_right].value = std::max(0.f, std::min(1.f, rt));

                // HAT → dpad buttons
                float hatX = axv(AMOTION_EVENT_AXIS_HAT_X);
                float hatY = axv(AMOTION_EVENT_AXIS_HAT_Y);
                auto setBtn = [&](GamepadButton b, bool v) {
                    if (v) s.buttons |=  (uint64_t)b;
                    else   s.buttons &= ~(uint64_t)b;
                };
                setBtn(GamepadButton::dpad_left,  hatX < -0.5f);
                setBtn(GamepadButton::dpad_right, hatX >  0.5f);
                setBtn(GamepadButton::dpad_up,    hatY < -0.5f);
                setBtn(GamepadButton::dpad_down,  hatY >  0.5f);

                s.timestamp = (uint64_t)(evt.eventTime / 1000);
                gp->updateState(s);
            }

        } else if (src & AINPUT_SOURCE_MOUSE) {
            // Mouse events (trackpad, external mouse, etc.)
            pImpl->ensureMouse(pImpl->observers);
            if (pImpl->mouse && evt.pointerCount > 0) {
                const auto &ptr = evt.pointers[0];
                
                float x = GameActivityPointerAxes_getX(&ptr);
                float y = GameActivityPointerAxes_getY(&ptr);
                float deltaX = GameActivityPointerAxes_getAxisValue(&ptr, AMOTION_EVENT_AXIS_RELATIVE_X);
                float deltaY = GameActivityPointerAxes_getAxisValue(&ptr, AMOTION_EVENT_AXIS_RELATIVE_Y);
                
                // If no relative values, calculate from position change
                if (deltaX == 0.0f && deltaY == 0.0f && action == AMOTION_EVENT_ACTION_MOVE) {
                    static float lastX = 0.0f, lastY = 0.0f;
                    deltaX = x - lastX;
                    deltaY = y - lastY;
                    lastX = x;
                    lastY = y;
                }
                
                pImpl->mouse->onMotionEvent(deltaX, deltaY, x, y);
                
                // Handle mouse button state from action
                if (action == AMOTION_EVENT_ACTION_DOWN) {
                    pImpl->mouse->onButtonEvent(MouseButton::left, true);
                } else if (action == AMOTION_EVENT_ACTION_UP) {
                    pImpl->mouse->onButtonEvent(MouseButton::left, false);
                }
                
                // Check for scroll
                float scrollX = GameActivityPointerAxes_getAxisValue(&ptr, AMOTION_EVENT_AXIS_HSCROLL);
                float scrollY = GameActivityPointerAxes_getAxisValue(&ptr, AMOTION_EVENT_AXIS_VSCROLL);
                if (scrollX != 0.0f || scrollY != 0.0f) {
                    pImpl->mouse->onScrollEvent(scrollX, scrollY);
                }
            }
        } else if ((src & AINPUT_SOURCE_CLASS_MASK) == AINPUT_SOURCE_CLASS_POINTER) {
            // Touch events
            auto &touch = pImpl->touchState;

            if (action == AMOTION_EVENT_ACTION_DOWN ||
                action == AMOTION_EVENT_ACTION_POINTER_DOWN) {

                int pIdx = (evt.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                           >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                if (touch.count < kMaxTouchPoints) {
                    auto &pt      = touch.points[touch.count++];
                    pt.id         = (uint32_t)evt.pointers[pIdx].id;
                    pt.phase      = TouchPhase::began;
                    pt.x          = GameActivityPointerAxes_getX(&evt.pointers[pIdx]);
                    pt.y          = GameActivityPointerAxes_getY(&evt.pointers[pIdx]);
                    pt.pressure   = GameActivityPointerAxes_getAxisValue(
                                        &evt.pointers[pIdx], AMOTION_EVENT_AXIS_PRESSURE);
                    pt.timestamp  = (uint64_t)(evt.eventTime / 1000);
                }

            } else if (action == AMOTION_EVENT_ACTION_UP ||
                       action == AMOTION_EVENT_ACTION_POINTER_UP ||
                       action == AMOTION_EVENT_ACTION_CANCEL) {

                int pIdx = (evt.action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                           >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                uint32_t removeId = (uint32_t)evt.pointers[pIdx].id;
                for (uint32_t t = 0; t < touch.count; ++t) {
                    if (touch.points[t].id == removeId) {
                        touch.points[t] = touch.points[--touch.count];
                        break;
                    }
                }

            } else if (action == AMOTION_EVENT_ACTION_MOVE) {
                for (int p = 0; p < evt.pointerCount; ++p) {
                    uint32_t tid = (uint32_t)evt.pointers[p].id;
                    for (uint32_t t = 0; t < touch.count; ++t) {
                        if (touch.points[t].id == tid) {
                            touch.points[t].phase    = TouchPhase::moved;
                            touch.points[t].x        = GameActivityPointerAxes_getX(&evt.pointers[p]);
                            touch.points[t].y        = GameActivityPointerAxes_getY(&evt.pointers[p]);
                            touch.points[t].pressure = GameActivityPointerAxes_getAxisValue(
                                                           &evt.pointers[p], AMOTION_EVENT_AXIS_PRESSURE);
                            touch.points[t].timestamp = (uint64_t)(evt.eventTime / 1000);
                            break;
                        }
                    }
                }
            }
        }
    }
    android_app_clear_motion_events(buf);

    // Update touch device state
    if (touchDevice) {
        touchDevice->updateState(pImpl->touchState);
    }

    // --- Key events ---
    for (int i = 0; i < buf->keyEventsCount; ++i) {
        auto &evt    = buf->keyEvents[i];
        bool pressed = (evt.action == AKEY_EVENT_ACTION_DOWN);
        int32_t kc   = evt.keyCode;
        int32_t src  = evt.source;

        uint64_t gpBtn = gamepadButtonFromKeyCode(kc);
        if (gpBtn != 0) {
            AndroidGamepad *gp = pImpl->findOrAddGamepad(evt.deviceId, pImpl->observers);
            if (gp) {
                GamepadState s{};
                gp->getState(s);
                if (pressed) s.buttons |=  gpBtn;
                else         s.buttons &= ~gpBtn;
                gp->updateState(s);
            }
        }
        
        // Process keyboard events from keyboard or unknown sources
        if ((src & AINPUT_SOURCE_KEYBOARD) || !(src & AINPUT_SOURCE_GAMEPAD)) {
            pImpl->ensureKeyboard(pImpl->observers);
            if (pImpl->keyboard) {
                pImpl->keyboard->onKeyEvent(kc, pressed);
            }
        }
    }
    android_app_clear_key_events(buf);
    
    // Reset mouse deltas at end of frame
    if (pImpl->mouse) {
        pImpl->mouse->resetAccumulators();
    }
}

void AndroidInputSystem::addObserver(DeviceObserver *observer) {
    pImpl->observers.push_back(observer);
    for (auto &gp : pImpl->gamepads) {
        observer->onDeviceConnected(*gp);
    }
    if (touchDevice) {
        observer->onDeviceConnected(*touchDevice);
    }
}

void AndroidInputSystem::removeObserver(DeviceObserver *observer) {
    auto &obs = pImpl->observers;
    auto it = std::find(obs.begin(), obs.end(), observer);
    if (it != obs.end()) obs.erase(it);
}

int AndroidInputSystem::gamepadCount() const {
    return (int)pImpl->gamepads.size();
}

GamepadDevice* AndroidInputSystem::gamepadAt(int index) {
    if (index < 0 || index >= (int)pImpl->gamepads.size()) return nullptr;
    return pImpl->gamepads[index].get();
}

bool AndroidInputSystem::hasKeyboard() const {
    return pImpl->keyboard != nullptr;
}

KeyboardDevice* AndroidInputSystem::keyboard() {
    return pImpl->keyboard.get();
}

bool AndroidInputSystem::hasMouse() const {
    return pImpl->mouse != nullptr;
}

MouseDevice* AndroidInputSystem::mouse() {
    return pImpl->mouse.get();
}

bool AndroidInputSystem::hasTouch() const {
    return true;  // Android always has touch
}

TouchDevice* AndroidInputSystem::touch() {
    return touchDevice.get();
}

// Legacy API implementations
void AndroidInputSystem::processInputBuffers(android_app *pApp) {
    // Store app pointer temporarily and call update
    android_app* savedApp = app;
    app = pApp;
    update();
    app = savedApp;
}

const TouchState& AndroidInputSystem::touchState() const noexcept {
    return pImpl->touchState;
}

const std::deque<std::string>& AndroidInputSystem::recentKeys() const noexcept {
    return pImpl->recentKeys;
}
