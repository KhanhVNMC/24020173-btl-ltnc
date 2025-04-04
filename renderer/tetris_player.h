//
// Created by GiaKhanhVN on 3/3/2025.
//
#include "tetris_renderer.h"
#include "sprites/gameworld.cpp"
#include "sprites/entities/Redgga.h"
#include "sprites/player/playerentity.h"
#include "sprites/entity_prop.h"
#ifndef TETRIS_PLAYER_H
#define TETRIS_PLAYER_H

static int TETRIS_SCORE[5] = { 0, 50, 110, 630, 2300 }; // score for each type of line clears
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

static int Y_LANES[4] = {
        10, 190, 380, 550
};

static int Y_LANES_ENEMIES[4] = {
        30, 220, 400, 580
};

static string CLEAR_MESSAGES[5] = {
        "", "single", "double", "triple", "tetris"
};

static int TETRIS_COLORS[6] = {
        0xFF0000, // red
        0xFFB71C, // orange
        0xFFEA00, // yellow
        0x00FF51, // green
        0x00FFFF, // aqua
        0xEE00FF, // purple
};

static int MINO_COLORS[7] = {
        0xEE00FF, // T Mino
        0xFF0000, // Z Mino
        0x00FF51, // S mino
        0xFFB71C, // L mino
        0x008cff,// J mino
        0x00FFFF, // I moino
        0xFFEA00, // O mono

};

#define X_LANE_PLAYER 780
#define X_LANE_ENEMIES 1400

class TetrisPlayer {
    static void spawnPhysicsBoundText(string str, int x, int y, double randVelX, double randVelY, int lifetime, double gravity, double scalar, int strgap, int width, const int* colors = nullptr, const int applyThisColorToAll = -1) {
        for (int i = 0; i < str.length(); i++) {
            auto [source, dest] = puts_component_char(x + (strgap * i), y, scalar, str[i], width);
            const auto part = new Particle(
                    // texture
                    {source.x, source.y, source.w, source.h},
                    // destination
                    dest.w, dest.h, dest.x, dest.y,
                    randVelX, randVelY, lifetime, gravity
            );
            part->setTextureFile("../assets/font.bmp");
            if (applyThisColorToAll != -1) part->setTint(applyThisColorToAll);
            else if (colors != nullptr) part->setTint(colors[i]);
            part->spawn();
        }
    }

    static void spawnDamageIndicator(const int x, const int y, const int damage, bool offensive) {
        const string damageStr = std::to_string(damage);
        const double scalar = 3;
        const int strgap = 40, width = 18;
        const double randDmgVelX = randomFloat(-2, 4), randDmgVelY = -randomFloat(-1, 2);

        spawnPhysicsBoundText(damageStr, x, y, randDmgVelX, randDmgVelY, 80, 0.05, scalar, strgap, width, nullptr, offensive ? MINO_COLORS[6] : MINO_COLORS[1]);
    }

    static void spawnBoardTitle(const int x, const int y, string title, const int* colors = nullptr) {
        spawnPhysicsBoundText(title, x, y, -0.1, 0, 60, 0.03, 2.5, 30, 15, colors);
    }

    static void spawnBoardSubtitle(const int x, const int y, string title, const int color) {
        spawnPhysicsBoundText(title, x, y, -0.1, 0, 60, 0.03, 2, 20, 15, nullptr, color);
    }

    static void spawnBoardMiniSubtitle(const int x, const int y, string title, const int color) {
        spawnPhysicsBoundText(title, x, y, -0.1, 0, 60, 0.03, 1.5, 20, 15, nullptr, color);
    }

    static void spawnMiscIndicator(const int x, const int y, string indicator, const int color) {
        spawnPhysicsBoundText(indicator, x, y, 0.5, 0, 60, 0.03, 2, 20, 15, nullptr, color);
    }

    static void renderThunderbolt(SDL_Renderer* renderer, const int x, const int y, const int offset) {
        auto cached = disk_cache::bmp_load_and_cache(renderer, "../assets/SPRITES.bmp");
        const struct_render_component component = {
                58 + (23 * offset), 0, 22, 30,
                x, y, static_cast<int>(22 * 1.25), static_cast<int>(30 * 1.25)
        };
        render_component(renderer, cached, component, 1);
    }

    static void renderArmor(SDL_Renderer* renderer, const int x, const int y, const int offset) {
        auto cached = disk_cache::bmp_load_and_cache(renderer, "../assets/SPRITES.bmp");
        const struct_render_component component = {
                230 + (19 * offset), 0, 18, 18,
                x, y, static_cast<int>(18 * 1.25), static_cast<int>(18 * 1.25)
        };
        render_component(renderer, cached, component, 1);
    }

