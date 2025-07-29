#pragma once

#include <jni.h>

#define MANAGER_ANDROID_CMD_START 0x01
#define MANAGER_ANDROID_CMD_STOP  0x02

namespace systems::leal::campello_input {

    struct ManagerAndroidStruct {
        JavaVM *vm;
        jobject jcontext;
        void *internalData;
    };

}