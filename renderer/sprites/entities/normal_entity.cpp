//
// Created by GiaKhanhVN on 4/1/2025.
//

#include <cmath>
#include "normal_entity.h"
#include "../../sdl_components.h"
#include "../../spritesystem/particles.h"
#include "../../tetris_player.h"

void NormalEntity::moveSmooth(const int targetX, const int targetY, const function<void()> onComplete, const int speed_) {
    targetMoveX = targetX;
    targetMoveY = targetY;
    this->speed = speed_;
    this->onMovedComplete = onComplete;

    cout << "moving to " << targetX << " " << targetY << endl;
    cout << "scheduled " << (onMovedComplete != nullptr) << endl << endl;

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

void NormalEntity::attackPlayer(const void* p) {
    TetrisPlayer* target = ((TetrisPlayer*) p);
    auto curLoc = target->getLocation();
    auto toReturn = this->getLocation();
    auto lastLane = target->currentLane;

    this->isAttacking = true;

    // warn the player that the entity is attacking
    scheduleAnimation(ENTITY_APPROACH, [&, target, curLoc, toReturn, lastLane]() {
        // move there and attack
        moveSmooth(curLoc.x, curLoc.y, [&, target, toReturn, lastLane]() {
            // call the parent's damage method
            target->inflictDamage(static_cast<int>(randomFloat(this->damageBounds[0], this->damageBounds[1])), lastLane);
            cout << "lock released 1" << endl;

            // attack "animation"
            // return to the spawn point
            moveSmooth(toReturn.x, toReturn.y, [&]() {
                this->isAttacking = false;
                cout << "lock released" << endl;
            }, 10);
        }, 15);
    }, 40);
}

void NormalEntity::scheduleAnimation(int animation, function<void()> toRunLater, int fs) {
    setAnimation(animation, 0);
    this->animationAfterAttackAnimation = toRunLater;
    frameSpeed = fs; // slows it down/speed it up idc
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

    // play hurt animation
    flipSprite(SDL_FLIP_NONE);
    scheduleAnimation(ENTITY_DAMAGED, [&]() {
        setAnimation(ENTITY_IDLE);
        flipSprite(SDL_FLIP_HORIZONTAL);
    }, 15);

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
    // kill the entity (and clear up garbage too)
    this->discard();
}

void NormalEntity::onDrawCall() {
    internalClock++; // advance the sprite-bound clock

    // process move task
    processMove();
    // offset to render the sprite
    this->texture.textureX = this->originalTextureX + (128 * textureOffset);

    this->teleport(strictX, strictY);

    //advance animation frame if it's time
    if (internalClock % frameSpeed == 0) {
        textureOffset = (textureOffset + 1) % maxOffset;
    }

    // if at the end of the animation sequence, reset the frame speed to 5 and change to desired type
    if (textureOffset >= maxOffset - 1 && animationAfterAttackAnimation != nullptr) {
        this->animationAfterAttackAnimation();
        this->frameSpeed = 5;
        this->animationAfterAttackAnimation = nullptr;
    }

    // only count in seconds
    if (internalClock % 60 == 0) { // every second (16.6MS)
        attackDelayCounter++;
        // check if it's time to perform the attack
        if (attackDelayCounter >= (minAttackInterval + rand() % (maxAttackInterval - minAttackInterval))) {
            attackPlayer(pTetrisPlayer);
            attackDelayCounter = 0; // reset the counter
        }
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

void NormalEntity::setAnimation(const int animation, const int startFrame) {
    // Reset state
    textureOffset = startFrame;
    internalClock = 0; // reset the clock & speed offset
    frameSpeed = 5;

    switch (animation) {
        case ENTITY_IDLE:
            this->texture.textureY = IDLE_FRAME_Y;
            maxOffset = 5; // 8 sprites
            break;

        case ENTITY_DAMAGED:
            this->texture.textureY = FORWARD_FRAME_Y;
            maxOffset = 3; // 4 sprites
            break;

        case ENTITY_APPROACH:
            this->texture.textureY = BACKWARD_FRAME_Y;
            maxOffset = 2; // 4 sprites
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

        cout << "moved to: " << targetMoveX << " " << targetMoveY << endl;

        targetMoveX = -1;
        targetMoveY = -1;

        if (onMovedComplete != nullptr) {
            auto onMovedFunctionCopy = onMovedComplete;
            onMovedComplete = nullptr;
            onMovedFunctionCopy();
        }
        return;
    }

    float moveX = (dx / distance) * speed;
    float moveY = (dy / distance) * speed;
    strictX += moveX;
    strictY += moveY;
}