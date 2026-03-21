#include "Grid.h"

Grid::Grid(int w, int h) : width(w), height(h), cells(w * h) {}

Cell& Grid::get(int x, int y) {
    return cells[y * width + x];
}

const Cell& Grid::get(int x, int y) const {
    return cells[y * width + x];
}

bool Grid::inBounds(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

bool Grid::isBoundary(int x, int y) const {
    return x == 0 || x == width - 1 || y == 0 || y == height - 1;
}

void Grid::setType(int x, int y, CellType t) {
    if (!inBounds(x, y)) return;

    // exits and entrances only allowed on boundary
    if ((t == CellType::EXIT || t == CellType::ENTRANCE) && !isBoundary(x, y))
        return;

    get(x, y).type = t;
}

void Grid::clearDensity() {
    for (auto& c : cells) c.density = 0.0f;
}