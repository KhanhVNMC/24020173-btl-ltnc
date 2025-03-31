//
// Created by GiaKhanhVN on 3/31/2025.
//

#ifndef TETISENGINE_PARTICLES_H
#define TETISENGINE_PARTICLES_H
#include "sprite.h"
#include <vector>
#include <cstdlib>
#include <ctime>

class Particle : public Sprite {
private:
    double px, py; // Floating-point position
    double vx, vy; // Velocity
    int lifetime;  // Frames until particle disappears
    double gravity; // Downward acceleration

public:
    /**
     * @param texture texture to use
     * @param width particle width (sprite's)
     * @param height particle height (sprite's)
     * @param initialX the first position X
     * @param initialY the first pos Y
     * @param initialVx initial Y velocity (moves down with gravity)
     * @param initialVy initial X velocity (moves down with gravity)
     * @param lifetime frames that this shit would survive for
     * @param gravity downwards momentum
     */
    Particle(SpriteTexture texture, int width, int height, double initialX, double initialY,
             double initialVx, double initialVy, int lifetime, double gravity)
            : Sprite(texture, width, height),
              px(initialX), py(initialY),
              vx(initialVx), vy(initialVy),
              lifetime(lifetime), gravity(gravity) {
        x = static_cast<int>(px);
        y = static_cast<int>(py);
    }

    void onDrawCall() override {
        // update position with velocity
        px += vx;
        py += vy;
        // apply gravity (downward, so vy increases)
        vy += gravity;
        // teleport
        this->teleport(static_cast<int>(px), static_cast<int>(py));
        // time
        lifetime--;
        if (lifetime <= 0) {
            this->discard(); // remove from garbage and clean so no mem leaks kek
        }
    }
};

class ParticleSystem : public Sprite {
private:
    SpriteTexture particleTexture; // texture for particle (sprite.h)
    int particleWidth, particleHeight; // bounding size
    int emitInterval; // interval of busting
    int emitCounter;  // for counting the intervals
    int numParticlesPerEmit; // amount per busting
    double initialVxMin, initialVxMax; // horizontal velocity range
    double initialVyMin, initialVyMax; // vertical velocity range (negative = up, positive = down [y major])
    int lifetime; // life of each particle
    double gravity; // Gravity value

    // random float generator
    static double randomFloat(double min, double max) {
        return min + static_cast<double>(rand()) / RAND_MAX * (max - min);
    }

public:
    bool once; // emit once and die

    ParticleSystem(SpriteTexture particleTexture, int particleWidth, int particleHeight,
                   int emitInterval, int numParticlesPerEmit,
                   double initialVxMin, double initialVxMax,
                   double initialVyMin, double initialVyMax,
                   int lifetime, double gravity)
            : Sprite({0, 0, 0, 0}, 0, 0), // Zero size so it doesn’t render
              particleTexture(particleTexture),
              particleWidth(particleWidth), particleHeight(particleHeight),
              emitInterval(emitInterval), emitCounter(0),
              numParticlesPerEmit(numParticlesPerEmit),
              initialVxMin(initialVxMin), initialVxMax(initialVxMax),
              initialVyMin(initialVyMin), initialVyMax(initialVyMax),
              lifetime(lifetime), gravity(gravity) {}

    // this gets called every single game tick
    void onDrawCall() override {
        emitCounter++; // only emit in certain frames (+ % mod)
        if (emitCounter >= emitInterval) {
            emitCounter = 0;
            for (int i = 0; i < numParticlesPerEmit; i++) {
                double vx = randomFloat(initialVxMin, initialVxMax); // choose a random velocity to bust
                double vy = randomFloat(initialVyMin, initialVyMax);
                // new particle eh
                Particle* p = new Particle(particleTexture, particleWidth, particleHeight,
                                           x, y, vx, vy, lifetime, gravity);
                p->spawn(); // begin render (Each frame)
            }
            if (once) { // kill the entity because its once
                this->discard();
            }
        }
    }

    ~ParticleSystem() {
        // what the fuck
        // PSA: Removed the garbage collector entry because shit lag as fuck
    }
};

#endif //TETISENGINE_PARTICLES_H
