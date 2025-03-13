//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "../spritesystem/sprite.h"
#include <functional>
#include <cmath>

using namespace std;
class FlandreScarlet final : public Sprite {
public:
    explicit FlandreScarlet(const SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL, const int width = 50, const int height = 50, const int initialRotation = 0)
    : Sprite({ 0, 41, 110, 93 }, width, height, initialRotation) {
        this->setTextureFile("../assets/flandre.bmp");
        this->scale(5);
        this->flipSprite(flip);
    }

    /**
     * The ACTUAL X AND Y POSITIONS, THE PROVIDED SPRITE::X AND ::Y IS FOR
     * ANIMATION PURPOSES!!! (INDEPENDENT)
     */
    int strictX;
    int strictY;

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
     * @param speed_ pixel per frame
     */
    void moveSmooth(const int targetX, const int targetY, const function<void()> onComplete = nullptr, const int speed_ = 5) {
        targetMoveX = targetX;
        targetMoveY = targetY;
        this->speed = speed_;
        this->onMovedComplete = onComplete;

        if (targetMoveX > strictX) {
            setAnimation(RUN_FORWARD);
        } else {
            setAnimation(RUN_BACKWARD);
        }
    }

    int maxOffset;
    int frameSpeed;
    typedef enum {
        IDLE,
        RUN_FORWARD,
        RUN_BACKWARD,
        ATTACK_01,
        ATTACK_02,
        ATTACK_CRIT
    } Animation;

    void setAnimation(const int animation) {
        // reset state
        this->height = 50;
        textureOffset = 0;
        // set
        if (animation == IDLE) {
            this->texture.textureY = 41;
            frameSpeed = 5;
            maxOffset = 8; // there's 8 sprites
            return;
        }

        if (animation == RUN_FORWARD) {
            this->texture.textureY = 136;
            frameSpeed = 5;
            maxOffset = 4; // there's 4 sprites
            return;
        }

        if (animation == RUN_BACKWARD) {
            this->texture.textureY = 225;
            frameSpeed = 5;
            maxOffset = 4; // there's 4 sprites
            return;
        }

        if (animation == ATTACK_01) {
            this->texture.textureY = 312;
            this->texture.height = 128;
            this->texture.width = 115;

            this->height = 60;

            frameSpeed = 5;
            maxOffset = 4; // there's 4 sprites
        }
    }

    int textureOffset;
    function<void()> onMovedComplete;
    void onDrawCall() override {
        processMove();
        // offset to render the sprite
        this->texture.textureX = this->originalTextureX + (128 * textureOffset);

        // sinusoidal (troi noi theo hinh sin)
        this->teleport(strictX, static_cast<int>(strictY + (std::sin(SpritesRenderingPipeline::renderPasses() / 20.0) * 5)));

        //advance animation frame if it's time
        if (SpritesRenderingPipeline::renderPasses() % frameSpeed == 0) {
            textureOffset = (textureOffset + 1) % maxOffset;
        }
    }
private:
    void processMove() {
        if (targetMoveX == -1 && targetMoveY == -1) return;
        // move towards the target x position
        if (targetMoveX != -1 && targetMoveX != strictX) {
            strictX += targetMoveX > strictX ? speed : -speed;
        } else {
            targetMoveX = -1;
        }
        // move towards the target Y position
        if (targetMoveY != -1 && targetMoveY != strictY) {
            strictY += targetMoveY > strictY ? speed : -speed;
        } else {
            targetMoveY = -1;
        }
        if (targetMoveX == -1 && targetMoveY == -1 && onMovedComplete) {
            onMovedComplete();
            onMovedComplete = nullptr;
        }
    }

public:
    ~FlandreScarlet() override = default;
};

/**
 * The star-night background (overlay on top of each other for parallax effect)
 */
class BackgroundScroll final : public Sprite {
private:
    int spriteScrollSpeed = 2;
public:
    explicit BackgroundScroll(const SDL_RendererFlip flip, int x, int y, const int spriteScrollSpeed = 2, const int width = 512, const int height = 256, const int initialRotation = 0):
    Sprite({ 0, 0, 512, 256 }, width, height, initialRotation) {
        this->setTextureFile("../assets/starlight.bmp");
        this->x = x; this->y = y;
        this->spriteScrollSpeed = spriteScrollSpeed;
        this->scale(4);
        this->flipSprite(flip);
    }

    void onDrawCall() override {
        this->x -= spriteScrollSpeed;
        // if the starmap scrolls pass its ending (edge), snap it back to its "offscreen-right" location
        if (x <= -texture.width * scalar) {
            x = static_cast<int>(texture.width * scalar);
        }
    }

    ~BackgroundScroll() override = default;
};

class SpikyBallMonster : public Sprite {
    explicit SpikyBallMonster(const SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL, const int width = 50, const int height = 50, const int initialRotation = 0)
    : Sprite({ 0, 41, 110, 93 }, width, height, initialRotation) {
        this->setTextureFile("../assets/spiky.bmp");
        this->scale(5);
        this->flipSprite(flip);
    }
};