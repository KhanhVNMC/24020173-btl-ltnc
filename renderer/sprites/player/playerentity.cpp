//
// Created by GiaKhanhVN on 4/1/2025.
//

#include <cmath>
#include "playerentity.h"

void FlandreScarlet::moveSmooth(const int targetX, const int targetY, const function<void()> &onComplete, const int speed_) {
    targetMoveX = targetX;
    targetMoveY = targetY;
    this->speed = speed_;
    this->onMovedComplete = onComplete;

    if (targetMoveX == strictX && targetMoveY != strictY) {
        setAnimation(targetMoveY < strictY ? RUN_FORWARD : RUN_BACKWARD);
        if (targetMoveY < strictY) {
            rotate(340);
        }
        return;
    };
    if (targetMoveX > strictX) {
        setAnimation(RUN_FORWARD);
    } else {
        setAnimation(RUN_BACKWARD);
    }
}

void FlandreScarlet::onDrawCall() {
    processMove();
    // offset to render the sprite
    this->texture.textureX = this->originalTextureX + (128 * textureOffset);

    // sinusoidal
    this->teleport(strictX, static_cast<int>(strictY + (std::sin(SpritesRenderingPipeline::renderPasses() / 20.0) * 5)));

    //advance animation frame if it's time
    if (SpritesRenderingPipeline::renderPasses() % frameSpeed == 0) {
        textureOffset = (textureOffset + 1) % maxOffset;
    }

    // if just attacked, reset the frame speed to 5 and change to desired type
    if (textureOffset >= maxOffset - 1 && animationAfterAttackAnimation != nullptr) {
        this->animationAfterAttackAnimation();
        this->frameSpeed = 5;
        this->animationAfterAttackAnimation = nullptr;
    }
}

void FlandreScarlet::onBeforeTextureDraw(SDL_Texture *texture) {
    // if the player just took damage, glow them for 10 frames and flash between red & nothing
    if (const long renderPasses = SpritesRenderingPipeline::renderPasses(); glowRedUntil > renderPasses) {
        if (renderPasses % 6 == 0) {
            SDL_SetTextureColorMod(texture, 0xFF, 0xFF, 0xFF);
            return;
        }
        SDL_SetTextureColorMod(texture, 0xFF, 0, 0);
    }
}

void FlandreScarlet::setAnimation(const int animation) {
    // Reset state
    textureOffset = 0;
    frameSpeed = 5;

    // Set height for attack animations
    if (animation >= ATTACK_01) {
        this->height = 60;
        this->rotate(-45);
        this->scale(4.75); // make it slightly bigger
    } else {
        this->height = 45;
        this->texture.height = DEFAULT_SPRITE_H;
        this->texture.width = DEFAULT_SPRITE_W;
        // reset the flipping state
        flipSprite(this->initialFlip); // reset
        this->scale(4); // no scale (back to original)
        this->rotate(0);
    }

    switch (animation) {
        case IDLE:
            this->texture.textureY = IDLE_FRAME_Y;
            maxOffset = 8; // 8 sprites
            break;

        case RUN_FORWARD:
            this->texture.textureY = FORWARD_FRAME_Y;
            maxOffset = 4; // 4 sprites
            break;

        case RUN_BACKWARD:
            this->texture.textureY = BACKWARD_FRAME_Y;
            maxOffset = 4; // 4 sprites
            break;

        case ATTACK_01:
            this->texture.textureY = ATTACK_1_FRAME_Y;
            this->texture.height = ATTACK_SPRITE_H;
            this->texture.width = ATTACK_SPRITE_W;
            maxOffset = 4; // 4 sprites
            flipSprite(SDL_FLIP_NONE);
            break;

        default:
            break;
    }
}

void FlandreScarlet::scheduleAnimation(int animation, function<void()> toRunLater, int fs) {
    setAnimation(animation);
    this->animationAfterAttackAnimation = toRunLater;
    frameSpeed = fs; // slows it down/speed it up idc
}

void FlandreScarlet::damagedAnimation() {
    glowRedUntil = SpritesRenderingPipeline::renderPasses() + 20; // 10 frames
}

void FlandreScarlet::processMove() {
    if (targetMoveX == -1 && targetMoveY == -1) return;
    float dx = targetMoveX - strictX;
    float dy = targetMoveY - strictY;
    float distance = sqrt(dx * dx + dy * dy); // euclid

    if (distance <= speed) {
        strictX = targetMoveX;
        strictY = targetMoveY;
        targetMoveX = -1;
        targetMoveY = -1;
        if (onMovedComplete) {
            onMovedComplete();
            onMovedComplete = nullptr;
        }
        return;
    }

    float moveX = (dx / distance) * speed;
    float moveY = (dy / distance) * speed;
    strictX += moveX;
    strictY += moveY;
}