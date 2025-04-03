//
// Created by SONPHUONG on 4/2/2025.
//

#ifndef NORMAL_ENTITY_H
#define NORMAL_ENTITY_H
#include "../../spritesystem/sprite.h"
#include "../entity_prop.h"
#include <cmath>
#include <functional>

using namespace std;

typedef struct {
    int type; // 0 = armor, 1 = energy, (-1 = none)
    int amount;
} KillRewards;

class NormalEntity : public Sprite {
public:
    explicit NormalEntity(const SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL, const int width = 60, const int height = 50, const int initialRotation = 0)
            : Sprite({ 0, 41, DEFAULT_SPRITE_W, DEFAULT_SPRITE_H }, width, height, initialRotation) {
        this->setTextureFile("../assets/entity_01.bmp");
        this->scale(4);
        this->flipSprite(flip);
    }

    bool isDead = false;
    bool isAttacking = false;

    /**
     * The damage threshold the entity will deal
     */
    int damageBounds[2] = {0, 0};

    /**
     * The amount of time the entity will wait before crapping your pants again
     */
    double attackDelayFrames = 10; // default is 10s

    int currentHealth = 0, maxHealthPoints = 0;
    int glowRedUntil = 0;

    void setDamageThresholds(int lowerBound, int upperBound) {
        this->damageBounds[0] = lowerBound;
        this->damageBounds[1] = upperBound;
    }

    void setAttackDelaySeconds(double seconds) {
        this->attackDelayFrames = seconds * 16.66;
    }

    void setMaxHealth(int health) {
        this->currentHealth = this->maxHealthPoints = health;
    }

    void setHealth(int health) {
        this->currentHealth = min(maxHealthPoints, max(0, health));
    }

    KillRewards damageEntity(int damage);

    void die(bool isArmor);

    void attackPlayer(void* p);

    /**
     * The ACTUAL X AND Y POSITIONS, THE PROVIDED SPRITE::X AND ::Y IS FOR
     * ANIMATION PURPOSES!!! (INDEPENDENT)
     */
    int strictX{};
    int strictY{};

    /**
     * Teleport and shit (instantly)
     * @param sx target x
     * @param sy target y
     */
    void teleportStrict(const int sx, const int sy) {
        this->strictX = sx;
        this->strictY = sy;
    }

    /**
     * Target position for smooth movement
     * -1 means no movement is in progress
     */
    int targetMoveX = -1;
    int targetMoveY = -1;
    /**
     * Movement speed in pixels per frame (when you call moveSmooth you can fiddle with this)
     */
    int speed = 1;
    /**
     * Ease smoothly from one point to another
     * @param targetX
     * @param targetY
     * @param onComplete
     * @param speed_ pixel per frame
     */
    void moveSmooth(const int targetX, const int targetY, const function<void()>& onComplete = nullptr, const int speed_ = 5);

    void attackAnimation();

    int frameSpeed{};
    int maxOffset{};
    void setAnimation(const int animation);

    int textureOffset{};
    function<void()> onMovedComplete;
    void onDrawCall() override;
    void onDrawCallExtended(SDL_Renderer* renderer) override;
    void onBeforeTextureDraw(SDL_Texture* texture) override;
private:
    void processMove();

public:
    ~NormalEntity() override = default;
};

#endif //NORMAL_ENTITY_H
