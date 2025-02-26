//
// Created by GiaKhanhVN on 2/25/2025.
//

#ifndef PARTICLES_H
#define PARTICLES_H
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // x0, y0, t
    int x0, y0, alive_ticks;
    // excitement unit
    double initial_x_velocity, initial_y_velocity;
    // soft particle
    bool soft;
    // the texture
    int p_texture_grid;
} particle_unit;

particle_unit spawn_particle(
    int x, int y,
    double vx, double vy,
    int aliveTicks, int texture);

particle_unit spawn_spewing_particle(
    int x, int y,
    int aliveTicks, int texture);

double xOffsetFunction(const particle_unit* particle, double currentTick, int epsilon);
double yOffsetFunction(const particle_unit* particle, double currentTick, int beta);

#endif //PARTICLES_H