    // internal engine
    TetrisEngine* tetrisEngine;
    deque<int> garbageQueue;
    long long tetrisScore = 0;
    int currentTetrisLevel = 0;

    long long firstPiecePlacedTime = -1;
    int piecesPlaced = 0;

    long long firstDamageInflictedTime = -1;
    int totalDamage = 0;

    // animation & player entity
    FlandreScarlet* flandre;
    SDL_Renderer* renderer;

    // attack thingy
public:
    int currentLane = 0;
    int accumulatedCharge = 0;
    int currentArmorPoints = 10;

    bool isMovingToAnotherLane = false;
    bool isAttacking = false;

    NormalEntity* enemyOnLanes[4] = {nullptr, nullptr, nullptr, nullptr}; // 4 lanes, 4 available monsters (initialized as 0)

    TetrisPlayer(SDL_Renderer* renderer_, TetrisEngine* engine) {
        this->renderer = renderer_;
        this->tetrisEngine = engine;

        this->flandre = new FlandreScarlet();
        this->flandre->teleportStrict(X_LANE_PLAYER, Y_LANES[currentLane]);
        this->flandre->setAnimation(RUN_FORWARD);
        this->flandre->spawn();

        enemyOnLanes[0] = new Redgga();
        enemyOnLanes[0]->teleportStrict(X_LANE_ENEMIES, Y_LANES_ENEMIES[currentLane]);
        enemyOnLanes[0]->setAnimation(ENTITY_IDLE);
        enemyOnLanes[0]->spawn();

        this->tetrisEngine->runOnTickEnd([&] { onTetrisTick(); });
        // hook into events
        this->tetrisEngine->runOnMinoLocked([&](int mino) {
            if (firstPiecePlacedTime == -1) {
                firstPiecePlacedTime = System::currentTimeMillis();
            }
            this->piecesPlaced++;
            enemyOnLanes[0]->attackPlayer(this);
        });
        this->tetrisEngine->onComboBreaks([&](const int combo) { onComboBreaks(combo); });
        this->tetrisEngine->onPlayfieldEvent([&](const PlayfieldEvent& event) { playFieldEvent(event); });

        // init gravity to lvl 1
        updateLevelAndGravity(1);

        // boot the engine up
        this->tetrisEngine->start();
    }

    SpriteLoc getLocation() {
        return this->flandre->getLocation();
    }

    [[maybe_unused]] void sprintfcdbg(TetrisEngine* tetris, int spriteCount);
    void process_input(SDL_Event& event, TetrisEngine* engine);

    void moveToLane(const int targetLane) {
        if (this->isMovingToAnotherLane) return; // prevent overlapping
        this->isMovingToAnotherLane = true;
        this->currentLane = targetLane % 4; // prevent overshooting

        // move to specified location
        this->flandre->moveSmooth(780, Y_LANES[currentLane], [&]() {
            this->flandre->setAnimation(RUN_FORWARD);
            this->isMovingToAnotherLane = false;
            this->flandre->rotate(0);
        }, 10); // super-fast lane-switching
    }

    void addStats(bool isCharge, int amount) {
        if (isCharge) accumulatedCharge += amount;
        else currentArmorPoints += amount;
        // display the damage accumulated
        spawnMiscIndicator(isCharge ? 310 : 270, isCharge ? 10 : 25, "+" + std::to_string(amount), isCharge ? MINO_COLORS[5] : 0xc9c9c9);
    }

    void onDamageSend(const int damage) {
        if (firstDamageInflictedTime == -1) {
            firstDamageInflictedTime = System::currentTimeMillis();
        }
        totalDamage += damage;
        if (accumulatedCharge < 40) {
            addStats(true, damage);
        } else {
            // overflow, then send the entire shit away
            releaseDamageOnCurrentLane();
        }
    }

    int boardRumble = 0;
    void inflictDamage(int damage, int oldLane) {
        if (currentLane != oldLane) return;

        if (currentArmorPoints > 0) {
            if (currentArmorPoints == 1) {
                damage *= 0.75;
                --currentArmorPoints;
                spawnMiscIndicator(270, 55, "-1", 0xc9c9c9);
            } else {
                damage *= 0.5;
                currentArmorPoints -= 2;
                spawnMiscIndicator(270, 55, "-2", 0xc9c9c9);
            }
        }

        garbageQueue.push_back(damage);
        this->spawnDamageIndicator(getLocation().x + 40, getLocation().y + 20, damage, false);

        this->flandre->damagedAnimation();
        boardRumble = 10; // rumble for 10 frames
    }

