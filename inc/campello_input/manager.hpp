#pragma once

#include <memory>
#include <campello_input/controller_data.hpp>

namespace systems::leal::campello_input {

    enum class ControllerStatus {
        connected,
        disconnected
    };

    typedef void (*ControllerStatusCallback)(const int32_t controllerIndex, const ControllerStatus status);

    class Manager {
    private:
        void *native;

        ControllerStatusCallback controllersCallback;
        bool controllers[16];
        ControllerData controllerData[16];

        Manager(void *pd);
    public:

        ~Manager();
        
        void processNativeEvent(uint64_t cmd);

        void processData(void *pd);

        void setControllerStatusCallback(ControllerStatusCallback callback) {
            controllersCallback = callback;
        }

        bool getControllerData(const uint32_t index, ControllerData *data) {
            if (controllers[index]) {
                *data = controllerData[index];
                return true;
            }
            return false;
        }

        static std::shared_ptr<Manager> init(void *pd);

    };

}