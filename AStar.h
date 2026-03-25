#pragma once
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include "Grid.h"
#include "Agent.h"

struct ANode {
    int x, y;
    float g, f;
    int parentX, parentY;
    bool operator>(const ANode& o) const { return f > o.f; }
};

inline float heuristic(int x, int y, int gx, int gy, Heuristic h) {
    float dx = abs(x - gx), dy = abs(y - gy);
    switch (h) {
        case Heuristic::MANHATTAN:  return dx + dy;
        case Heuristic::EUCLIDEAN:  return sqrtf(dx*dx + dy*dy);
        case Heuristic::CHEBYSHEV:  return std::max(dx, dy);
        case Heuristic::DIJKSTRA:   return 0.0f;
        case Heuristic::GREEDY:     return sqrtf(dx*dx + dy*dy) * 100.0f;
        case Heuristic::WEIGHTED:   return sqrtf(dx*dx + dy*dy) * 1.5f;
        default:                    return 0.0f;
    }
}

inline std::vector<std::pair<int,int>> astar(
    const Grid& grid, int sx, int sy, int gx, int gy, Heuristic h)
{
    std::vector<std::vector<float>> g(grid.height, std::vector<float>(grid.width, 1e9f));
    std::vector<std::vector<std::pair<int,int>>> parent(
        grid.height, std::vector<std::pair<int,int>>(grid.width, {-1,-1}));

    std::priority_queue<ANode, std::vector<ANode>, std::greater<ANode>> pq;
    g[sy][sx] = 0;
    pq.push({sx, sy, 0, heuristic(sx,sy,gx,gy,h), -1, -1});

    int dx8[] = {-1,0,1,-1,1,-1,0,1};
    int dy8[] = {-1,-1,-1,0,0,1,1,1};
    float dc8[] = {1.41f,1,1.41f,1,1,1.41f,1,1.41f};

    while (!pq.empty()) {
        ANode cur = pq.top(); pq.pop();
        int x = cur.x, y = cur.y;

        if (x == gx && y == gy) {
            // reconstruct path
            std::vector<std::pair<int,int>> path;
            while (x != -1) {
                path.push_back({x, y});
                auto [px, py] = parent[y][x];
                x = px; y = py;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        if (cur.g > g[y][x]) continue;
        parent[y][x] = {cur.parentX, cur.parentY};

        for (int i = 0; i < 8; i++) {
            int nx = x + dx8[i], ny = y + dy8[i];
            if (!grid.inBounds(nx, ny)) continue;
            if (grid.get(nx, ny).type == CellType::OBSTACLE) continue;
            float ng = g[y][x] + dc8[i];
            if (ng < g[ny][nx]) {
                g[ny][nx] = ng;
                float f = ng + heuristic(nx, ny, gx, gy, h);
                pq.push({nx, ny, ng, f, x, y});
            }
        }
    }
    return {}; // no path found
}