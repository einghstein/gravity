import pygame
import math

pygame.init()

WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Gravity Sim")

clock = pygame.time.Clock()
font = pygame.font.SysFont("Arial", 18)

sim_scale = 4.371363744640307e-09
steps_per_frame = 100000
offset = [0, -HEIGHT // 2]
scale_scaling_factor = 1.1

G = 6.67430e-11
title = 0


class Planet:
    def __init__(self, position, radius, mass, color, title=str(title)):
        self.x, self.y = position
        if title == "Earth":
            self.vx, self.vy = 0, 29780
        else:
            self.vx, self.vy = 0, 0
        self.radius = radius
        self.mass = mass
        self.color = color
        self.title = title
        title += 1

    def draw(self, surface, offset, radial_scale=1):
        sx = int(self.x * sim_scale) - offset[0]
        sy = int(self.y * sim_scale) - offset[1]
        r = int(self.radius * sim_scale * radial_scale)

        pygame.draw.circle(surface, self.color, (sx, sy), max(r, 1))

        if radial_scale != 1:
            pygame.draw.circle(surface, (255, 0, 0), (sx, sy), max(r / radial_scale, 1))

    def apply_gravity(self, other):
        dx = other.x - self.x
        dy = other.y - self.y

        dist_sq = dx*dx + dy*dy
        if dist_sq == 0:
            return

        dist = math.sqrt(dist_sq)

        force = G * self.mass * other.mass / dist_sq

        ax = force * dx / dist / self.mass
        ay = force * dy / dist / self.mass

        self.vx += ax
        self.vy += ay

    def move(self):
        self.x += self.vx
        self.y += self.vy


planets = [
    Planet((0, 0), 696340000, 1.989e30, (255,255,0), "Sun"),      # Sun
    Planet((149600000000, 0), 1737000, 5.972e24, (0,100,255), "Earth") # Earth
]

running = True
FPS = 60

while running:

    clock.tick(FPS)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    keys = pygame.key.get_pressed()

    if keys[pygame.K_a]:
        offset[0] -= 10
    if keys[pygame.K_d]:
        offset[0] += 10
    if keys[pygame.K_w]:
        offset[1] -= 10
    if keys[pygame.K_s]:
        offset[1] += 10

    if keys[pygame.K_UP]:
        sim_scale *= scale_scaling_factor

    if keys[pygame.K_DOWN]:
        sim_scale /= scale_scaling_factor


    # physics
    for _ in range(steps_per_frame):

        for p in planets:
            for other in planets:
                if p != other:
                    p.apply_gravity(other)

        for p in planets:
            p.move()


    screen.fill((0,0,0))

    for planet in planets:
        if planet.title == "Earth":
            planet.draw(screen, offset, radial_scale=1000)
        else:
            planet.draw(screen, offset, radial_scale=10)


    fps = clock.get_fps()
    fps_text = font.render(f"FPS: {fps:.2f}", True, (255,255,255))
    screen.blit(fps_text,(10,10))

    pygame.display.flip()

pygame.quit()