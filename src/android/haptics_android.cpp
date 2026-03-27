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

#include "inc/campello_input/haptics_android.hpp"
#include <android/log.h>
#include <cstring>

using namespace systems::leal::campello_input;

AndroidHaptics::AndroidHaptics() = default;

AndroidHaptics::~AndroidHaptics() {
    cleanup();
}

void AndroidHaptics::cleanup() {
    if (_activity && _activity->vm && _vibrator) {
        JNIEnv* env = nullptr;
        if (_activity->vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            env->DeleteGlobalRef(_vibrator);
            if (_vibrationEffectClass) {
                env->DeleteGlobalRef(_vibrationEffectClass);
            }
        }
        _activity->vm->DetachCurrentThread();
    }
    _vibrator = nullptr;
    _vibrationEffectClass = nullptr;
    _initialized = false;
}

bool AndroidHaptics::initialize(ANativeActivity* activity) {
    if (_initialized) {
        return true;
    }
    
    if (!activity || !activity->vm || !activity->clazz) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Invalid activity");
        return false;
    }
    
    _activity = activity;
    
    JNIEnv* env = nullptr;
    if (activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to attach thread");
        return false;
    }
    
    // Get the activity class
    jclass activityClass = env->GetObjectClass(activity->clazz);
    if (!activityClass) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to get activity class");
        activity->vm->DetachCurrentThread();
        return false;
    }
    
    // Get getSystemService method
    jmethodID getSystemService = env->GetMethodID(activityClass, "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;");
    if (!getSystemService) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to get getSystemService method");
        activity->vm->DetachCurrentThread();
        return false;
    }
    
    // Get VIBRATOR_SERVICE constant
    jfieldID vibratorServiceField = env->GetStaticFieldID(activityClass, "VIBRATOR_SERVICE",
        "Ljava/lang/String;");
    if (!vibratorServiceField) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to get VIBRATOR_SERVICE field");
        activity->vm->DetachCurrentThread();
        return false;
    }
    
    jobject vibratorServiceString = env->GetStaticObjectField(activityClass, vibratorServiceField);
    if (!vibratorServiceString) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to get VIBRATOR_SERVICE string");
        activity->vm->DetachCurrentThread();
        return false;
    }
    
    // Get vibrator service
    jobject vibrator = env->CallObjectMethod(activity->clazz, getSystemService, vibratorServiceString);
    env->DeleteLocalRef(vibratorServiceString);
    
    if (!vibrator) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to get vibrator service");
        activity->vm->DetachCurrentThread();
        return false;
    }
    
    // Store global reference to vibrator
    _vibrator = env->NewGlobalRef(vibrator);
    env->DeleteLocalRef(vibrator);
    
    // Get Vibrator class and methods
    jclass vibratorClass = env->GetObjectClass(_vibrator);
    if (!vibratorClass) {
        __android_log_print(ANDROID_LOG_ERROR, "campello_input",
            "AndroidHaptics::initialize: Failed to get Vibrator class");
        cleanup();
        activity->vm->DetachCurrentThread();
        return false;
    }
    
    _hasVibratorMethod = env->GetMethodID(vibratorClass, "hasVibrator", "()Z");
    _vibrateMethod = env->GetMethodID(vibratorClass, "vibrate", "(J)V");
    
    // Check for API 26+ VibrationEffect
    jclass vibrationEffectClass = env->FindClass("android/os/VibrationEffect");
    if (vibrationEffectClass) {
        _vibrationEffectClass = reinterpret_cast<jclass>(env->NewGlobalRef(vibrationEffectClass));
        _createOneShotMethod = env->GetStaticMethodID(_vibrationEffectClass, "createOneShot",
            "(JI)Landroid/os/VibrationEffect;");
        _hasAmplitudeControlMethod = env->GetMethodID(vibratorClass, "hasAmplitudeControl", "()Z");
        _hasVibrationEffect = true;
        env->DeleteLocalRef(vibrationEffectClass);
    }
    
    env->DeleteLocalRef(vibratorClass);
    env->DeleteLocalRef(activityClass);
    
    activity->vm->DetachCurrentThread();
    
    // Check capabilities
    checkCapabilities();
    
    _initialized = true;
    
    __android_log_print(ANDROID_LOG_INFO, "campello_input",
        "AndroidHaptics initialized. Capabilities: %d", static_cast<int>(_capabilities));
    
    return true;
}

