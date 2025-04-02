//
// Created by GiaKhanhVN on 4/1/2025.
//

#ifndef TETISENGINE_ENTITY_PROP_H
#define TETISENGINE_ENTITY_PROP_H

typedef enum {
    IDLE,
    RUN_FORWARD,
    RUN_BACKWARD,
    ATTACK_01,
} PlayerAnimation;

typedef enum {
    ENTITY_IDLE,
    ENTITY_APPROACH,
    ENTITY_DAMAGED
} NormalAnimation;

typedef enum {
    BOSS_IDLE,
    BOSS_APPROACH,
    BOSS_ATTACK,
    BOSS_SKILL,
    BOSS_DAMAGED
} BossAnimation;

static constexpr int IDLE_FRAME_Y = 41;
static constexpr int FORWARD_FRAME_Y = 136;
static constexpr int BACKWARD_FRAME_Y = 225;
static constexpr int ATTACK_1_FRAME_Y = 312;

static constexpr int ATTACK_SPRITE_H = 128;
static constexpr int ATTACK_SPRITE_W = 115;

static constexpr int DEFAULT_SPRITE_W = 110;
static constexpr int DEFAULT_SPRITE_H = 93;

#endif //TETISENGINE_ENTITY_PROP_H
