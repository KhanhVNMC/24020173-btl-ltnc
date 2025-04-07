//
// Created by GiaKhanhVN on 4/7/2025.
//
#include "tetris_player.h"

TetrisPlayer::TetrisPlayer(ExecutionContext* context, SDL_Renderer* sdlRenderer, TetrisEngine* engine) {
    // register constants
    this->renderer = sdlRenderer;
    this->tetrisEngine = engine;
    this->context = context;

    // hook into events
    this->tetrisEngine->runOnTickEnd([&] { onTetrisTick(); });
    this->tetrisEngine->runOnGameOver([&]() { onGameOver(); });
    this->tetrisEngine->runOnMinoLocked([&](int cleared) {
        // if first piece, mark this as the first time
        if (firstPiecePlacedTime == -1) {
            firstPiecePlacedTime = System::currentTimeMillis();
        }
        this->piecesPlaced++;
        this->accumulatedCharge = 40;
        this->onMinoLocked(cleared);
    });
    this->tetrisEngine->onComboBreaks([&](const int combo) { });
    this->tetrisEngine->onPlayfieldEvent([&](const PlayfieldEvent& event) { playFieldEvent(event); });

    // init gravity to lvl 1
    updateLevelAndGravity(1);
    // first lane is 3 (bottom)
    this->currentLane = 3;

    // init background (it will stay still until the game starts)
    initParallaxBackground();

    // register the representative player entity
    this->flandre = new FlandreScarlet();
    this->flandre->teleportStrict(X_LANE_PLAYER, Y_LANES[currentLane]);
    this->flandre->setAnimation(IDLE);
    this->flandre->spawn();
}

TetrisPlayer::~TetrisPlayer() {
    context->unhook(tetrisEngineExecId);
    delete tetrisEngine; // this will blow up, gg
}

void TetrisPlayer::initParallaxBackground() {
    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_NONE, 0, 0, 2)); // first layer
    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_NONE, 2048, 0, 2)); // first layer follow up

    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_HORIZONTAL, 0, 0, 3));
    parallaxBackgrounds.push_back(new BackgroundScroll(SDL_FLIP_HORIZONTAL, 2048, 0, 3));

    for (BackgroundScroll* parallax : parallaxBackgrounds) {
        if (parallax != nullptr) parallax->spawn();
    }
}

void TetrisPlayer::startEngineAndGame() {
    // boot the engine up (schedule 5s countdown)
    this->tetrisEngine->gameInterrupt(true); // do not let the game start, wait until cooldown ends

    // countdown (3s)
    for (int i = 3; i >= 0; --i) {
        tetrisEngine->scheduleDelayedTask((3 - i) * 60, [&, i]() {
            spawnMiddleScreenText(i == 0 ? 330 : 377, 390, i == 0 ? "go!" : to_string(i), MINO_COLORS[3]);
            // game actually start here
            if (i == 0) {
                // resume context
                tetrisEngine->gameInterrupt(false);
                this->gameStartTime = System::currentTimeMillis();
                this->gameStarted = true;
                // resume parallax background
                for (BackgroundScroll* parallax : parallaxBackgrounds) {
                    if (parallax != nullptr) parallax->scroll = true;
                }
                // make player "fly"
                this->flandre->setAnimation(RUN_FORWARD);
                // startWave  test
                startWave(1);
            }
        });
    }

    // start the engine without using main thread, as we will take over the execution context
    // right below
    this->tetrisEngine->start(false);
    // take over the execution context
    this->tetrisEngineExecId = context->hook([&]() { this->tetrisEngine->gameLoopBody(); });
}

