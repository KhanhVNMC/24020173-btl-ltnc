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

    long long firstDamageInflictedTime = -1;
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
        this->tetrisEngine->onPlayfieldEvent([&](PlayfieldEvent event) { playFieldEvent(event); });

        // boot the engine up
        this->tetrisEngine->start();
    }

    [[maybe_unused]] void sprintfcdbg(TetrisEngine* tetris, int spriteCount);
    void process_input(SDL_Event& event, TetrisEngine* engine);

    void onDamageSend(const int damage) {
        if (firstDamageInflictedTime == -1) {
            firstDamageInflictedTime = System::currentTimeMillis();
        }
        totalDamage += damage;
    }

    int currentBackToBack;
    void playFieldEvent(const PlayfieldEvent& event) {
        const int cleared = (int)event.getLinesCleared().size();
        // Check for back-to-back events
        if (cleared >= 4 || ((event.isSpin() || event.isMiniSpin()) && cleared > 0)) {
            currentBackToBack++; // Increase back-to-back count
            //if (currentBackToBack >= 1) setTextFor(this.backToBackInd, "&6&lB2B x" + this.currentBackToBack); // Update the back-to-back indicator
        } else if (cleared > 0 && currentBackToBack > 0) {
            currentBackToBack = -1; // Reset back-to-back count
            //setTextFor(this.backToBackInd, "&c&lB2B x0"); // Update the back-to-back indicator
            //engine.scheduleDelayedTask(30, () -> setTextFor(this.backToBackInd, "")); // Clear back-to-back indicator after delay
        }
        // calculate damage throughput
        int baseDamage = cleared >= 4 ? cleared : (cleared <= 1 ? 0 : (cleared == 2 ? 1 : 2));
        // spin bonus
        if (event.isSpin()) {
            baseDamage = cleared * 2;
        }
        // b2b bonus
        baseDamage += max(0, currentBackToBack);
        // combo bonus (primitive)
        baseDamage += static_cast<int>((tetrisEngine->getComboCount()) * 0.5);
        onDamageSend(baseDamage);
    }

    void renderTetrisInterface(const int ox, const int oy) {
        render_tetris_board(ox, oy, renderer, this->tetrisEngine);

        // render PPS
        char ppsString[16];
        const double secondsElapsed = (System::currentTimeMillis() - firstPiecePlacedTime) / 1000.0;
        snprintf(ppsString, sizeof(ppsString), "%.2f/s", firstPiecePlacedTime == -1 ? 0.0 : piecesPlaced / secondsElapsed);

        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 147, oy + (Y_OFFSET / 2) + 425, "pieces", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 145, oy + (Y_OFFSET / 2) + 450, ppsString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 25, oy + (Y_OFFSET / 2) + 440, std::to_string(piecesPlaced) + ".", 2.75, 1, 22, 12);

        // render APM
        char apmString[16];
        const double minutesElapsed = ((System::currentTimeMillis() - firstDamageInflictedTime) / 1000.0) / 60.0;
        const double apm = firstDamageInflictedTime == -1 ? 0.0 : totalDamage / minutesElapsed;
        snprintf(apmString, sizeof(apmString), apm >= 100 ? "%.1f/m" : "%.2f/m", apm);

        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 147, oy + (Y_OFFSET / 2) + 425 + 70, "attacks", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + 140, oy + (Y_OFFSET / 2) + 450 + 70, apmString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, ox - (MINO_SIZE) + (apm >= 10 ? 0 : 20), oy + (Y_OFFSET / 2) + 440 + 70, std::to_string(min(9999, totalDamage)) + ".", 2.75, 1, 22, 12);
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

        sprintfcdbg(this->tetrisEngine, static_cast<int>(SpritesRenderingPipeline::getSprites().size()));
        SDL_RenderPresent(renderer); // Show updated frame
    }
};
#endif
