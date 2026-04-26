#pragma once
#include <math.h>
#include <string.h>

constexpr float M_2PI = 2.0f * M_PI;

struct Vector2 {
    float x = 0, y = 0;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
    Vector2 operator*(float s) const { return {x * s, y * s}; }
    Vector2 operator+(const Vector2& o) const { return {x + o.x, y + o.y}; }
    Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
};

inline Vector2 rotated90Deg(const Vector2& v)      { return {-v.y, v.x}; }
inline Vector2 rotatedMinus90Deg(const Vector2& v) { return {v.y, -v.x}; }
inline Vector2 invert(const Vector2& v)             { return {-v.x, -v.y}; }
inline float dot(const Vector2& a, const Vector2& b) { return a.x*b.x + a.y*b.y; }
inline float length(const Vector2& v)               { return sqrtf(v.x*v.x + v.y*v.y); }
inline Vector2 normalized(const Vector2& v) {
    float l = length(v);
    return l > 0 ? Vector2(v.x / l, v.y / l) : Vector2(0, 0);
}
