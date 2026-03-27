// campello_input — Android Input Monitor example
// Shows gamepad axes/buttons, touch, and keyboard input in real-time using
// GameActivity + OpenGL ES, powered by InputSystem (unified API).

#include <jni.h>
#include <android/input.h>

#include "AndroidOut.h"
#include "InputRenderer.h"
#include "InputDisplayState.h"

#include <memory>  // for std::unique_ptr

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

#include <campello_input/input_system.hpp>

extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.c>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// App state
// ---------------------------------------------------------------------------

struct AppData {
    InputRenderer        *renderer = nullptr;
    std::unique_ptr<InputSystem> input;
};

// ---------------------------------------------------------------------------
// Event filter — allow touch and joystick motion events through
// ---------------------------------------------------------------------------

static bool motionEventFilter(const GameActivityMotionEvent *evt) {
    auto cls = evt->source & AINPUT_SOURCE_CLASS_MASK;
    return cls == AINPUT_SOURCE_CLASS_POINTER || cls == AINPUT_SOURCE_CLASS_JOYSTICK;
}

// ---------------------------------------------------------------------------
// App command handler
// ---------------------------------------------------------------------------

static void handleCmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW: {
            auto *data = new AppData();
            data->renderer = new InputRenderer(pApp);
            // Create InputSystem with Android context
            PlatformContext ctx{.native = pApp};
            data->input = InputSystem::create(ctx);
            pApp->userData = data;
            break;
        }
        case APP_CMD_TERM_WINDOW: {
            if (pApp->userData) {
                auto *data = static_cast<AppData *>(pApp->userData);
                pApp->userData = nullptr;
                delete data->renderer;
                delete data;
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

void android_main(android_app *pApp) {
    pApp->onAppCmd = handleCmd;
    android_app_set_motion_event_filter(pApp, motionEventFilter);
    
    // Create InputSystem with Android context
    PlatformContext ctx{.native = pApp};
    auto inputSystem = InputSystem::create(ctx);

    do {
        bool done = false;
        while (!done) {
            int events;
            android_poll_source *pSource;
            int result = ALooper_pollOnce(0, nullptr, &events,
                                          reinterpret_cast<void **>(&pSource));
            switch (result) {
                case ALOOPER_POLL_TIMEOUT:
                case ALOOPER_POLL_WAKE:
                    done = true;
                    break;
                case ALOOPER_EVENT_ERROR:
                    aout << "ALooper error" << std::endl;
                    break;
                default:
                    if (pSource) pSource->process(pApp, pSource);
            }
        }

        if (pApp->userData) {
            auto *data = static_cast<AppData *>(pApp->userData);

            // Update input - processes GameActivity input buffers
            data->input->update();

            // Build display state from InputSystem
            InputDisplayState display;

            int gpCount = data->input->gamepadCount();
            for (int i = 0; i < gpCount && i < InputDisplayState::kMaxGamepads; ++i) {
                auto *gp = data->input->gamepadAt(i);
                auto &entry      = display.gamepads[i];
                entry.active     = true;
                entry.deviceId   = i;  // Unified API doesn't expose Android device ID
                std::snprintf(entry.name, sizeof(entry.name), "%s", gp->name());
                gp->getState(entry.state);
                display.gamepadCount = i + 1;
            }

            // Get touch state from TouchDevice
            if (data->input->hasTouch()) {
                auto *touch = data->input->touch();
                if (touch) {
                    touch->getState(display.touch);
                }
            }

            data->renderer->render(display);
        }
    } while (!pApp->destroyRequested);
}

} // extern "C"