    void releaseDamageOnCurrentLane() {
        if (isAttacking || isMovingToAnotherLane) return; // currently moving, do NOT attack
        const int finalDamage = accumulatedCharge;
        accumulatedCharge = 0; // reset charges

        if (enemyOnLanes[currentLane] == nullptr || enemyOnLanes[currentLane]->isAttacking || enemyOnLanes[currentLane]->isDead) {
            spawnMiscIndicator(310, 10, "miss!", MINO_COLORS[1]);
            return;
        }

        auto monster = enemyOnLanes[currentLane];
        auto monsterLocation = monster->getLocation();
        const int currentLaneRef = currentLane;

        // compliment the user (25+ = incredible!, 10+ awesome, the rest? = nice)
        spawnMiscIndicator(310, 10, finalDamage > 25 ? "fantastic!" : (finalDamage > 10 ? "awesome!" : "nice!"), MINO_COLORS[2]);

        // move to the monster and deal damage
        this->isAttacking = true; // this will stop the release from happening consecutively

        // move the body backwards (like charging)
        this->flandre->scheduleAnimation(RUN_BACKWARD, [&, finalDamage, monster, monsterLocation, currentLaneRef]() {
            // fly to the enemy
            this->flandre->moveSmooth(monsterLocation.x - 100, monsterLocation.y - 60, [&, finalDamage, monster, monsterLocation, currentLaneRef]() {
                // deal the damage
                this->spawnDamageIndicator(monsterLocation.x + 40, monsterLocation.y, finalDamage, true);
                auto rewards = monster->damageEntity(finalDamage);
                bool killed = rewards.type != -1;

                // if the enemy is dead
                if (killed) {
                    addStats(rewards.type == 1, rewards.amount);
                    this->enemyOnLanes[currentLaneRef] = nullptr; // mark the enemy as none
                }

                // damage animation
                this->flandre->scheduleAnimation(ATTACK_01, [&, finalDamage]() {
                    // go back to where she was
                    this->flandre->moveSmooth(X_LANE_PLAYER, Y_LANES[currentLane], [&]() {
                        this->flandre->setAnimation(RUN_FORWARD);
                        this->isAttacking = false;
                    }, 10);
                }, 15);
            }, 15);
        }, 10);
    }

    void onComboBreaks(const int combo) {
        if (accumulatedCharge > 0) releaseDamageOnCurrentLane();
    }

