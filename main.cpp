#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <algorithm>
#include <vector>
#include "Config.h"
#include "Grid.h"
#include "Agent.h"

SDL_Color densityColor(float d) {
    d = std::min(1.0f, std::max(0.0f, d));
    if (d < 0.5f) {
        float t = d * 2.0f;
        return { 0, (Uint8)(180 * t), (Uint8)(255 - 100 * t), 80 };
    } else {
        float t = (d - 0.5f) * 2.0f;
        return { (Uint8)(255 * t), (Uint8)(180 - 180 * t), 0, 80 };
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

        // color by behavior
        if (a.behavior == Behavior::CALM)
            SDL_SetRenderDrawColor(ren, 196, 174, 232, 255); // lavender
        else
            SDL_SetRenderDrawColor(ren, 240, 184, 200, 255); // pink

        // draw filled circle using rects
        for (int dy = -r; dy <= r; dy++)
            for (int dx = -r; dx <= r; dx++)
                if (dx*dx + dy*dy <= r*r) {
                    SDL_Rect dot = { px + dx, py + dy, 1, 1 };
                    SDL_RenderFillRect(ren, &dot);
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
    std::vector<Agent> agents;

    int mode = 0; // 0=obstacle, 1=exit, 2=entrance, 3=erase, 4=calm agent, 5=panic agent
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
                if (e.key.keysym.sym == SDLK_5) mode = 4; // calm agent
                if (e.key.keysym.sym == SDLK_6) mode = 5; // panic agent
                if (e.key.keysym.sym == SDLK_SPACE) {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    mx /= CELL_SIZE;
                    my /= CELL_SIZE;
                    if (grid.inBounds(mx, my))
                        grid.setType(mx, my, CellType::EMPTY);
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN ||
               (e.type == SDL_MOUSEMOTION && e.motion.state)) {
                int mx = e.button.x / CELL_SIZE;
                int my = e.button.y / CELL_SIZE;
                if (grid.inBounds(mx, my)) {
                    if (mode == 0) grid.setType(mx, my, CellType::OBSTACLE);
                    else if (mode == 1) grid.setType(mx, my, CellType::EXIT);
                    else if (mode == 2) grid.setType(mx, my, CellType::ENTRANCE);
                    else if (mode == 3) grid.setType(mx, my, CellType::EMPTY);
                    else if (mode == 4 && e.type == SDL_MOUSEBUTTONDOWN)
                        agents.push_back(Agent((float)mx, (float)my, Behavior::CALM));
                    else if (mode == 5 && e.type == SDL_MOUSEBUTTONDOWN)
                        agents.push_back(Agent((float)mx, (float)my, Behavior::PANIC));
                }
            }
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