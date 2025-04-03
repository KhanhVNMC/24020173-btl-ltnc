//
// Created by GiaKhanhVN on 4/1/2025.
//

#include <cmath>
#include "normal_entity.h"
#include "../../sdl_components.h"
#include "../../spritesystem/particles.h"

void NormalEntity::moveSmooth(const int targetX, const int targetY, const function<void()> &onComplete, const int speed_) {
    targetMoveX = targetX;
    targetMoveY = targetY;
    this->speed = speed_;
    this->onMovedComplete = onComplete;

    setAnimation(ENTITY_IDLE);
}

void NormalEntity::onBeforeTextureDraw(SDL_Texture *texture) {
    // if the player just took damage, glow them for 10 frames and flash between red & nothing
    if (const long renderPasses = SpritesRenderingPipeline::renderPasses(); glowRedUntil > renderPasses) {
        if (renderPasses % 6 == 0) {
            SDL_SetTextureColorMod(texture, 0xFF, 0xFF, 0xFF);
            return;
        }
        SDL_SetTextureColorMod(texture, 0xFF, 0, 0);
    }
}

KillRewards NormalEntity::damageEntity(int damage) {
    glowRedUntil = SpritesRenderingPipeline::renderPasses() + 20; // 10 frames
    setHealth(currentHealth - damage);

    // blood animation
    SpriteTexture particleTex = {287, 0, 16, 18};
    ParticleSystem* ps = new ParticleSystem(
            particleTex, 12, 12,
            0, 20,
            -6.0, 6.0,
            -15.0, -5.0,
            60, 0.5
    );
    ps->once = true;
    ps->teleport(strictX + 120, strictY + 120); // Set emitter position
    ps->spawn();

    if (currentHealth <= 0) {
        // the entity has been killed
        KillRewards rewards = { rand() % 2, (rand() % 6) + 1 };
        die(rewards.type == 0);
        return rewards; // 1->5
    }
    return {-1, -1}; // not killed
}

void NormalEntity::die(bool isArmor) {
    this->isDead = true;
    this->texture.width = 0;
    this->texture.height = 0;
    // death particles (+armor) or (+charge)
    SpriteTexture particleTex = isArmor ? SpriteTexture{304, 0, 17, 17} : SpriteTexture{322, 0, 17, 17};
    ParticleSystem* ps = new ParticleSystem(
            particleTex, 24, 24,
            0, 10,
            -4.0, 4.0,
            -5.0, -2.0,
            60, 0.25
    );
    ps->once = true;
    ps->teleport(strictX + 120, strictY + 120); // Set emitter position
    ps->spawn();
    // kill the entity
    this->discard();
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

void NormalEntity::onDrawCallExtended(SDL_Renderer *renderer) {
    // render health icon
    auto cached = disk_cache::bmp_load_and_cache(renderer, "../assets/SPRITES.bmp");
    const struct_render_component component = {
            0, 0, 18, 18,
            strictX, strictY, static_cast<int>(18 * 1.25), static_cast<int>(18 * 1.25)
    };
    render_component(renderer, cached, component, 1);

    // render health bar
    SDL_SetRenderDrawColor(renderer, 79, 79, 79, 255); // gray
    SDL_Rect underlay = { strictX + 40, strictY, 200, 20 };
    SDL_RenderFillRect(renderer, &underlay);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
    SDL_Rect health = { strictX + 40, strictY, static_cast<int>(ceil(200.0 * ((double)currentHealth / maxHealthPoints))), 20 };
    SDL_RenderFillRect(renderer, &health);

    SDL_SetRenderDrawColor(renderer, 0,0,0,255); // black
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