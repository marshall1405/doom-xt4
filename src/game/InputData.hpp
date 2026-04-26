#pragma once

enum class Direction { UP, DOWN, LEFT, RIGHT, UP_LEFT, UP_RIGHT, STOPPED };

struct InputData {
    float x = 0;          // continuous turn (unused now, kept for future)
    float turnAngle = 0;  // fixed turn in radians applied directly per click
    bool fwd      = false;
    bool bwd      = false;
    bool shoot    = false;
    bool anyPress = false;
    bool correct  = true;

    bool leftClick()  const { return shoot; }
    bool rightClick() const { return false; }

    Direction getDirection() const {
        if (fwd)  return Direction::UP;
        if (bwd)  return Direction::DOWN;
        return Direction::STOPPED;
    }
};
