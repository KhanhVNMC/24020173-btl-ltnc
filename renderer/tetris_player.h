//
// Created by GiaKhanhVN on 3/3/2025.
//
#include "../sbg.h"
#include "spritesystem/sprite.h"
#include "sdl_inc.h"
#include "entities/bigblackbox.cpp"

#ifndef TETRIS_PLAYER_CPP
#define TETRIS_PLAYER_CPP

static int TETRIS_SCORE[5] = { 0,40, 100, 300, 1200 }; // score for each line clears
static int LEVEL_THRESHOLD = 10; // advance every X levels
static double LEVELS_GRAVITY[16] = { // speed of each level
        0, // lvl 0 does not exist
        0.01667,
        0.021017,
        0.026977,
        0.035256,
        0.04693,
        0.06361,
        0.0879,
        0.1236,
        0.1775,
        0.2598,
        0.388,
        0.59,
        0.92,
        1.46,
        2.36,
};

class TetrisPlayer {
    // internal engine
    TetrisEngine* tetrisEngine;
    queue<int> garbageQueue;
    long long tetrisScore = 0;
    int currentTetrisLevel = 0;

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

        // init gravity to lvl 1
        updateLevelAndGravity(1);

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

    void updateLevelAndGravity(int newLevel) {
        currentTetrisLevel = newLevel;
        // increase engine gravity
        this->tetrisEngine->getCurrentConfig()->setGravity(LEVELS_GRAVITY[min(15, currentTetrisLevel)]);
        this->tetrisEngine->updateMutableConfig();
    }

