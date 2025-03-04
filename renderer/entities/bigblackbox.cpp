//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "../spritesystem/sprite.h"
#include <cmath>
#include <iostream>

class FlandreScarlet final : public Sprite {
public:
    explicit FlandreScarlet(const SDL_RendererFlip flip = SDL_FLIP_HORIZONTAL, const int width = 50, const int height = 50, const int initialRotation = 0): Sprite(nullptr, width, height, initialRotation) {
        this->setupTexture(new SpriteTexture{ 0, 41, 110, 93 }, "../assets/flandre.bmp");
        this->x = 0;
        this->y = 0;
        this->scale(5);
        this->flipSprite(flip);
    }

    /**
     * The ACTUAL X AND Y POSITIONS, THE PROVIDED SPRITE::X AND ::Y IS FOR
     * ANIMATION PURPOSES!!!
     */
    int strictX;
    int strictY;

    /**
     * Teleport and shit
     * @param sx target x
     * @param sy target y
     */
    void teleportStrict(const int sx, const int sy) {
        this->strictX = sx;
        this->strictY = sy;
    }

    int targetMoveX = -1;
    int targetMoveY = -1;
    int speed = 1;
    /**
     * Ease smoothly from one point to another
     * @param targetX
     * @param targetY
     * @param speed_ pixel per frame
     */
    void moveSmooth(const int targetX, const int targetY, const int speed_ = 5) {
        targetMoveX = targetX;
        targetMoveY = targetY;
        this->speed = speed_;

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
    } Animation;

    void setAnimation(const int animation) {
        // reset state
        textureOffset = 0;
        // set
        if (animation == IDLE) {
            this->texture->textureY = 41;
            frameSpeed = 5;
            maxOffset = 8; // there's 8 sprites
            return;
        }

        if (animation == RUN_FORWARD) {
            this->texture->textureY = 136;
            frameSpeed = 5;
            maxOffset = 4; // there's 4 sprites
            return;
        }

        if (animation == RUN_BACKWARD) {
            this->texture->textureY = 225;
            frameSpeed = 5;
            maxOffset = 4; // there's 4 sprites
            return;
        }
    }

    int frames = 0;
    int textureOffset;
    void onDrawCall() override {
        processMove();
        this->texture->textureX = this->originalTextureX + (128 * textureOffset);
        this->teleport(strictX, static_cast<int>(strictY + (sin(frames / 20.0) * 5)));
        if (frames % frameSpeed == 0) {
            textureOffset = (textureOffset + 1) % maxOffset;
        }
        ++frames;
    }
private:
    void processMove() {
        if (targetMoveX == -1 && targetMoveY == -1) return;

        if (targetMoveX != -1 && targetMoveX != strictX) {
            strictX += targetMoveX > strictX ? speed : -speed;
        } else {
            targetMoveX = -1;
        }

        std::cout << targetMoveY << " " << strictY << std::endl;
        if (targetMoveY != -1 && targetMoveY != strictY) {
            strictY += targetMoveY > strictY ? speed : -speed;
        } else {
            targetMoveY = -1;
        }
    }

public:
    ~FlandreScarlet() override {
        delete this->texture;
    }
};

class BackgroundScroll final : public Sprite {
private:
    int spriteScrollSpeed = 2;
public:
    explicit BackgroundScroll(const SDL_RendererFlip flip, int x, int y, const int spriteScrollSpeed = 2, const int width = 512, const int height = 256, const int initialRotation = 0): Sprite(nullptr, width, height, initialRotation) {
        this->setupTexture(new SpriteTexture{ 0, 0, 512, 256 }, "../assets/starlight.bmp");
        this->x = x;
        this->y = y;
        this->spriteScrollSpeed = spriteScrollSpeed;
        this->scale(4);
        this->flipSprite(flip);
    }

    int calls = 0;
    int frames = 0;
    void onDrawCall() override {
        this->x -= spriteScrollSpeed;
        // if the starmap scrolls pass its ending (edge), snap it back to its "offscreen-right" location
        if (x <= -texture->width * scalar) {
            x = static_cast<int>(texture->width * scalar);
        }
    }


    ~BackgroundScroll() override {
        delete this->texture;
    }
};