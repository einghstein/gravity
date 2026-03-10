import sys
import pygame
import math

#!/usr/bin/env python3
"""
Base Pygame script.
Save as python.py and run: python3 python.py
"""


# Configuration
WIDTH, HEIGHT = 1920/4*3, 1080/4*3
FPS = 60
BG_COLOR = (0, 0, 0)

# Globals (replaces Game class)
pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Pygame Base")
clock = pygame.time.Clock()
font = pygame.font.Font(None, 24)

run = True

gravity_constant = 6.67430e-11

sim_speed = int(17280 / 4)
sim_space_multiplier = 0.000001
sim_radius_multiplier = 0.00001
sim_move_speed = 10000000
key_cooldown = 0

begin_offset = [WIDTH // 2, HEIGHT // 2]

#temporary planet for testing
moon_begin_speed = [0, 66900]

class Planet:
    def __init__(self, position, speed, radius, mass, color):
        self.position = position
        self.speed = speed
        self.radius = radius
        self.mass = mass
        self.color = color
        self.acceleration = [0, 0]

    def draw(self, surface):
        pygame.draw.circle(surface, self.color, (self.position[0] * sim_space_multiplier + begin_offset[0], self.position[1] * sim_space_multiplier + begin_offset[1]), self.radius * sim_radius_multiplier)
        print(f"Drawing planet at scaled position: ({self.position[0] * sim_space_multiplier}, {self.position[1] * sim_space_multiplier}) with scaled radius: {self.radius * sim_radius_multiplier}")

    def update(self,  dt):
        for planet in planets:
            if planet != self:
                pull = gravity_constant * (self.mass * planet.mass) / ((planet.position[0] - self.position[0])**2 + (planet.position[1] - self.position[1])**2)
                force = pull / self.mass
                dx = planet.position[0] - self.position[0]
                dy = planet.position[1] - self.position[1]
                alfa = math.atan2(dy, dx)
                self.acceleration[0] += force * math.cos(alfa)
                self.acceleration[1] += force * math.sin(alfa)

        # Update position based on speed and acceleration
        self.speed[0] += self.acceleration[0] * dt
        self.speed[1] += self.acceleration[1] * dt
        self.position[0] += self.speed[0] * dt
        self.position[1] += self.speed[1] * dt


planets = [Planet([0, 0], [0, 0], 6371000, 5.972e24, (0, 0, 255)),
           Planet([384400000, 0], moon_begin_speed, 1737000, 7.34767309e22, (255, 255, 255))]

def handle_events():
    global run
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            run = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                run = False

def update(dt):
    global sim_space_multiplier, sim_radius_multiplier, key_cooldown
    key_cooldown -= 1
    keys = pygame.key.get_pressed()
    if keys[pygame.K_SPACE]:
        print("Spacebar pressed")

    if keys[pygame.K_l] and key_cooldown <= 0:
        sim_space_multiplier *= 0.9
        sim_radius_multiplier *= 0.9
        key_cooldown = 10  # Set cooldown
    if keys[pygame.K_p] and key_cooldown <= 0:
        sim_space_multiplier *= 1.1
        sim_radius_multiplier *= 1.1
        key_cooldown = 10  # Set cooldown
    if keys[pygame.K_a]:
        for planet in planets:
            planet.position[0] += sim_move_speed  # Example action: move planets to the right
    if keys[pygame.K_d]:
        for planet in planets:
            planet.position[0] -= sim_move_speed  # Example action: move planets to the left
    if keys[pygame.K_w]:
        for planet in planets:
            planet.position[1] += sim_move_speed  # Move up
    if keys[pygame.K_s]:
        for planet in planets:
            planet.position[1] -= sim_move_speed  # Move down

    screen.fill(BG_COLOR)
    for planet in planets:
        for _ in range(sim_speed):
            planet.update(dt)
        planet.draw(screen)
    pygame.display.flip()


#for _ in range(5):
#    for planet in planets:
#        planet.position[0] -= sim_move_speed 
#        planet.position[1] -= sim_move_speed  # Move down


while run:
    dt = clock.tick(FPS) / 1000.0  # delta time in seconds
    handle_events()
    update(dt)

pygame.quit()
sys.exit()