    int clearedLines = 0; // total cleared lines
    int currentBackToBack = 0; // keep track of back to back(s)
    void playFieldEvent(const PlayfieldEvent& event) {
        const int cleared = (int)event.getLinesCleared().size();

        // CALCULATE DAMAGE AND SCORE
        // update the amount of cleared lines
        clearedLines += cleared;

        // calculate classic tetris scores (ONLY if cleared > 0)
        if (cleared >= 5) tetrisScore += 2460; // edge case
        else if (cleared > 0) {
            int score = (TETRIS_SCORE[event.isSpin() ? 4 : cleared] * currentTetrisLevel) * (event.isSpin() && cleared == 3 ? 1.5 : 1);
            score += tetrisEngine->getComboCount() * 5;
            score += max(0, currentBackToBack) * 10;

            tetrisScore += score;
            int newLevel = 1 + static_cast<int>(clearedLines / LEVEL_THRESHOLD);
            if (newLevel <= 15 && newLevel != currentTetrisLevel) {
                updateLevelAndGravity(newLevel);
            }
        }

        // check for back-to-back events
        if (cleared >= 4 || ((event.isSpin() || event.isMiniSpin()) && cleared > 0)) {
            currentBackToBack++; // increase back-to-back count
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

        // fire event
        if (baseDamage > 0) onDamageSend(baseDamage);
    }

    #define Y_OFFSET_STATISTICS 65
    void renderTetrisStatistics(const int ox, const int oy) {
        const int GRID_X_OFFSET = ox - (MINO_SIZE);
        const int GRID_Y_OFFSET = oy + (Y_OFFSET / 2);
        // begin render statistics

        // render PPS
        const double secondsElapsed = (System::currentTimeMillis() - firstPiecePlacedTime) / 1000.0;
        auto ppsString = str_printf("%.2f/s", firstPiecePlacedTime == -1 ? 0.0 : piecesPlaced / secondsElapsed);

        // render the text that tells PPS (PIECES PER SECOND)
        /*
         * TIME
         * 0. 0.00/S
         * [TOTAL PISSES]  [PPS]/S
         */
        render_component_string_rvs(renderer, GRID_X_OFFSET + 147, GRID_Y_OFFSET + 405, "pieces", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, GRID_X_OFFSET + 145, GRID_Y_OFFSET + 430, ppsString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, GRID_X_OFFSET + 25, GRID_Y_OFFSET + 420, std::to_string(piecesPlaced) + ".", 2.75, 1, 22, 12);

        // render APM
        const double minutesElapsed = ((System::currentTimeMillis() - firstDamageInflictedTime) / 1000.0) / 60.0;
        const double apm = firstDamageInflictedTime == -1 ? 0.0 : totalDamage / minutesElapsed;
        auto apmString = str_printf(apm >= 100 ? "%.1f/m" : "%.2f/m", apm);

        // render the text that tells APM (attacks per minute)
        /*
         * TIME
         * 0. 0.00/M
         * [TOTAL DAMAGE]  [APM]/M
         */
        render_component_string_rvs(renderer, GRID_X_OFFSET + 147, GRID_Y_OFFSET + 405 + Y_OFFSET_STATISTICS, "attacks", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, GRID_X_OFFSET + 140, GRID_Y_OFFSET + 430 + Y_OFFSET_STATISTICS, apmString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, GRID_X_OFFSET + (apm >= 10 ? 0 : 20), GRID_Y_OFFSET + 420 + Y_OFFSET_STATISTICS, std::to_string(min(9999, totalDamage)) + ".", 2.75, 1, 22, 12);

        // render time passed
        const int64_t timePassedMs = System::currentTimeMillis() - tetrisEngine->startedAt;
        const int64_t timePassedS  = timePassedMs / 1000;

        const int64_t displayMs    = timePassedMs % 1000;
        const int64_t displayMin   = timePassedS / 60;
        const int64_t displaySec   = timePassedS % 60;

        auto timeString      = str_printf("%02d:%02d", displayMin, displaySec);
        auto msString        = str_printf(".%03d", displayMs);

        // render the text that tells time, preview below
        /*
         * TIME
         * 00:00 .000
         * MM:SS .-MS
         */
        render_component_string_rvs(renderer, GRID_X_OFFSET + 147, GRID_Y_OFFSET + 405 + 2 * Y_OFFSET_STATISTICS, "time", 1.55, 1, 15, 14);
        render_component_string_rvs(renderer, GRID_X_OFFSET + 140, GRID_Y_OFFSET + 430 + 2 * Y_OFFSET_STATISTICS, msString, 2, 1, 17, 12);
        render_component_string_rvs(renderer, GRID_X_OFFSET + 60, GRID_Y_OFFSET + 420 + 2 * Y_OFFSET_STATISTICS, timeString, 2.75, 1, 22, 12);

        // render VS Score
        auto scoreString = str_printf("%012d", min(99999999999L, tetrisScore)); // limit to 12 chars
        render_component_string(renderer, GRID_X_OFFSET + 170, GRID_Y_OFFSET + 605, scoreString, 3, 1, 27, 12);

        // render speed lvl
        auto levelString = str_printf("%02d/15", currentTetrisLevel); // limit to 12 chars
        render_component_string(renderer, GRID_X_OFFSET + 500, GRID_Y_OFFSET + 380 + 2 * Y_OFFSET_STATISTICS, "speed lvl", 1.55, 1, 15, 14);
        render_component_string(renderer, GRID_X_OFFSET + 500, GRID_Y_OFFSET + 405 + 2 * Y_OFFSET_STATISTICS, levelString, 2, 1, 17, 12);
    }

    void renderTetrisInterface(const int ox, const int oy) {
        // render the board body first
        render_tetris_board(ox, oy, renderer, this->tetrisEngine);
        // and then the statistics
        renderTetrisStatistics(ox, oy);
    }

    SDL_Event _event;
    void onTetrisTick() {
        while (SDL_PollEvent(&_event)) {
            if (_event.type == SDL_QUIT) {
                exit(1);
            }
            process_input(_event, this->tetrisEngine);
        }

        SDL_RenderClear(renderer);
        SpritesRenderingPipeline::renderEverything(renderer);
        renderTetrisInterface(static_cast<int>(100 + (sin(tetrisEngine->ticksPassed / 2.0) * 3)), 50);

        sprintfcdbg(this->tetrisEngine, static_cast<int>(SpritesRenderingPipeline::getSprites().size()));
        SDL_RenderPresent(renderer); // Show updated frame
    }
};
#endif
