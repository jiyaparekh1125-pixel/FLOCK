#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <algorithm>
#include <vector>
#include "Config.h"
#include "Grid.h"
#include "Agent.h"
#include "FlowField.h"

SDL_Color densityColor(float d) {
    d = std::min(1.0f, std::max(0.0f, d));
    if (d < 0.5f) {
        float t = d * 2.0f;
        return { 0, (Uint8)(180 * t), (Uint8)(255 - 100 * t), 40 };
    } else {
        float t = (d - 0.5f) * 2.0f;
        return { (Uint8)(255 * t), (Uint8)(180 - 180 * t), 0, 40 };
    }
}

void renderGrid(SDL_Renderer* ren, const Grid& grid) {
    for (int y = 0; y < grid.height; y++) {
        for (int x = 0; x < grid.width; x++) {
            const Cell& c = grid.get(x, y);
            SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1 };
            switch (c.type) {
                case CellType::EMPTY:
                    SDL_SetRenderDrawColor(ren, 218, 238, 249, 255); break;
                case CellType::OBSTACLE:
                    SDL_SetRenderDrawColor(ren, 244, 167, 167, 255); break;
                case CellType::EXIT:
                    SDL_SetRenderDrawColor(ren, 168, 230, 184, 255); break;
                case CellType::ENTRANCE:
                    SDL_SetRenderDrawColor(ren, 168, 200, 240, 255); break;
            }
            SDL_RenderFillRect(ren, &rect);

            if (c.type == CellType::EMPTY && c.density > 0.05f) {
                SDL_Color dc = densityColor(c.density);
                SDL_SetRenderDrawColor(ren, dc.r, dc.g, dc.b, dc.a);
                SDL_RenderFillRect(ren, &rect);
            }
        }
    }
}

void renderAgents(SDL_Renderer* ren, const std::vector<Agent>& agents) {
    for (const auto& a : agents) {
        if (a.reachedExit) continue;
        int px = (int)(a.x * CELL_SIZE + CELL_SIZE / 2);
        int py = (int)(a.y * CELL_SIZE + CELL_SIZE / 2);
        int r = CELL_SIZE / 3;
        if (a.behavior == Behavior::CALM)
            SDL_SetRenderDrawColor(ren, 196, 174, 232, 255);
        else
            SDL_SetRenderDrawColor(ren, 240, 184, 200, 255);
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++)
                if (dx*dx + dy*dy <= r*r) {
                    SDL_Rect dot = { px + dx, py + dy, 1, 1 };
                    SDL_RenderFillRect(ren, &dot);
                }
    }
}

void updateAgents(std::vector<Agent>& agents, const Grid& grid, const FlowField& ff) {
    for (auto& a : agents) {
        if (a.reachedExit) continue;
        int cx = (int)a.x;
        int cy = (int)a.y;
        if (grid.inBounds(cx, cy) && grid.get(cx, cy).type == CellType::EXIT) {
            a.reachedExit = true;
            continue;
        }
        if (grid.inBounds(cx, cy)) {
            float dx = ff.getDirX(cx, cy);
            float dy = ff.getDirY(cx, cy);
            float nx = a.x + dx * a.speed;
            float ny = a.y + dy * a.speed;
            int inx = (int)nx, iny = (int)ny;
            if (grid.inBounds(inx, iny) && grid.get(inx, iny).type != CellType::OBSTACLE) {
                a.x = nx;
                a.y = ny;
            }
        }
    }
}

void updateDensity(Grid& grid, const std::vector<Agent>& agents) {
    for (int y = 0; y < grid.height; y++)
        for (int x = 0; x < grid.width; x++)
            grid.get(x, y).density *= 0.85f;

    for (const auto& a : agents) {
        if (a.reachedExit) continue;
        int cx = (int)a.x, cy = (int)a.y;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = cx + dx, ny = cy + dy;
                if (!grid.inBounds(nx, ny)) continue;
                float dist = sqrtf((float)(dx*dx + dy*dy));
                float weight = expf(-dist * dist / 2.0f);
                grid.get(nx, ny).density = std::min(1.0f, grid.get(nx, ny).density + weight * 0.08f);
            }
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow(
        "FLOCK",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, 0
    );
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    Grid grid(GRID_W, GRID_H);
    FlowField ff(GRID_W, GRID_H);
    std::vector<Agent> agents;

    bool simRunning = false;
    bool needsRecompute = true;
    int mode = 0;
    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_1) mode = 0;
                if (e.key.keysym.sym == SDLK_2) mode = 1;
                if (e.key.keysym.sym == SDLK_3) mode = 2;
                if (e.key.keysym.sym == SDLK_4) mode = 3;
                if (e.key.keysym.sym == SDLK_5) mode = 4;
                if (e.key.keysym.sym == SDLK_6) mode = 5;
                if (e.key.keysym.sym == SDLK_r) {
                    simRunning = !simRunning;
                    needsRecompute = true;
                }
                if (e.key.keysym.sym == SDLK_c) agents.clear();
                if (e.key.keysym.sym == SDLK_SPACE) {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    mx /= CELL_SIZE;
                    my /= CELL_SIZE;
                    if (grid.inBounds(mx, my))
                        grid.setType(mx, my, CellType::EMPTY);
                    needsRecompute = true;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN ||
               (e.type == SDL_MOUSEMOTION && e.motion.state)) {
                int mx = e.button.x / CELL_SIZE;
                int my = e.button.y / CELL_SIZE;
                if (grid.inBounds(mx, my)) {
                    if (mode == 0) { grid.setType(mx, my, CellType::OBSTACLE); needsRecompute = true; }
                    else if (mode == 1) { grid.setType(mx, my, CellType::EXIT); needsRecompute = true; }
                    else if (mode == 2) { grid.setType(mx, my, CellType::ENTRANCE); needsRecompute = true; }
                    else if (mode == 3) { grid.setType(mx, my, CellType::EMPTY); needsRecompute = true; }
                    else if (mode == 4 && e.type == SDL_MOUSEBUTTONDOWN)
                        agents.push_back(Agent((float)mx, (float)my, Behavior::CALM));
                    else if (mode == 5 && e.type == SDL_MOUSEBUTTONDOWN)
                        agents.push_back(Agent((float)mx, (float)my, Behavior::PANIC));
                }
            }
        }

        if (needsRecompute) {
            ff.compute(grid);
            needsRecompute = false;
        }

        if (simRunning) {
            updateAgents(agents, grid, ff);
            updateDensity(grid, agents);
        }

        SDL_SetRenderDrawColor(ren, 184, 216, 240, 255);
        SDL_RenderClear(ren);
        renderGrid(ren, grid);
        renderAgents(ren, agents);
        SDL_RenderPresent(ren);
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}