void TetrisPlayer::onGameOver() {
    // disallow player input the moment the game is over
    this->isGameOver = true;
    // interrupt the game loop
    this->tetrisEngine->gameInterrupt(true);
    // make the board shake for one last time
    this->boardRumble = 20;

    // play player's death animation
    this->flandre->deathAnimation();

    // all enemies fly away
    for (NormalEntity* enemy : enemyOnLanes) {
        if (enemy == nullptr) continue;
        enemy->moveSmooth(1800, enemy->strictY);
    }

    // fall down to the bottom of the screen
    tetrisEngine->scheduleDelayedTask(30, [&]() {
        // stop parallax scrolling
        for (BackgroundScroll* parallax : parallaxBackgrounds) {
            if (parallax != nullptr) parallax->scroll = false;
        }
        // flip upside down and fall
        this->flandre->rotate(180);
        this->flandre->flipSprite(SDL_FLIP_HORIZONTAL);
        this->flandre->moveSmooth(this->flandre->strictX, 900, nullptr, 8, true);
    });

    // the entire board falls down
    tetrisEngine->scheduleDelayedTask(60, [&]() {
        this->boardFallAnimationCount = true;
        // all enemies remove
        int i = 0;
        for (NormalEntity* enemy : enemyOnLanes) {
            if (enemy == nullptr) continue;
            enemy->remove();
            enemyOnLanes[i] = nullptr;
            i++;
        }
    });
}

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

void TetrisPlayer::killEnemyOnLane(int lane) {
    if (enemyOnLanes[lane] == nullptr) return;
    enemyOnLanes[lane]->remove();
    enemyOnLanes[lane] = nullptr;
};

void TetrisPlayer::moveToLane(const int targetLane) {
    if (this->isMovingToAnotherLane || this->isAttacking || !this->gameStarted || this->isGameOver) return; // prevent overlapping
    this->isMovingToAnotherLane = true;
    this->currentLane = targetLane % 4; // prevent overshooting

    // move to specified location
    this->flandre->moveSmooth(X_LANE_PLAYER, Y_LANES[currentLane], [&]() {
        this->flandre->setAnimation(RUN_FORWARD);
        this->isMovingToAnotherLane = false;
        this->flandre->rotate(0);
    }, 10); // super-fast lane-switching
}

void TetrisPlayer::addStats(bool isCharge, int amount) {
    if (isCharge) accumulatedCharge += amount;
    else currentArmorPoints += amount;
    // display the damage accumulated
    spawnMiscIndicator(isCharge ? 310 : 270, isCharge ? 10 : 25, "+" + std::to_string(amount), isCharge ? MINO_COLORS[5] : 0xc9c9c9);
}

