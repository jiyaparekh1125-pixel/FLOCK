#pragma once
#include <vector>
#include <utility>

enum class Behavior { CALM, PANIC };
enum class Heuristic {
    MANHATTAN,
    EUCLIDEAN,
    CHEBYSHEV,
    DIJKSTRA,
    GREEDY,
    WEIGHTED
};

struct Agent {
    float x, y;
    float speed;
    Behavior behavior;
    bool reachedExit = false;
    Heuristic heuristic;
    std::vector<std::pair<int,int>> path;
    int pathIndex = 0;

    Agent(float x, float y, Behavior b = Behavior::CALM)
        : x(x), y(y),
          behavior(b),
          speed(b == Behavior::CALM ? 0.05f : 0.12f),
          heuristic(b == Behavior::CALM ? Heuristic::EUCLIDEAN : Heuristic::GREEDY)
    {}
};