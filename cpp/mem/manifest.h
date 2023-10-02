#ifndef MEM_MANIFEST_H
#define MEM_MANIFEST_H

#include <unordered_map>

enum class ActType : uint8_t {
    AUD, HPT, MOV, VIS
};

struct Interaction {
    /** Input/sense IDs must be positive and Output/{control/expression} IDs must be negative. */
    int8_t id;
    [[maybe_unused]] ActType type;
    /** Range -1..1 ; 0 means centre */
    [[maybe_unused]] float x, y, z;
    /** In pixels */
    [[maybe_unused]] uint16_t width, height, depth;
};

/*
 * Information defined here are of my Galaxy A50 phone.
 * More info: https://www.gsmarena.com/samsung_galaxy_a50-9554.php
 * GPU of my phone is Arm Mali-G72 and supports Vulkan 1.0.
 * More info: https://developer.arm.com/Processors/Mali-G72
 */
class Manifest {
public:
    // Sensors, Controls and Expressions
    inline static std::unordered_map<int8_t, Interaction> interactions;

    // Interaction ID `0` is reserved for Normality.
    static void init() {
        interactions[1] = Interaction{ // back camera
                1, ActType::VIS, .7, .75, -1, 30, 80, 6
        };
        interactions[2] = Interaction{ // selfie camera
                2, ActType::VIS, 0, .9, 1, 6, 6, 3
        };
        interactions[3] = Interaction{ // microphone
                3, ActType::AUD, .4, 1, 0, 2, 4, 2
        };
        interactions[4] = Interaction{ // touchscreen
                4, ActType::HPT, 0, 0, 1, 1080, 2340, 2
        };

        interactions[-1] = Interaction{ // monitor screen
                -1, ActType::VIS, 0, 0, 1, 1080, 2131, 2
        };
        interactions[-2] = Interaction{ // speaker
                -2, ActType::AUD, .55, 1, 0, 10, 4, 5
        };
        interactions[-3] = Interaction{ // vibration motor (inaccurate)
                -3, ActType::MOV, -.5, -.75, 0, 15, 15, 5
        };
    }
};

#endif //MEM_MANIFEST_H
