#pragma once
#include <vector>
#include <queue>
#include <tuple>
#include "Grid.h"

struct FlowField {
    // direction per cell: dx, dy toward nearest exit
    std::vector<float> dirX, dirY;
    std::vector<float> cost;
    int width, height;

    FlowField(int w, int h) : width(w), height(h),
        dirX(w * h, 0), dirY(w * h, 0),
        cost(w * h, 1e9f) {}

    void compute(const Grid& grid) {
        // reset
        std::fill(cost.begin(), cost.end(), 1e9f);
        std::fill(dirX.begin(), dirX.end(), 0);
        std::fill(dirY.begin(), dirY.end(), 0);

        // min-heap: {cost, x, y}
        using T = std::tuple<float, int, int>;
        std::priority_queue<T, std::vector<T>, std::greater<T>> pq;

        // seed from all exit cells
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (grid.get(x, y).type == CellType::EXIT) {
                    cost[y * width + x] = 0;
                    pq.push({0, x, y});
                }
            }
        }

        int dx8[] = {-1,0,1,-1,1,-1,0,1};
        int dy8[] = {-1,-1,-1,0,0,1,1,1};
        float dc8[] = {1.41f,1,1.41f,1,1,1.41f,1,1.41f};

        while (!pq.empty()) {
            auto [c, x, y] = pq.top(); pq.pop();
            if (c > cost[y * width + x]) continue;

            for (int i = 0; i < 8; i++) {
                int nx = x + dx8[i];
                int ny = y + dy8[i];
                if (!grid.inBounds(nx, ny)) continue;
                if (grid.get(nx, ny).type == CellType::OBSTACLE) continue;

                float nc = c + dc8[i];
                if (nc < cost[ny * width + nx]) {
                    cost[ny * width + nx] = nc;
                    pq.push({nc, nx, ny});
                }
            }
        }

        // compute directions (point toward lower cost neighbor)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (grid.get(x, y).type == CellType::OBSTACLE) continue;
                float best = cost[y * width + x];
                float bx = 0, by = 0;
                for (int i = 0; i < 8; i++) {
                    int nx = x + dx8[i];
                    int ny = y + dy8[i];
                    if (!grid.inBounds(nx, ny)) continue;
                    if (cost[ny * width + nx] < best) {
                        best = cost[ny * width + nx];
                        bx = (float)dx8[i];
                        by = (float)dy8[i];
                    }
                }
                // normalize
                float len = sqrtf(bx*bx + by*by);
                if (len > 0) { bx /= len; by /= len; }
                dirX[y * width + x] = bx;
                dirY[y * width + x] = by;
            }
        }
    }

    float getDirX(int x, int y) const { return dirX[y * width + x]; }
    float getDirY(int x, int y) const { return dirY[y * width + x]; }
};