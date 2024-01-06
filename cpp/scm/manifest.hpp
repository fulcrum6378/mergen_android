#ifndef SCM_MANIFEST_H
#define SCM_MANIFEST_H

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
                1, ActType::VIS, .7f, .75f, -1.0f, 30u, 80u, 6u
        };
        interactions[2] = Interaction{ // selfie camera
                2, ActType::VIS, 0.0f, .9f, 1.0f, 6u, 6u, 3u
        };
        interactions[3] = Interaction{ // microphone
                3, ActType::AUD, .4f, 1.0f, 0.0f, 2u, 4u, 2u
        };
        interactions[4] = Interaction{ // touchscreen
                4, ActType::HPT, 0.0f, 0.0f, 1.0f, 1080u, 2340u, 2u
        };

        interactions[-1] = Interaction{ // monitor screen
                -1, ActType::VIS, 0.0f, 0.0f, 1.0f, 1080u, 2131u, 2u
        };
        interactions[-2] = Interaction{ // speaker
                -2, ActType::AUD, .55f, 1.0f, 0.0f, 10u, 4u, 5u
        };
        interactions[-3] = Interaction{ // vibration motor (inaccurate)
                -3, ActType::MOV, -.5f, -.75f, 0.0f, 15u, 15u, 5u
        };
    }
};

#endif //SCM_MANIFEST_H
