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

typedef enum {
    HARD,
    MEDIUM,
    EASY
} EnemyDifficulty;

class NormalEntity : public Sprite {
public:
    const void* pTetrisPlayer;

    explicit NormalEntity(const void* tetrisPlayer, const SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL, const int width = 60, const int height = 50, const int initialRotation = 0)
            : Sprite({ 0, 41, DEFAULT_SPRITE_W, DEFAULT_SPRITE_H }, width, height, initialRotation) {
        this->setTextureFile("../assets/entity_01.bmp");
        this->scale(4);
        this->flipSprite(flip);
        this->pTetrisPlayer = tetrisPlayer;
    }

    // typedefs
    bool isMiniboss = false;
    bool isBoss = false;

    // fields
    int defaultFrameSpeed = 5;
    EnemyDifficulty difficulty = EASY;

    bool isDead = false;
    bool isAttacking = false;
    bool isSpawning = true;

    /**
     * The damage threshold the entity will deal
     */
    int damageBounds[2] = {0, 0};

    /**
     * The amount of time the entity will wait before make you shit your pants again
     */
    double attackSpeed = 10; // speed at which the entity attacks
    int attackDelayCounter = 0; // counter to track time until the next attack
    int minAttackInterval, maxAttackInterval; // attack interval range (min, max)

    /**
     * The amount of health the entity has
     */
    int currentHealth = 0, maxHealthPoints = 0;

    /**
     * For the red flashing effect
     */
    int glowRedUntil = 0;

    void setDamageThresholds(int lowerBound, int upperBound) {
        this->damageBounds[0] = lowerBound;
        this->damageBounds[1] = upperBound;
    }

    void setAttackSpeed(double speed) {
        this->attackSpeed = speed;
        this->minAttackInterval = static_cast<int>(attackSpeed * 0.7); // 70%
        this->maxAttackInterval = static_cast<int>(attackSpeed * 1.3); // 130%
    }

    void setMaxHealth(int health) {
        this->currentHealth = this->maxHealthPoints = health;
    }

    void setHealth(int health) {
        this->currentHealth = min(maxHealthPoints, max(0, health));
    }

    void setDifficulty(EnemyDifficulty enemyDifficulty) {
        this->difficulty = enemyDifficulty;
    }

    KillRewards damageEntity(int damage);

    void die(bool isArmor);
    void remove();

    virtual void attackPlayer(const void* p);

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
    void moveSmooth(const int targetX, const int targetY, const function<void()> onComplete = nullptr, const int speed_ = 5);

    void attackAnimation();

    int frameSpeed{};
    int maxOffset{};
    virtual void setAnimation(const int animation, const int startFrame = 0);

    function<void()> animationAfterAttackAnimation = nullptr;
    void scheduleAnimation(int animation, function<void()> toRunLater, int fs);

    int textureOffset{};
    function<void()> onMovedComplete;

    int internalClock = 0;
    void onDrawCall() override;
    void onDrawCallExtended(SDL_Renderer* renderer) override;
    void onBeforeTextureDraw(SDL_Texture* texture) override;
private:
    void processMove();

public:
    ~NormalEntity() override = default;
};

#endif //NORMAL_ENTITY_H