bool AndroidHaptics::checkCapabilities() {
    if (!_activity || !_activity->vm || !_vibrator) {
        return false;
    }
    
    JNIEnv* env = nullptr;
    if (_activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        return false;
    }
    
    _capabilities = HapticsCapability::none;
    
    // Check if device has vibrator
    if (_hasVibratorMethod) {
        jboolean hasVibrator = env->CallBooleanMethod(_vibrator, _hasVibratorMethod);
        if (hasVibrator) {
            _capabilities = _capabilities | HapticsCapability::rumble;
        }
    }
    
    // Check for amplitude control (API 26+)
    if (_hasAmplitudeControlMethod && _hasVibrationEffect) {
        jboolean hasAmplitude = env->CallBooleanMethod(_vibrator, _hasAmplitudeControlMethod);
        if (hasAmplitude) {
            _capabilities = _capabilities | HapticsCapability::rumble_high_freq;
        }
    }
    
    // Android doesn't typically support per-trigger haptics like game controllers
    // Some devices might support it via haptic generators, but that's not standard
    
    _activity->vm->DetachCurrentThread();
    return true;
}

bool AndroidHaptics::supports(HapticsCapability capability) const noexcept {
    return (static_cast<int>(_capabilities) & static_cast<int>(capability)) != 0;
}

bool AndroidHaptics::playRumble(float lowFreqIntensity, float highFreqIntensity, uint32_t durationMs) {
    if (!_initialized || !_vibrator) {
        return false;
    }
    
    // Use the higher intensity for the vibration
    float intensity = std::max(lowFreqIntensity, highFreqIntensity);
    if (intensity <= 0.0f) {
        return true;  // Nothing to play
    }
    
    JNIEnv* env = nullptr;
    if (_activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        return false;
    }
    
    bool result = false;
    
    if (_hasVibrationEffect && _createOneShotMethod) {
        // Use VibrationEffect for better control on API 26+
        // Amplitude is 1-255
        int amplitude = static_cast<int>(intensity * 255.0f);
        if (amplitude < 1) amplitude = 1;
        if (amplitude > 255) amplitude = 255;
        
        jobject effect = env->CallStaticObjectMethod(_vibrationEffectClass, _createOneShotMethod,
            static_cast<jlong>(durationMs), static_cast<jint>(amplitude));
        
        if (effect) {
            // Call vibrate(VibrationEffect) method
            jclass vibratorClass = env->GetObjectClass(_vibrator);
            jmethodID vibrateEffectMethod = env->GetMethodID(vibratorClass, "vibrate",
                "(Landroid/os/VibrationEffect;)V");
            if (vibrateEffectMethod) {
                env->CallVoidMethod(_vibrator, vibrateEffectMethod, effect);
                result = true;
            }
            env->DeleteLocalRef(effect);
            env->DeleteLocalRef(vibratorClass);
        }
    } else if (_vibrateMethod) {
        // Fallback to simple vibration on older devices
        jlong duration = static_cast<jlong>(durationMs);
        env->CallVoidMethod(_vibrator, _vibrateMethod, duration);
        result = true;
    }
    
    _activity->vm->DetachCurrentThread();
    return result;
}

bool AndroidHaptics::stopRumble() {
    if (!_initialized || !_vibrator) {
        return false;
    }
    
    JNIEnv* env = nullptr;
    if (_activity->vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        return false;
    }
    
    // Call cancel() method
    jclass vibratorClass = env->GetObjectClass(_vibrator);
    jmethodID cancelMethod = env->GetMethodID(vibratorClass, "cancel", "()V");
    if (cancelMethod) {
        env->CallVoidMethod(_vibrator, cancelMethod);
    }
    
    env->DeleteLocalRef(vibratorClass);
    _activity->vm->DetachCurrentThread();
    
    return true;
}

bool AndroidHaptics::playTriggerEffect(TriggerEffect effect, TriggerIntensity intensity,
                                       uint32_t durationMs) {
    // Android doesn't have native per-trigger haptics support
    // We could map this to regular rumble as a fallback
    (void)effect;
    
    float intensityValue = 0.0f;
    switch (intensity) {
        case TriggerIntensity::light:  intensityValue = 0.3f; break;
        case TriggerIntensity::medium: intensityValue = 0.6f; break;
        case TriggerIntensity::heavy:  intensityValue = 1.0f; break;
    }
    
    return playRumble(intensityValue, intensityValue, durationMs);
}

bool AndroidHaptics::stopTriggerEffect(bool isLeftTrigger) {
    // Just stop all rumble since Android doesn't support per-trigger haptics
    (void)isLeftTrigger;
    return stopRumble();
}
