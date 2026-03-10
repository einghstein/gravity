#include <stdlib.h>

#include <iostream>
#include <vector>
#include <cmath>

// SFML support removed — console-only simulator

// Gravitational constant (tunable for simulation scale)
const double G = 66.743; // current workspace value

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

int main() {
	// Create an N-body system
	std::vector<Body> bodies;
	// Example: three bodies
	bodies.emplace_back(Body(-1.0, 0.0, 1.0, 5.0, 1.0));
	bodies.emplace_back(Body(0.0, 0.0, 1.0, 10.0, 1.0));
	bodies.emplace_back(Body(1.0, 0.2, 1.0, 7.5, 1.0));

	const double dt = 0.01;
	const int steps = 5000;

	for (int step = 0; step < steps; ++step) {
		// reset forces
		for (auto& b : bodies) b.resetForce();

		// compute pairwise forces
		for (size_t i = 0; i < bodies.size(); ++i) {
			for (size_t j = i + 1; j < bodies.size(); ++j) {
				Vector2 f = bodies[i].update(bodies[j]);
				// force on i by j is +f, on j by i is -f
				bodies[i].addForce(f);
				bodies[j].addForce(Vector2(-f.x, -f.y));
			}
		}

		// integrate
		for (auto& b : bodies) b.integrate(dt);

		// print positions every 50 steps
		if (step % 50 == 0) {
			std::cout << "Step " << step << "\n";
			for (size_t i = 0; i < bodies.size(); ++i) {
				const Body& b = bodies[i];
				std::cout << "  Body " << i << ": pos=(" << b.x << ", " << b.y << ") vel=(" << b.vx << ", " << b.vy << ")\n";
			}
		}
	}

	return 0;
}


