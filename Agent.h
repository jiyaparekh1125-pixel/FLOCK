#pragma once
#include <vector>

enum class Behavior { CALM, PANIC };

struct Agent {
    float x, y;
    float speed;
    Behavior behavior;
    bool reachedExit = false;

    Agent(float x, float y, Behavior b = Behavior::CALM)
        : x(x), y(y),
          behavior(b),
          speed(b == Behavior::CALM ? 0.05f : 0.12f)
    {}
};