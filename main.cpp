#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

const int WIDTH = 800;
const int HEIGHT = 600;

double sim_scale = 4.371363744640307e-09;
const int steps_per_frame = 100000;
int offset_x = 0;
int offset_y = -HEIGHT / 2;
const double scale_scaling_factor = 1.1;

const double G = 6.67430e-11;

struct Planet {
    double x, y;
    double vx, vy;
    double radius;
    double mass;
    SDL_Color color;
    std::string title;

    Planet(double px, double py, double r, double m, SDL_Color c, std::string t)
        : x(px), y(py), radius(r), mass(m), color(c), title(t) 
    {
        if (title == "Earth") {
            vx = 0;
            vy = 29780;
        } else {
            vx = vy = 0;
        }
    }

    void draw(SDL_Renderer* renderer, int offset_x, int offset_y, double radial_scale=1) {
        int sx = static_cast<int>(x * sim_scale) - offset_x;
        int sy = static_cast<int>(y * sim_scale) - offset_y;
        int r = std::max(static_cast<int>(radius * sim_scale * radial_scale), 1);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        for(int w = 0; w < r*2; ++w) {
            for(int h = 0; h < r*2; ++h) {
                int dx = r - w;
                int dy = r - h;
                if (dx*dx + dy*dy <= r*r) {
                    SDL_RenderDrawPoint(renderer, sx + dx, sy + dy);
                }
            }
        }

        if (radial_scale != 1) {
            int inner_r = std::max(static_cast<int>(r / radial_scale), 1);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            for(int w = 0; w < inner_r*2; ++w) {
                for(int h = 0; h < inner_r*2; ++h) {
                    int dx = inner_r - w;
                    int dy = inner_r - h;
                    if (dx*dx + dy*dy <= inner_r*inner_r) {
                        SDL_RenderDrawPoint(renderer, sx + dx, sy + dy);
                    }
                }
            }
        }
    }

    void apply_gravity(const Planet& other) {
        double dx = other.x - x;
        double dy = other.y - y;
        double dist_sq = dx*dx + dy*dy;
        if (dist_sq == 0) return;

        double dist = std::sqrt(dist_sq);
        double force = G * mass * other.mass / dist_sq;

        double ax = force * dx / dist / mass;
        double ay = force * dy / dist / mass;

        vx += ax;
        vy += ay;
    }

    void move() {
        x += vx;
        y += vy;
    }
};

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Gravity Sim",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WIDTH, HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18);
    if (!font) {
        std::cerr << "Font load failed: " << TTF_GetError() << std::endl;
    }

    std::vector<Planet> planets = {
        Planet(0, 0, 696340000, 1.989e30, {255,255,0,255}, "Sun"),
        Planet(149600000000, 0, 1737000, 5.972e24, {0,100,255,255}, "Earth")
    };

    bool running = true;
    SDL_Event event;

    const int FPS = 60;
    Uint32 frameDelay = 1000 / FPS;

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_A]) offset_x -= 10;
        if (keys[SDL_SCANCODE_D]) offset_x += 10;
        if (keys[SDL_SCANCODE_W]) offset_y -= 10;
        if (keys[SDL_SCANCODE_S]) offset_y += 10;
        if (keys[SDL_SCANCODE_UP]) sim_scale *= scale_scaling_factor;
        if (keys[SDL_SCANCODE_DOWN]) sim_scale /= scale_scaling_factor;

        // physics
        for (int step = 0; step < steps_per_frame; ++step) {
            for (auto& p : planets) {
                for (auto& other : planets) {
                    if (&p != &other) p.apply_gravity(other);
                }
            }
            for (auto& p : planets) p.move();
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (auto& planet : planets) {
            if (planet.title == "Earth") planet.draw(renderer, offset_x, offset_y, 1000);
            else planet.draw(renderer, offset_x, offset_y, 10);
        }

        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
    }

    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}