#pragma once
#include "math.hpp"
#include "map.h"

struct Enemy {
    Vector2 pos;
    bool alive = true;

    Enemy(float x = 0, float y = 0) : pos(x, y) {}

    void update(const Vector2& playerPos, float dt) {
        if (!alive) return;
        Vector2 diff = {playerPos.x - pos.x, playerPos.y - pos.y};
        float dist = length(diff);
        if (dist > 0.8f && dist < 12.0f) {
            Vector2 d = normalized(diff);
            float newX = pos.x + d.x * dt * 0.5f;
            float newY = pos.y + d.y * dt * 0.5f;
            if (gameMap[(int)pos.y][(int)newX] == 0) pos.x = newX;
            if (gameMap[(int)newY][(int)pos.x] == 0) pos.y = newY;
        }
    }
};

constexpr int NUM_ENEMIES = 4;
static Enemy enemies[NUM_ENEMIES] = {
    {3.0f,  3.0f},
    {12.0f, 3.0f},
    {3.0f,  12.0f},
    {12.0f, 12.0f},
};
