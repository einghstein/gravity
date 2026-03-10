// /home/ein/praca/gravity/gravity/visual.cpp
// Simple SDL2-based interface to render circles on a black screen.
// Build: g++ visual.cpp -lSDL2 -std=c++17 -O2 -o visual

#if defined(__has_include)
#  if __has_include(<SDL2/SDL.h>)
#    include <SDL2/SDL.h>
#  elif __has_include(<SDL.h>)
#    include <SDL.h>
#  else
#    error "SDL header not found; install SDL2 and configure includePath"
#  endif
#else
#  include <SDL2/SDL.h>
#endif
#include <vector>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <string>

// --- Simulation types (integrated from main.cpp) ---------------------------------

// Gravitational constant (tunable for simulation scale)
const double G = 60.743e4;
// Softening parameter to avoid singularity at very small separations
const double SOFTENING = 1e-3;

struct Vector2 {
    double x;
    double y;

    Vector2() : x(0), y(0) {}
    Vector2(double x, double y) : x(x), y(y) {}

    Vector2 operator+(const Vector2& o) const { return Vector2(x + o.x, y + o.y); }
    Vector2 operator-(const Vector2& o) const { return Vector2(x - o.x, y - o.y); }
    Vector2 operator*(double s) const { return Vector2(x * s, y * s); }
    Vector2& operator+=(const Vector2& o) { x += o.x; y += o.y; return *this; }
    double length() const { return std::sqrt(x*x + y*y); }
};

// Default simulation speed multiplier: 1.0 = normal, <1 slower, >1 faster
constexpr double DEFAULT_SIM_SPEED = 1.0;

class Body {
public:
    double x;
    double y;
    double vx;
    double vy;
    double radius;
    double mass;
    double density;
    // accumulated force
    Vector2 force;

    Body(double x=0, double y=0, double radius=0, double mass=0, double density=0)
        : x(x), y(y), vx(0), vy(0), radius(radius), mass(mass), density(density), force() {}

    // Compute gravitational force vector exerted on this body by `other`.
    // Returns the force vector (Fx, Fy). Uses Newton's law: F = G * m1 * m2 / (r^2 + eps^2)
    Vector2 update(const Body& other) const {
        Vector2 dir(other.x - x, other.y - y);
        double r2 = dir.x*dir.x + dir.y*dir.y + SOFTENING*SOFTENING;
        double r = std::sqrt(r2);
        if (r <= 0.0) return Vector2(0,0);
        double forceMag = G * mass * other.mass / r2;
        Vector2 unit(dir.x / r, dir.y / r);
        return unit * forceMag;
    }

    void resetForce() { force.x = 0; force.y = 0; }
    void addForce(const Vector2& f) { force += f; }

    // Integrate using symplectic (semi-implicit) Euler: update velocity from force, then position
    void integrate(double dt) {
        // a = F / m
        if (mass > 0) {
            vx += (force.x / mass) * dt;
            vy += (force.y / mass) * dt;
            x += vx * dt;
            y += vy * dt;
        }
    }
};

// -------------------------------------------------------------------------------

struct Circle {
    double x, y;        // position
    double r;           // radius
    uint8_t R, G, B;    // color

    Circle(double px = 0, double py = 0, double pr = 10,
           uint8_t pR = 255, uint8_t pG = 255, uint8_t pB = 255)
        : x(px), y(py), r(pr), R(pR), G(pG), B(pB) {}
};

class Visual {
public:
    Visual(int width = 800, int height = 600, const char* title = "Circles")
        : win(nullptr), ren(nullptr), w(width), h(height)
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
            std::exit(1);
        }
        win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               w, h, SDL_WINDOW_SHOWN);
        if (!win) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
            cleanup_and_exit();
        }
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!ren) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
            cleanup_and_exit();
        }
    }

    ~Visual() {
        if (ren) SDL_DestroyRenderer(ren);
        if (win) SDL_DestroyWindow(win);
        SDL_Quit();
    }

    void addCircle(const Circle& c) { circles.push_back(c); }
    void clearCircles() { circles.clear(); }

    // Render one frame (black background, then all circles)
    void renderFrame() {
        // Clear to black
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // Draw circles
        for (const auto& c : circles) drawFilledCircle(c);

        SDL_RenderPresent(ren);
    }

    // Start a simple event loop. Press ESC or close window to quit.
    void run() {
        bool running = true;
        SDL_Event ev;
        while (running) {
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_QUIT) running = false;
                else if (ev.type == SDL_KEYDOWN) {
                    if (ev.key.keysym.sym == SDLK_ESCAPE) running = false;
                }
            }
            renderFrame();
            SDL_Delay(1);
        }
    }

