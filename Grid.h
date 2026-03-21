#pragma once
#include <vector>

enum class CellType { EMPTY, OBSTACLE, EXIT, ENTRANCE };

struct Cell {
    CellType type = CellType::EMPTY;
    int agentCount = 0;
    float density = 0.0f;
};

class Grid {
public:
    Grid(int w, int h);

    Cell& get(int x, int y);
    const Cell& get(int x, int y) const;
    bool inBounds(int x, int y) const;
    bool isBoundary(int x, int y) const;
    void setType(int x, int y, CellType t);
    void clearDensity();

    int width, height;

private:
    std::vector<Cell> cells;
};