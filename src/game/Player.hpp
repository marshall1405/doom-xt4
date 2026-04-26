#pragma once
#include "math.hpp"
#include "InputData.hpp"
#include "map.h"
#include "system.h"

class Player {
    float _yaw = 0;
    float _vel = 3.0f;
public:
    Vector2 pos;
    Vector2 dir;
    float r;
    int health = 100;
    int ammo   = 50;

    Player(float x = 8.0f, float y = 8.0f, float radius = 0.25f)
        : pos(x, y), r(radius) {
        dir = Vector2(1, 0);
    }

    void update(InputData input, float dt) {
        if (!input.correct) return;

        _yaw -= input.x * dt * SENSITIVITY; // continuous (unused)
        _yaw -= input.turnAngle;            // fixed per-click
        if (_yaw < 0.f)     _yaw += M_2PI;
        if (_yaw >= M_2PI)  _yaw -= M_2PI;
        dir = Vector2(cosf(_yaw), sinf(_yaw));

        Direction direction = input.getDirection();
        if (direction == Direction::STOPPED) return;

        Vector2 moveDir = (direction == Direction::DOWN) ? invert(dir) : dir;

        float newX = pos.x + moveDir.x * dt * _vel;
        float newY = pos.y + moveDir.y * dt * _vel;

        int ix = (int)newX, iy = (int)newY;
        int cx = (int)pos.x, cy = (int)pos.y;

        if (ix >= 0 && ix < MAP_W && gameMap[cy][ix] == 0) pos.x = newX;
        if (iy >= 0 && iy < MAP_H && gameMap[iy][cx] == 0) pos.y = newY;
    }
};

Player player(8.0f, 8.0f, 0.25f);
