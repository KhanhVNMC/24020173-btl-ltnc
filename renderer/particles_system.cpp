//
// Created by GiaKhanhVN on 2/25/2025.
//

#include "particles.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

[[maybe_unused]] int randomASqueeze(const int a) {
    return (rand() % (2 * a + 1)) - a;
}

particle_unit spawn_particle(
    const int x, const int y,
    const double vx, const double vy,
    const int aliveTicks, const int texture)
{
    return (particle_unit){
        x, y, aliveTicks, vx, vy, false, texture
    };
}

particle_unit spawn_spewing_particle(
    const int x, const int y,
    const int aliveTicks, const int texture)
{
    return spawn_particle(x, y, randomASqueeze(5), randomASqueeze(5), aliveTicks, texture);
}


constexpr double GRAVITY_FACTOR = 0.5 * 9.8; // 9.8m/s^2
double xOffsetFunction(const particle_unit* particle, const double currentTick, const int epsilon) {
    return (particle->x0 + (particle->initial_x_velocity * currentTick) + epsilon);
}

double yOffsetFunction(const particle_unit* particle, const double currentTick, const int beta) {
    return (particle->y0 + (particle->initial_y_velocity * currentTick) - (GRAVITY_FACTOR * currentTick * currentTick) + beta);
}