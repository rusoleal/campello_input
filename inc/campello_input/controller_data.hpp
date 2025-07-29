#pragma once

namespace systems::leal::campello_input {

    struct AxisData {
        float value;
        float min;
        float max;
    };

    struct ControllerData {

        uint64_t timestamp;
        uint64_t buttons;
        AxisData axis[32];
    };

}