void TetrisPlayer::onDamageSend(const int damage) {
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

void TetrisPlayer::inflictDamage(int damage, int oldLane) {
    if (!this->gameStarted || this->isGameOver) return;
    if (this->isAttacking || this->isMovingToAnotherLane || currentLane != oldLane) {
        spawnMiscIndicator(flandre->strictX, Y_LANES[oldLane], "miss!", MINO_COLORS[3]);
        return;
    }

    if (currentArmorPoints > 0 && !sFragile) {
        if (currentArmorPoints == 1) {
            damage *= 0.75;
            --currentArmorPoints;
            spawnMiscIndicator(270, 55, "-1", 0xc9c9c9);
        } else {
            damage *= 0.5;
            currentArmorPoints -= 2;
            spawnMiscIndicator(270, 55, "-2", 0xc9c9c9);
        }
    } else if (sFragile) {
        // ineffective
        spawnMiscIndicator(270, 55, "-0", 0xc9c9c9);
    }

    damage = max(1, damage);
    garbageQueue.push_back(damage);
    this->spawnDamageIndicator(getLocation().x + 40, getLocation().y + 20, damage, false);

    this->flandre->damagedAnimation(true);
    boardRumble = 10; // rumble for 10 frames
}

void TetrisPlayer::inflictDebuff(int debuff, int timeInSeconds, int oldLane) {
    if (!this->gameStarted || this->isGameOver) return;
    if (this->isAttacking || this->isMovingToAnotherLane || currentLane != oldLane) {
        spawnMiscIndicator(flandre->strictX, Y_LANES[oldLane], "miss!", MINO_COLORS[3]);
        return;
    }

    setDebuff(static_cast<Debuff>(debuff), true); // inflict the debuff
    sDebuffTime[debuff] = timeInSeconds * 60; // time in frames

    spawnMiscIndicator(flandre->strictX, Y_LANES[oldLane], "debuff!", MINO_COLORS[1]);
    this->flandre->damagedAnimation(false);
    boardRumble = 10; // rumble for 10 frames
}

void TetrisPlayer::releaseDamageOnCurrentLane() {
    if (isAttacking || isMovingToAnotherLane || !this->gameStarted) return; // currently moving, do NOT attack
    if (enemyOnLanes[currentLane] != nullptr && enemyOnLanes[currentLane]->isAttacking) {
        return; // if the enemy is attacking, we cannot release damage
    }

    const int finalDamage = accumulatedCharge * (sWeakness ? 0.75 : 1); // 25% less effective if weakness
    accumulatedCharge = 0; // reset charges

    // if there's no enemy on the current lane OR the enemy there is dead, user missed
    if (enemyOnLanes[currentLane] == nullptr || enemyOnLanes[currentLane]->isDead || enemyOnLanes[currentLane]->isSpawning) {
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
                // free the memory of the thing
                this->enemyOnLanes[currentLaneRef] = nullptr; // mark the enemy as none
                // increment the counter
                waveKilledEnemies++;
                if (waveKilledEnemies >= 4) {
                    onWaveCompletion();
                }
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

void TetrisPlayer::onMinoLocked(const int linesCleared) {
    // release damage if no lines cleared but charge is present
    if (linesCleared <= 0 && accumulatedCharge > 0) {
        releaseDamageOnCurrentLane();
    }

    // manage the garbage thingy (rise garbage)
    // if empty, no garbage, we no care
    if (linesCleared <= 0 && !garbageQueue.empty()) {
        // queue the garbage up
        int currentHoleIndex = rand() % 10; // the garbage hole
        int amount = garbageQueue.front(); // amount of garbo to raise
        garbageQueue.pop_front();

        if (amount <= 0) return;
        // lock the game while we raise the garbage
        tetrisEngine->gameInterrupt(true);
        // raise
        for (int i = 0; i < amount; i++) {
            // raise line(s) every 5 ticks (5 / 60 of a second)
            tetrisEngine->scheduleDelayedTask(i * 5, [&, i, amount, currentHoleIndex]() {
                tetrisEngine->raiseGarbage(1, currentHoleIndex);
                if (i >= amount - 1) {
                    // resume
                    tetrisEngine->gameInterrupt(false);
                }
            });
        }
    }
}

void TetrisPlayer::playFieldEvent(const PlayfieldEvent& event) {
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

    // counter-attack
    int counteredDamage = 0;
    while (!garbageQueue.empty() && baseDamage > 0) {
        int amount = garbageQueue.front(); // amount of garbo to raise
        garbageQueue.pop_front();

        if (baseDamage >= amount) {
            baseDamage -= amount;
            counteredDamage += amount;
        } else {
            garbageQueue.push_front(amount - baseDamage);
            counteredDamage += baseDamage;
            baseDamage = 0;
        }
    }

    // if the player countered damage, show it (left side)
    if (counteredDamage > 0) spawnPriorityIndicator(230, 640, to_string(counteredDamage), MINO_COLORS[5]);

    // fire event
    if (baseDamage > 0) onDamageSend(baseDamage);
}

void TetrisPlayer::updateLevelAndGravity(const int newLevel) {
    currentTetrisLevel = min(15, newLevel);
    // increase engine gravity
    this->tetrisEngine->getCurrentConfig()->setGravity(LEVELS_GRAVITY[min(15, currentTetrisLevel)]);
    this->tetrisEngine->updateMutableConfig(sSuperSonic);
}

#define Y_OFFSET_STATISTICS 65
void TetrisPlayer::renderTetrisStatistics(const int ox, const int oy) {
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
    const int64_t timePassedMs = gameStartTime != -1 ? (System::currentTimeMillis() - this->gameStartTime) : 0;
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

    // render status effects
    int yOffset = 0;
    const int baseX = 780;
    const int baseY = 170;
    const int spacing = 85;

    if (sBlinded) {
        renderDebuffIcon(renderer, baseX, baseY + (spacing * yOffset), 0);
        render_component_string(renderer, baseX - 20, baseY + (spacing * yOffset) - 20, str_printf("%05.2f", sDebuffTime[BLIND] / 60.0), 1.1, 1, 15);
        ++yOffset;
    }
    if (sNoHold) {
        renderDebuffIcon(renderer, baseX, baseY + (spacing * yOffset), 1);
        render_component_string(renderer, baseX - 20, baseY + (spacing * yOffset) - 20, str_printf("%05.2f", sDebuffTime[NO_HOLD] / 60.0), 1.1, 1, 15);
        ++yOffset;
    }
    if (sSuperSonic) {
        renderDebuffIcon(renderer, baseX, baseY + (spacing * yOffset), 2);
        render_component_string(renderer, baseX - 20, baseY + (spacing * yOffset) - 20, str_printf("%05.2f", sDebuffTime[SUPER_SONIC] / 60.0), 1.1, 1, 15);
        ++yOffset;
    }
    if (sWeakness) {
        renderDebuffIcon(renderer, baseX, baseY + (spacing * yOffset), 3);
        render_component_string(renderer, baseX - 20, baseY + (spacing * yOffset) - 20, str_printf("%05.2f", sDebuffTime[WEAKNESS] / 60.0), 1.1, 1, 15);
        ++yOffset;
    }
    if (sFragile) {
        renderDebuffIcon(renderer, baseX, baseY + (spacing * yOffset), 4);
        render_component_string(renderer, baseX - 20, baseY + (spacing * yOffset) - 20, str_printf("%05.2f", sDebuffTime[FRAGILE] / 60.0), 1.1, 1, 15);
        ++yOffset;
    }
}

void TetrisPlayer::renderGarbageQueue(const int ox, const int oy) {
    const int GBQ_X_OFFSET = ox - (MINO_SIZE) + 180;
    const int GBQ_Y_OFFSET = oy + (Y_OFFSET) + 540;

    // to push the garbage up
    int accumulatedY = 0;

    // garbage is red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
    for (auto &garbo : garbageQueue) {
        if (garbo <= 0) continue;
        int toRisePixels = 30 * garbo; // each 30 pixels represent a line in the matrix (playfield)

        // each new "heap" of garbage is rendered above the old one 2 pixels
        SDL_Rect garbage = { GBQ_X_OFFSET, GBQ_Y_OFFSET - toRisePixels - accumulatedY, 12, toRisePixels };
        accumulatedY += toRisePixels + 2;

        SDL_RenderFillRect(renderer, &garbage);
    }

    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
}

void TetrisPlayer::renderTetrisInterface(const int ox, const int oy) {
    long renderPasses = SpritesRenderingPipeline::renderPasses();
    int shakeFactor = boardRumble > 0 ? std::sin(renderPasses) * 10 : 0;

    // if blinded, board is invisible
    // every 3 seconds, flash it for 1s so player knows what's still there
    bool isInvisible = sBlinded;
    if (isInvisible && (renderPasses / 60) % 3 == 0) {
        isInvisible = false;
    }

    // render the board body first
    render_tetris_board(ox + shakeFactor, oy + boardDrop, renderer, this->tetrisEngine, isInvisible);
    // and then the statistics
    renderTetrisStatistics(ox, oy + boardDrop);
    // render garbage queue
    renderGarbageQueue(ox + shakeFactor, oy + boardDrop);

    // count down the rumble
    if (boardRumble > 0) {
        --boardRumble;
    }
}

void TetrisPlayer::onWaveCompletion() {
    spawnPhysicsBoundText("wave " + to_string(lastWave) + " clear!", 1600, 400, -10, 0, 300, 0, 4, 50, 15, nullptr, MINO_COLORS[2]);
    // rewards
    this->tetrisEngine->scheduleDelayedTask(60, [&]() {
        int amount = 2 + lastWaveDifficulty + (rand() % 10);
        bool isArmor = (rand() % 2) == 1;

        addStats(!isArmor, amount);
        spawnPhysicsBoundText("+" + to_string(amount) + " " + (isArmor ? "armor" : "attack") + "!", 1600, 500, -10, 0, 300, 0, 4, 50, 15, nullptr, !isArmor ? MINO_COLORS[5] : 0xc9c9c9);
    });

    // next wave in 4s
    this->tetrisEngine->scheduleDelayedTask(180, [&]() {
        this->startWave(lastWave + 1);
    });
}