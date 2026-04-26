#pragma once
#include <stdint.h>
#include <Arduino.h>

// Turn rate: radians per unit of input.x per second
constexpr float SENSITIVITY = 0.08f;

inline uint64_t getTime() {
    return micros();
}
