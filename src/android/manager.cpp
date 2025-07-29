#include <android/log.h>
#include <android/input.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <campello_input/manager.hpp>
#include "inc/campello_input/manager_android_struct.hpp"

using namespace systems::leal::campello_input;

Manager::Manager(void *pd) {
    this->native = pd;
    __android_log_print(ANDROID_LOG_DEBUG, "campello_input", "Manager::Manager()");
}

Manager::~Manager()
{
    //auto androidStruct = (ManagerAndroidStruct *)this->native;

    //Paddleboat_destroy((JNIEnv *)androidStruct->internalData);
    //androidStruct->vm->DetachCurrentThread();
    __android_log_print(ANDROID_LOG_DEBUG, "campello_input", "Manager::~Manager()");
}

void Manager::processNativeEvent(uint64_t cmd) {
    auto androidStruct = (ManagerAndroidStruct *)this->native;
    switch (cmd) {
        case MANAGER_ANDROID_CMD_START:
            //Paddleboat_onStart((JNIEnv *)androidStruct->internalData);
            break;
        case MANAGER_ANDROID_CMD_STOP:
            //Paddleboat_onStop((JNIEnv *)androidStruct->internalData);
            break;
    }
}

std::shared_ptr<Manager> Manager::init(void *pd) {
    return nullptr;

    /*__android_log_print(ANDROID_LOG_DEBUG, "campello_input", "Manager::init()");

    auto androidStruct = (ManagerAndroidStruct *)pd;

    JNIEnv *env;
    androidStruct->vm->AttachCurrentThread(&env, nullptr);
    androidStruct->internalData = env;

    auto status = Paddleboat_init(env, androidStruct->jcontext);
    if (status != PADDLEBOAT_NO_ERROR) {
        return nullptr;
    }*/

    return std::shared_ptr<Manager>(new Manager(pd));
}

void Manager::processData(void *pd) {

    auto *inputBuffer = android_app_swap_input_buffers((android_app *)pd);

    if (!inputBuffer) {
        // no inputs yet.
        return;
    }

    // handle motion events (motionEventsCounts can be 0).
    for (auto i = 0; i < inputBuffer->motionEventsCount; i++) {
        auto &motionEvent = inputBuffer->motionEvents[i];
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value.
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        __android_log_print(ANDROID_LOG_DEBUG, "campello_input", "Pointer(s)");
        //aout << "Pointer(s): ";

        // get the x and y position of this event if it is not ACTION_MOVE.
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);

        // determine the action type and process the event accordingly.
        switch (action & AMOTION_EVENT_ACTION_MASK) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                //aout << "(" << pointer.id << ", " << x << ", " << y << ") "<< "Pointer Down";
                break;

            case AMOTION_EVENT_ACTION_CANCEL:
                // treat the CANCEL as an UP event: doing nothing in the app, except
                // removing the pointer from the cache if pointers are locally saved.
                // code pass through on purpose.
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                //aout << "(" << pointer.id << ", " << x << ", " << y << ") " << "Pointer Up";
                break;

            case AMOTION_EVENT_ACTION_MOVE:
                // There is no pointer index for ACTION_MOVE, only a snapshot of
                // all active pointers; app needs to cache previous active pointers
                // to figure out which ones are actually moved.
                for (auto index = 0; index < motionEvent.pointerCount; index++) {
                    pointer = motionEvent.pointers[index];
                    x = GameActivityPointerAxes_getX(&pointer);
                    y = GameActivityPointerAxes_getY(&pointer);
                    //aout << "(" << pointer.id << ", " << x << ", " << y << ")";

                    //if (index != (motionEvent.pointerCount - 1)) aout << ",";
                    //aout << " ";
                }
                //aout << "Pointer Move";
                break;
            default:
                //aout << "Unknown MotionEvent Action: " << action;
        }
        //aout << std::endl;
    }
    // clear the motion input count in this buffer for main thread to re-use.
    android_app_clear_motion_events(inputBuffer);

    // handle input key events.
    for (auto i = 0; i < inputBuffer->keyEventsCount; i++) {
        auto &keyEvent = inputBuffer->keyEvents[i];
        //aout << "Key: " << keyEvent.keyCode <<" ";
        switch (keyEvent.action) {
            case AKEY_EVENT_ACTION_DOWN:
                //aout << "Key Down";
                break;
            case AKEY_EVENT_ACTION_UP:
                //aout << "Key Up";
                break;
            case AKEY_EVENT_ACTION_MULTIPLE:
                // Deprecated since Android API level 29.
                //aout << "Multiple Key Actions";
                break;
            default:
                //aout << "Unknown KeyEvent Action: " << keyEvent.action;
        }
        //aout << std::endl;
    }
    // clear the key input count too.
    android_app_clear_key_events(inputBuffer);

}