private:
    SDL_Window* win;
    SDL_Renderer* ren;
    int w, h;
    std::vector<Circle> circles;

    void cleanup_and_exit() {
        if (ren) SDL_DestroyRenderer(ren);
        if (win) SDL_DestroyWindow(win);
        SDL_Quit();
        std::exit(1);
    }

    // Fast filled circle drawing via horizontal scanlines
    void drawFilledCircle(const Circle& c) {
        int y0 = static_cast<int>(std::ceil(c.y - c.r));
        int y1 = static_cast<int>(std::floor(c.y + c.r));
        SDL_SetRenderDrawColor(ren, c.R, c.G, c.B, 255);
        for (int y = y0; y <= y1; ++y) {
            double dy = y + 0.5 - c.y; // sample at pixel center
            double dx = std::sqrt(std::max(0.0, c.r * c.r - dy * dy));
            int x0 = static_cast<int>(std::ceil(c.x - dx));
            int x1 = static_cast<int>(std::floor(c.x + dx));
            // Clip to screen bounds
            if (y < 0 || y >= h) continue;
            if (x1 < 0 || x0 >= w) continue;
            if (x0 < 0) x0 = 0;
            if (x1 >= w) x1 = w - 1;
            SDL_RenderDrawLine(ren, x0, y, x1, y);
        }
    }
};

int main(int argc, char** argv) {
    const int width = 800;
    const int height = 600;
    Visual vis(width, height, "N-body Visualizer");

    // Simulation state (from main.cpp)
    std::vector<Body> bodies;
    bodies.emplace_back(Body(-1.0, 0.0, 1.0, 5.0, 1.0));
    bodies.emplace_back(Body(0.0, 0.0, 1.0, 10.0, 1.0));
    bodies.emplace_back(Body(1.0, 0.2, 1.0, 7.5, 1.0));

    const double base_sim_dt = 0.01;       // base simulation timestep

    // parse sim speed from env or CLI (multiplier)
    double sim_speed = DEFAULT_SIM_SPEED;
    if (const char* env = std::getenv("SIM_SPEED")) {
        try { sim_speed = std::stod(std::string(env)); } catch(...) {}
    }
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-s" || a == "--sim-speed") && i + 1 < argc) {
            try { sim_speed = std::stod(argv[i+1]); } catch(...) {}
            ++i;
        }
    }

    if (!(std::isfinite(sim_speed)) || sim_speed <= 0.0) sim_speed = DEFAULT_SIM_SPEED;

    // effective dt used for each integrate() call
    double effective_dt = base_sim_dt * sim_speed;
    std::cerr << "SIM_SPEED=" << sim_speed << " effective_dt=" << effective_dt << "\n";

    // View transform: simulation units -> pixels
    double scale = 12.0; // pixels per simulation unit
    double offset_x = width / 2.0;
    double offset_y = height / 2.0;

    bool running = true;
    bool paused = false;
    SDL_Event ev;
    (void)0; // no fractional accumulator needed when scaling dt

    // Colors for bodies
    const uint8_t colors[][3] = {{255,0,0}, {0,255,0}, {0,120,255}, {255,255,0}};

    while (running) {
        // Event handling
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            else if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_SPACE: paused = !paused; break;
                    case SDLK_EQUALS: // = or +
                    case SDLK_KP_PLUS: scale *= 1.1; break; // zoom in
                    case SDLK_MINUS: // -
                    case SDLK_KP_MINUS: scale /= 1.1; break;    // zoom out
                    case SDLK_LEFT: offset_x += 20; break;
                    case SDLK_RIGHT: offset_x -= 20; break;
                    case SDLK_UP: offset_y += 20; break;
                    case SDLK_DOWN: offset_y -= 20; break;
                    case SDLK_PERIOD: // step forward when paused
                        if (paused) {
                            // single simulation step
                            for (size_t i = 0; i < bodies.size(); ++i) bodies[i].resetForce();
                            for (size_t i = 0; i < bodies.size(); ++i) {
                                for (size_t j = i+1; j < bodies.size(); ++j) {
                                    Vector2 f = bodies[i].update(bodies[j]);
                                    bodies[i].addForce(f);
                                    bodies[j].addForce(Vector2(-f.x, -f.y));
                                }
                            }
                            for (auto& b : bodies) b.integrate(effective_dt);
                        }
                        break;
                    default: break;
                }
            }
        }

        // Simulation updates: single step per frame using scaled dt
        if (!paused) {
            // reset forces
            for (auto& b : bodies) b.resetForce();

            // compute pairwise forces
            for (size_t i = 0; i < bodies.size(); ++i) {
                for (size_t j = i + 1; j < bodies.size(); ++j) {
                    Vector2 f = bodies[i].update(bodies[j]);
                    bodies[i].addForce(f);
                    bodies[j].addForce(Vector2(-f.x, -f.y));
                }
            }

            // integrate using effective_dt
            for (auto& b : bodies) b.integrate(effective_dt);
        }

        // Render
        vis.clearCircles();
        for (size_t i = 0; i < bodies.size(); ++i) {
            const Body& b = bodies[i];
            double sx = offset_x + b.x * scale;
            double sy = offset_y - b.y * scale; // invert Y for screen coords
            double sr = std::max(2.0, b.radius * scale * 0.6);
            const auto& col = colors[i % (sizeof(colors)/sizeof(colors[0]))];
            vis.addCircle(Circle(sx, sy, sr, col[0], col[1], col[2]));
        }
        vis.renderFrame();

        // Small delay to limit CPU usage / control frame rate
        SDL_Delay(16); // ~60 FPS
    }

    return 0;
}