    void updateLevelAndGravity(const int newLevel) {
        currentTetrisLevel = min(15, newLevel);
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
        if (cleared >= 5) tetrisScore += 2860; // edge case??
        else if (cleared > 0) {
            // a T-Spin double gives the same score as a TETRIS (QUAD)
            // a T-Spin Single gives the same score as a TRIPLE
            // a t-spin TRIPLE gives 1.5x score of TETRIS
            double score = (TETRIS_SCORE[event.isSpin() ? (cleared == 2 ? 4 : 3) : cleared] * currentTetrisLevel) * (event.isSpin() && cleared == 3 ? 1.5 : 1);
            score += tetrisEngine->getComboCount() * 5; // each combo gives +5
            score += max(0, currentBackToBack) * 50; // each back to back gives +50 score

            tetrisScore += static_cast<long long>(score);
            if (const int newLevel = 1 + (clearedLines / LEVEL_THRESHOLD); newLevel != currentTetrisLevel) {
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
        baseDamage += static_cast<int>(tetrisEngine->getComboCount() * 0.75);

        // render text
        if (event.isMiniSpin() || event.isSpin()) {
            string message;
            char minoLetter = tolower(event.getLastMino()->name()[0]);
            message += minoLetter;
            message += "-spin";
            if (event.isMiniSpin()) {
                spawnBoardMiniSubtitle(150, 300, "mini", MINO_COLORS[event.getLastMino()->ordinal]);
            }
            spawnBoardSubtitle(100, 320, message, MINO_COLORS[event.getLastMino()->ordinal]);
        }
        if (cleared > 0) {
            spawnBoardTitle(50, 350, CLEAR_MESSAGES[cleared], cleared == 4 ? TETRIS_COLORS : nullptr);
        }

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
        const auto ppsString = str_printf("%.2f/s", firstPiecePlacedTime == -1 ? 0.0 : piecesPlaced / secondsElapsed);

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
        const auto apmString = str_printf(apm >= 100 ? "%.1f/m" : "%.2f/m", apm);

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

        const auto timeString      = str_printf("%02d:%02d", displayMin, displaySec);
        const auto msString        = str_printf(".%03d", displayMs);

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
        const auto scoreString = str_printf("%012d", min(99999999999L, tetrisScore)); // limit to 12 chars
        render_component_string(renderer, GRID_X_OFFSET + 170, GRID_Y_OFFSET + 605, scoreString, 3, 1, 27, 12);

        // render speed lvl
        /*
         * SPEED LVL
         * 0/15
         */
        const auto levelString = str_printf("%02d/15", currentTetrisLevel); // limit to 12 chars
        render_component_string(renderer, GRID_X_OFFSET + 500, GRID_Y_OFFSET + 380 + 2 * Y_OFFSET_STATISTICS, "speed lvl", 1.55, 1, 15, 14);
        render_component_string(renderer, GRID_X_OFFSET + 500, GRID_Y_OFFSET + 405 + 2 * Y_OFFSET_STATISTICS, levelString, 2, 1, 17, 12);

        // render accumulated charges (5 stages: 0 - empty, 1 - 1/4, 2 - 1/2, 3 - 3/4, 4 - full)
        const int maxChargeSlots = 10;
        int accumulated = accumulatedCharge;
        const int thunderBarX = 10;
        const int thunderBarY = 10;
        for (int i = 0; i < maxChargeSlots; i++) {
            int offset = 4;
            if (accumulated >= 4) {
                offset = 0;
                accumulated -= 4;
            } else if (accumulated > 0) {
                offset = 4 - accumulated;
                accumulated = 0;
            }
            renderThunderbolt(renderer, thunderBarX + (30 * i), thunderBarY, offset);
        }

        // render current armor (3 stages: 0 - empty, 1 - 1/2, 2 - full)
        const int maxArmorSlots = 10;
        int accumulatedArmor = currentArmorPoints;
        const int armorBarX = 10;
        const int armorBarY = thunderBarY + 45;
        for (int i = 0; i < maxArmorSlots; i++) {
            int offset = 2;
            if (accumulatedArmor >= 2) {
                offset = 0;
                accumulatedArmor -= 2;
            } else if (accumulatedArmor > 0) {
                offset = 2 - accumulatedArmor;
                accumulatedArmor = 0;
            }
            renderArmor(renderer, armorBarX + (25 * i), armorBarY, offset);
        }

    }

    void renderGarbageQueue(const int ox, const int oy) {
        const int GBQ_X_OFFSET = ox - (MINO_SIZE) + 180;
        const int GBQ_Y_OFFSET = oy + (Y_OFFSET) + 540;

        // to push the garbage up
        int accumulatedY = 0;

        // garbage is red
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
        for (auto &garbo : garbageQueue) {
            int toRisePixels = 30 * garbo; // each 30 pixels represent a line in the matrix (playfield)

            // each new "heap" of garbage is rendered above the old one 2 pixels
            SDL_Rect garbage = { GBQ_X_OFFSET, GBQ_Y_OFFSET - toRisePixels - accumulatedY, 12, toRisePixels };
            accumulatedY += toRisePixels + 2;

            SDL_RenderFillRect(renderer, &garbage);
        }

        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    }

    void renderTetrisInterface(const int ox, const int oy) {
        int shakeFactor = boardRumble > 0 ? std::sin(SpritesRenderingPipeline::renderPasses()) * 10 : 0;
        // render the board body first
        render_tetris_board(ox + shakeFactor, oy, renderer, this->tetrisEngine);
        // and then the statistics
        renderTetrisStatistics(ox, oy);
        // render garbage queue
        renderGarbageQueue(ox + shakeFactor, oy);
        if (boardRumble > 0) {
            --boardRumble;
        }
    }

    SDL_Event _event{};
    void onTetrisTick() {
        while (SDL_PollEvent(&_event)) {
            if (_event.type == SDL_QUIT) {
                exit(1);
            }
            process_input(_event, this->tetrisEngine);
        }

        SDL_RenderClear(renderer);
        SpritesRenderingPipeline::renderEverything(renderer);
        renderTetrisInterface(100, 90);

        sprintfcdbg(this->tetrisEngine, static_cast<int>(SpritesRenderingPipeline::getSprites().size()));
        SDL_RenderPresent(renderer); // Show updated frame
    }
};
#endif
