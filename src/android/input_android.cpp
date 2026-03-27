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
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <game-activity/GameActivity.h>

#include "inc/campello_input/input_android.hpp"

#include <algorithm>
#include <cstring>
#include <deque>
#include <map>
#include <memory>
#include <vector>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Key-code helpers (moved from example into the library)
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

static const char* keyCodeToName(int32_t keyCode) {
    if (keyCode >= AKEYCODE_A && keyCode <= AKEYCODE_Z) {
        static char s[2]; s[0] = 'A' + (keyCode - AKEYCODE_A); s[1] = 0; return s;
    }
    if (keyCode >= AKEYCODE_0 && keyCode <= AKEYCODE_9) {
        static char s[2]; s[0] = '0' + (keyCode - AKEYCODE_0); s[1] = 0; return s;
    }
    switch (keyCode) {
        case AKEYCODE_ENTER:         return "Enter";
        case AKEYCODE_ESCAPE:        return "Esc";
        case AKEYCODE_DEL:           return "Bksp";
        case AKEYCODE_TAB:           return "Tab";
        case AKEYCODE_SPACE:         return "Space";
        case AKEYCODE_SHIFT_LEFT:
        case AKEYCODE_SHIFT_RIGHT:   return "Shift";
        case AKEYCODE_CTRL_LEFT:
        case AKEYCODE_CTRL_RIGHT:    return "Ctrl";
        case AKEYCODE_ALT_LEFT:
        case AKEYCODE_ALT_RIGHT:     return "Alt";
        case AKEYCODE_CAPS_LOCK:     return "Caps";
        case AKEYCODE_F1:            return "F1";
        case AKEYCODE_F2:            return "F2";
        case AKEYCODE_F3:            return "F3";
        case AKEYCODE_F4:            return "F4";
        case AKEYCODE_F5:            return "F5";
        case AKEYCODE_F6:            return "F6";
        case AKEYCODE_F7:            return "F7";
        case AKEYCODE_F8:            return "F8";
        case AKEYCODE_F9:            return "F9";
        case AKEYCODE_F10:           return "F10";
        case AKEYCODE_F11:           return "F11";
        case AKEYCODE_F12:           return "F12";
        case AKEYCODE_MINUS:         return "-";
        case AKEYCODE_EQUALS:        return "=";
        case AKEYCODE_SLASH:         return "/";
        case AKEYCODE_BACKSLASH:     return "\\";
        case AKEYCODE_PERIOD:        return ".";
        case AKEYCODE_COMMA:         return ",";
        case AKEYCODE_SEMICOLON:     return ";";
        case AKEYCODE_APOSTROPHE:    return "'";
        case AKEYCODE_GRAVE:         return "`";
        case AKEYCODE_LEFT_BRACKET:  return "[";
        case AKEYCODE_RIGHT_BRACKET: return "]";
        default:                     return nullptr;
    }
}

// ---------------------------------------------------------------------------
// AndroidGamepad
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
// AndroidInput::Impl
// ---------------------------------------------------------------------------

struct AndroidInput::Impl {
    std::vector<std::unique_ptr<AndroidGamepad>> gamepads;
    std::map<int32_t, int>                       deviceIdToIndex;
    std::vector<DeviceObserver *>                observers;
    uint32_t                                     nextId = 0;
    TouchState                                   touch{};
    std::deque<std::string>                      recentKeys;

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
};

// ---------------------------------------------------------------------------
// AndroidInput
// ---------------------------------------------------------------------------

AndroidInput::AndroidInput() : pImpl(new Impl()) {}
AndroidInput::~AndroidInput() { delete pImpl; }

void AndroidInput::addObserver(DeviceObserver *observer) {
    pImpl->observers.push_back(observer);
    for (auto &gp : pImpl->gamepads) {
        observer->onDeviceConnected(*gp);
    }
}

void AndroidInput::removeObserver(DeviceObserver *observer) {
    auto &obs = pImpl->observers;
    obs.erase(std::find(obs.begin(), obs.end(), observer));
}

int AndroidInput::gamepadCount() const noexcept {
    return (int)pImpl->gamepads.size();
}

AndroidGamepad* AndroidInput::gamepadAt(int index) const noexcept {
    if (index < 0 || index >= (int)pImpl->gamepads.size()) return nullptr;
    return pImpl->gamepads[index].get();
}

AndroidGamepad* AndroidInput::findGamepad(int32_t androidId) const noexcept {
    auto it = pImpl->deviceIdToIndex.find(androidId);
    if (it == pImpl->deviceIdToIndex.end()) return nullptr;
    return pImpl->gamepads[it->second].get();
}

const TouchState& AndroidInput::touchState() const noexcept {
    return pImpl->touch;
}

const std::deque<std::string>& AndroidInput::recentKeys() const noexcept {
    return pImpl->recentKeys;
}

void AndroidInput::processInputBuffers(android_app *pApp) {
    auto *buf = android_app_swap_input_buffers(pApp);
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

        } else if ((src & AINPUT_SOURCE_CLASS_MASK) == AINPUT_SOURCE_CLASS_POINTER) {
            // Touch events
            auto &touch = pImpl->touch;

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

            } else if (action == AMOTION_EVENT_ACTION_UP    ||
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

    // --- Key events ---
    for (int i = 0; i < buf->keyEventsCount; ++i) {
        auto &evt    = buf->keyEvents[i];
        bool pressed = (evt.action == AKEY_EVENT_ACTION_DOWN);
        int32_t kc   = evt.keyCode;

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
        } else if (pressed) {
            const char *name = keyCodeToName(kc);
            if (name) {
                pImpl->recentKeys.push_front(name);
                if (pImpl->recentKeys.size() > 10) pImpl->recentKeys.pop_back();
            }
        }
    }
    android_app_clear_key_events(buf);
}
