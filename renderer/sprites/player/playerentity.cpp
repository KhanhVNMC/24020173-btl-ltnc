//
// Created by GiaKhanhVN on 4/1/2025.
//

#include "playerentity.h"

void FlandreScarlet::moveSmooth(const int targetX, const int targetY, const function<void()> &onComplete, const int speed_) {
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

void FlandreScarlet::setAnimation(const int animation) {
    // Reset state
    textureOffset = 0;
    frameSpeed = 5;

    // Set height for attack animations
    if (animation >= ATTACK_01) {
        this->height = 60;
    } else {
        this->height = 50;
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

void FlandreScarlet::attackAnimation() {
    setAnimation(ATTACK_01);
}

void FlandreScarlet::processMove() {
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