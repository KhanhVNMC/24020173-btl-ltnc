//
// Created by GiaKhanhVN on 3/3/2025.
//
#include "../sbg.h"
#include "spritesystem/sprite.h"
#include "sdl_inc.h"
#include "entities/bigblackbox.cpp"

#ifndef TETRIS_PLAYER_CPP
#define TETRIS_PLAYER_CPP

class TetrisPlayer {
    // internal engine
    TetrisEngine* tetrisEngine;
    queue<int> garbageQueue;

    long long firstPiecePlacedTime = -1;
    int piecesPlaced = 0;

    long firstDamageInflictedTime = -1;
    int totalDamage = 0;

    // animation & player entity
    FlandreScarlet* flandre;
    SDL_Renderer* renderer;
    typedef enum {
        IDLE,
        RUN_FORWARD,
        RUN_BACKWARD,
    } Animation;
public:
    TetrisPlayer(SDL_Renderer* renderer_, TetrisEngine* engine) {
        this->renderer = renderer_;
        this->tetrisEngine = engine;

        this->flandre = new FlandreScarlet();
        this->flandre->teleportStrict(780, 250);
        this->flandre->setAnimation(RUN_FORWARD);
        this->flandre->spawn();

        this->tetrisEngine->runOnTickEnd([&] { onTetrisTick(); });
        // hook into events
        this->tetrisEngine->runOnMinoLocked([&](int mino) {
            if (firstPiecePlacedTime == -1) {
                firstPiecePlacedTime = System::currentTimeMillis();
            }
            this->piecesPlaced++;
        });

        // boot the engine up
        this->tetrisEngine->start();
    }

    [[maybe_unused]] void sprintfcdbg(TetrisEngine* tetris, int spriteCount);
    void process_input(SDL_Event& event, TetrisEngine* engine);

    void renderTetrisInterface(const int ox, const int oy) {
        render_tetris_board(ox, oy, renderer, this->tetrisEngine);

        // render PPS
        char ppsString[16];
        double secondsElapsed = (System::currentTimeMillis() - firstPiecePlacedTime) / 1000.0;
        snprintf(ppsString, sizeof(ppsString), "%.2f/s", firstPiecePlacedTime == -1 ? 0.0 : piecesPlaced / secondsElapsed);

        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 147, oy + (Y_OFFSET / 2) + 425, "pieces", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 145, oy + (Y_OFFSET / 2) + 450, ppsString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 25, oy + (Y_OFFSET / 2) + 440, std::to_string(piecesPlaced) + ".", 2.75, 1, 22, 12);

        // render APM
        char apmString[16];
        double minutesElapsed = ((System::currentTimeMillis() - firstDamageInflictedTime) / 1000.0) / 60.0;
        snprintf(apmString, sizeof(apmString), "%.2f/m", firstDamageInflictedTime == -1 ? 0.0 : totalDamage / minutesElapsed);

        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 147, oy + (Y_OFFSET / 2) + 425 + 70, "attacks", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 140, oy + (Y_OFFSET / 2) + 450 + 70, apmString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 20, oy + (Y_OFFSET / 2) + 440 + 70, std::to_string(totalDamage) + ".", 2.75, 1, 22, 12);
    }

    SDL_Event event;
    void onTetrisTick() {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(1);
            }
            process_input(event, this->tetrisEngine);
        }

        SDL_RenderClear(renderer);
        SpritesRenderingPipeline::renderEverything(renderer);
        renderTetrisInterface(100, 50);

        sprintfcdbg(this->tetrisEngine, SpritesRenderingPipeline::getSprites().size());
        SDL_RenderPresent(renderer); // Show updated frame
    }
};
#endif
