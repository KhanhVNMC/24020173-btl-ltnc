//
// Created by GiaKhanhVN on 4/1/2025.
//

#include <cmath>
#include "normal_entity.h"

void NormalEntity::moveSmooth(const int targetX, const int targetY, const function<void()> &onComplete, const int speed_) {
    targetMoveX = targetX;
    targetMoveY = targetY;
    this->speed = speed_;
    this->onMovedComplete = onComplete;

    setAnimation(ENTITY_IDLE);
}

void NormalEntity::onDrawCall() {
    processMove();
    // offset to render the sprite
    this->texture.textureX = this->originalTextureX + (128 * textureOffset);

    this->teleport(strictX, strictY);

    //advance animation frame if it's time
    if (SpritesRenderingPipeline::renderPasses() % frameSpeed == 0) {
        textureOffset = (textureOffset + 1) % maxOffset;
    }
}

void NormalEntity::setAnimation(const int animation) {
    // Reset state
    textureOffset = 0;
    frameSpeed = 5;

    switch (animation) {
        case ENTITY_IDLE:
            this->texture.textureY = IDLE_FRAME_Y;
            maxOffset = 4; // 8 sprites
            break;

        case ENTITY_DAMAGED:
            this->texture.textureY = FORWARD_FRAME_Y;
            maxOffset = 2; // 4 sprites
            break;

        case ENTITY_APPROACH:
            this->texture.textureY = BACKWARD_FRAME_Y;
            maxOffset = 3; // 4 sprites
            break;

        default:
            break;
    }
}

void NormalEntity::attackAnimation() {
    setAnimation(ATTACK_01);
}

void NormalEntity::processMove() {
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