//
// Created by GiaKhanhVN on 4/7/2025.
//
#include "tetris_player.h"

void TetrisPlayer::setDebuff(Debuff type, bool value) {
    switch (type) {
        case BLIND: sBlinded = value; break;
        case NO_HOLD: {
            if (sNoHold == value) break;
            sNoHold = value;
            this->tetrisEngine->getCurrentConfig()->setHoldEnabled(!value);
            this->tetrisEngine->updateMutableConfig(sSuperSonic);
            break;
        }
        case SUPER_SONIC: {
            if (sSuperSonic == value) break;
            sSuperSonic = value;
            this->tetrisEngine->updateMutableConfig(value);
            break;
        }
        case WEAKNESS: sWeakness = value; break;
        case FRAGILE: sFragile = value; break;
    }
}

TetrisPlayer::TetrisPlayer(ExecutionContext* context, SDL_Renderer* sdlRenderer, TetrisEngine* engine) {
    // register constants
    this->renderer = sdlRenderer;
    this->tetrisEngine = engine;
    this->context = context;

    // register the representative player entity
    this->flandre = new FlandreScarlet();
    this->flandre->teleportStrict(X_LANE_PLAYER, Y_LANES[currentLane]);
    this->flandre->setAnimation(RUN_FORWARD);
    this->flandre->spawn();

    // hook into events
    this->tetrisEngine->runOnTickEnd([&] { onTetrisTick(); });
    this->tetrisEngine->runOnMinoLocked([&](int cleared) {
    // if first piece, mark this as the first time
    if (firstPiecePlacedTime == -1) {
    firstPiecePlacedTime = System::currentTimeMillis();
    }
    //
    this->piecesPlaced++;
    this->onMinoLocked(cleared);
    });
    this->tetrisEngine->onComboBreaks([&](const int combo) { });
    this->tetrisEngine->onPlayfieldEvent([&](const PlayfieldEvent& event) { playFieldEvent(event); });

    // init gravity to lvl 1
    updateLevelAndGravity(1);
}

void TetrisPlayer::initParallaxBackground() {
    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_NONE, 0, 0, 2)); // first layer
    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_NONE, 2048, 0, 2)); // first layer follow up

    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_HORIZONTAL, 0, 0, 3));
    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_HORIZONTAL, 2048, 0, 3));
}

void TetrisPlayer::startEngineAndGame() {
    // boot the engine up
    this->tetrisEngine->scheduleDelayedTask(60, [&]() {
        this->tetrisEngine->gameInterrupt(true);
    });
    this->tetrisEngine->gameInterrupt(false);

    // start the engine without using this thread, as we will take over the execution context
    // right below
    this->tetrisEngine->start(true);

    // take over the execution context
    //tetrisEngineExecId = context->hook([&]() { this->tetrisEngine->gameLoopBody(); });
}

TetrisPlayer::~TetrisPlayer() {
    context->unhook(tetrisEngineExecId);
    flandre->discard();
    for (BackgroundScroll* parallax : parallaxBackgrounds) {
        if (parallax != nullptr) parallax->discard();
    }
    for (int i = 0; i < 4; i++) {
        if (enemyOnLanes[i] != nullptr) enemyOnLanes[i]->discard();
    }
    delete tetrisEngine; // UH
}

void TetrisPlayer::spawnEnemyOnLane(int lane, NormalEntity *entity) {
    enemyOnLanes[lane] = entity;
    // spawn hidden
    entity->teleportStrict(X_LANE_ENEMIES + 200, Y_LANES_ENEMIES[lane]);
    // move slowly to its designated position
    entity->moveSmooth(X_LANE_ENEMIES, Y_LANES_ENEMIES[lane], [&, entity]() {
        // once arrive, this mob is ready to be fucked
        entity->isSpawning = false;
    });
    entity->setAnimation(ENTITY_IDLE);
    entity->spawn();
}
