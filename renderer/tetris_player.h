//
// Created by GiaKhanhVN on 3/3/2025.
//
#include <random>
#include "tetris_renderer.h"
#include "sprites/gameworld.cpp"
#include "sprites/entities/Redgga.h"
#include "sprites/player/playerentity.h"
#include "sprites/entity_prop.h"
#include "sprites/entities/Grigga.h"
#include "sprites/entities/Blugga.h"
#include "sprites/entities/Nigga.h"
#include "sprites/entities/fairies/debuff_fairy.h"
#include "sprites/entities/fairies/BlinderFairy.h"
#include "sprites/entities/fairies/WeakenerFairy.h"
#include "sprites/entities/fairies/DistractorFairy.h"
#include "sprites/entities/fairies/DisturberFairy.h"
#include "../game/hooker.h"
#include "../game/gamescene.h"

#ifndef TETRIS_PLAYER_H
#define TETRIS_PLAYER_H

static int TETRIS_SCORE[5] = { 0, 50, 110, 630, 2300 }; // score for each type of line clears
static int LEVEL_THRESHOLD = 35; // advance every X lines
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
        0x00FFFF, // I mino
        0xFFEA00, // O mono
};

#define X_LANE_PLAYER 840
#define X_LANE_ENEMIES 1400

enum WaveDifficulty {
    WAVE_EASY,
    WAVE_MEDIUM,
    WAVE_HARD
};

enum FairyType {
    WEAKENER,
    DISTRACTOR,
    BLINDER,
    DISTURBER
};

enum GameMode {
    CAMPAIGN,
    ENDLESS,
};

/**
 * Choose a random element with a specific RNG bias
 */
template<typename T> T chooseRandomWithBias(const vector<T>& options) {
    static random_device rd;
    static mt19937 rng(rd());
    uniform_int_distribution<size_t> dist(0, options.size() - 1);
    return options[dist(rng)];
}

class TetrisPlayer : public GameScene {
public:
    // text
    static void spawnPhysicsBoundText(string str, int x, int y, double randVelX, double randVelY, int lifetime, double gravity, double scalar, int strgap, int width, const int* colors = nullptr, const int applyThisColorToAll = -1, bool priority = false);
    static void spawnDamageIndicator(const int x, const int y, const int damage, bool offensive);
    static void spawnBoardTitle(const int x, const int y, string title, const int* colors = nullptr);
    static void spawnBoardSubtitle(const int x, const int y, string title, const int color);
    static void spawnBoardMiniSubtitle(const int x, const int y, string title, const int color);
    static void spawnMiscIndicator(const int x, const int y, string indicator, const int color);
    static void spawnPriorityIndicator(const int x, const int y, string indicator, const int color);
    static void spawnMiddleScreenText(const int x, const int y, string title, const int color, const int life = 60);

    // icons
    static void renderThunderbolt(SDL_Renderer* renderer, const int x, const int y, const int offset);
    static void renderArmor(SDL_Renderer* renderer, const int x, const int y, const int offset);
    static void renderDebuffIcon(SDL_Renderer* renderer, const int x, const int y, const int offset);

    /*** internal objects ***/
    // player & background entities
    FlandreScarlet* flandre;
    vector<BackgroundScroll*> parallaxBackgrounds;

    // SDL
    SDL_Renderer* renderer;

    // engine handler
    TetrisEngine* tetrisEngine;
    deque<int> garbageQueue;

    // context
    int tetrisEngineExecId = 0;
    bool isGameOver = false;
    bool boardFallAnimationCount = false;

    // graphics cue
    int boardDrop = 0;
    int boardRumble = 0; // frames that the board will rumble
    /*** end of internal objects ***/

    /*** statistics ***/
    long long tetrisScore = 0;
    int currentTetrisLevel = 0;

    // start
    long long gameStartTime = -1;
    bool gameStarted = false;

    // timing and keeping track
    long long firstPiecePlacedTime = -1;
    int piecesPlaced = 0;

    long long firstDamageInflictedTime = -1;
    int totalDamage = 0; // keep track of output dmg (not counting counter)

    int clearedLines = 0; // total cleared lines
    int currentBackToBack = 0; // keep track of back to back(s)
    /*** end of statistics ***/

    /*** player attacks ***/
    int currentLane = 0;
    int accumulatedCharge = 0;
    int currentArmorPoints = 0;

    // flags to stop shit from exploding
    bool isMovingToAnotherLane = false;
    bool isAttacking = false;

    // enemies
    NormalEntity* enemyOnLanes[4] = {nullptr, nullptr, nullptr, nullptr}; // 4 lanes, 4 available monsters (initialized as 0)
    /*** end of player attacks ***/

    /*** status effects ***/
    bool sBlinded = false; // board invisible
    bool sNoHold = false; // cannot hold
    bool sSuperSonic = false; // mach 5 speed
    bool sWeakness = false; // reduce output dmg
    bool sFragile = false; // ineffective armor
    // time left for each debuff
    int sDebuffTime[5] = { 0, 0, 0, 0, 0 };
    /*** end of status effects ***/

    /*** input handling (ARR, DAS) ***/
    // DAS (ARR) for the entire game
    bool leftHeld = false;
    bool rightHeld = false;
    // timestamps
    int64_t leftPressTime = 0;
    int64_t rightPressTime = 0;
    int64_t nextLeftShiftTime = 0;
    int64_t nextRightShiftTime = 0;

    // const (this is my personal value in TETR.IO) [ms]
    const int DAS = 200; // delay before auto-repeat begins (ms)
    const int ARR = 50; // Auto Repeat Rate (interval between moves after the initial DAS)
    /*** end of input handling (ARR, DAS) ***/

    bool showDebug = false;
    // gameplay
    GameMode gamemode = CAMPAIGN;

    /**
     * Set a debuff
     * @param type debuff enum
     * @param value true if grant, false if remove
     */
    void setDebuff(Debuff type, bool value);
    /*** end of status effects ***/

    // the execution context (nullptr if none)
    ExecutionContext* context = nullptr;

    /**
     * Initialize a game of Tetris: Diarrhea Edition
     * @param context the exec context (can be nullptr)
     * @param sdlRenderer the SDL renderer
     * @param engine the main engine
     */
    TetrisPlayer(ExecutionContext* context, SDL_Renderer* sdlRenderer, TetrisEngine* engine, GameMode gamemode = CAMPAIGN);

    /**
     * This destructor deletes TetrisEngine, which is very dangerous if left
     * unchecked
     */
    protected: ~TetrisPlayer() override;

    /**
     * Initialize the parallax scrolling task (SIF)
     */
    public: void initParallaxBackground();

    /**
     * Start the Tetris game, along with the Engine (with countdown),
     * This action will take over the scene
     */
    void startScene() override;

    /**
     * Release the scene for use again (this will delete TetrisPlayer too)
     */
    void stopScene() override;

    /**
     * @return current screen location of player entity
     */
    SpriteLoc getLocation() {
        return this->flandre->getLocation();
    }

    /** debug methods **/
    [[maybe_unused]] void sprintfcdbg(TetrisEngine* tetris, int spriteCount);
    /** end of debug methods **/

    /**
     * Spawn a motherfucker on a lane
     * @param lane the lane index (0-3)
     * @param entity an entity
     */
    void spawnEnemyOnLane(int lane, NormalEntity* entity);

    /**
     * Kill a motherfucker on a lane
     * @param lane the lane index (0-3)
     */
    void killEnemyOnLane(int lane);

    /**
     * Move to a lane (0-3)
     * @param targetLane
     */
    void moveToLane(const int targetLane);

    /**
     * Add a specific amount of stats
     * @param isCharge true if blue thingy, false if armor
     * @param amount bruh
     */
    void addStats(bool isCharge, int amount);

    /**
     * Deal damage to the player
     * @param damage
     * @param oldLane the lane that the enemy started the attack on
     */
    void inflictDamage(int damage, int oldLane);

    /**
     * Grant debuff effect to the player
     * @param damage
     * @param oldLane the lane that the enemy started the attack on
     */
    void inflictDebuff(int debuff, int timeInSeconds, int oldLane);

    /**
     * Release all damage on the current lane
     */
    void releaseDamageOnCurrentLane();

    /**
     * Update engine speed
     * @param newLevel new lvl
     */
    void updateLevelAndGravity(const int newLevel);

    /**
     * (Event) fire when damage is send (not counting counter-attack)
     * @param damage amount of damage
     */
    void onDamageSend(const int damage);

    /**
     * (Event) fire when a playfield event is fired from the Tetris Engine
     * @param event (reference) the event class
     */
    void playFieldEvent(const PlayfieldEvent& event);

    /**
     * (Event) fire when a mino is locked to the playfield (from the Tetris Engine)
     * @param linesCleared amount of lines cleared
     */
    void onMinoLocked(const int linesCleared);

    /**
     * (Event) fire when the player fucked up (top out)
     */
    void onGameOver();

    /**
     * (Event) fire when the player finished a wave of monsters
     */
    void onWaveCompletion();

    /**
     * (Event) Handle SDL events for this Game Scene
     * @param event SDL Event
     */
    void processSceneInput(SDL_Event& event);

    /**
     * Render the Tetris Board's external features
     */
    void renderTetrisStatistics(const int ox, const int oy);
    void renderGarbageQueue(const int ox, const int oy);
    void renderTetrisInterface(const int ox, const int oy);

    function<void(ExecutionContext*, SDL_Renderer*)> gameOverSceneCallback = nullptr;
    /**
     * End game or game over
     * @param lost if the player lost/campaign ended
     */
    void showGameOverScreen(const bool lost = true);

    /**
     * Create a fairy entity using typeset
     * @param type
     * @return the heap-allocated fairy
     */
    DebuffFairy* createFairy(FairyType type) {
        switch (type) {
            case WEAKENER:  return new WeakenerFairy(this);
            case DISTRACTOR: return new DistractorFairy(this);
            case BLINDER:   return new BlinderFairy(this);
            case DISTURBER: return new DisturberFairy(this);
            default:        return nullptr;
        }
    }

    int totalKilledEnemies = 0;
    int waveKilledEnemies = 0;
    int lastWave = 0;
    WaveDifficulty lastWaveDifficulty = WAVE_EASY;

    /**
     * Start a wave of monsters
     * @param wave a valid number from 1 - infinity
     */
    void startWave(int wave) {
        WaveDifficulty wDifficulty = WAVE_EASY;
        string waveText = "easy";
        int waveColor = MINO_COLORS[2]; // green

        if (wave >= 3 && wave <= 5) {
            wDifficulty = WAVE_MEDIUM;
            waveText = "medium";
            waveColor = MINO_COLORS[6]; // yellow
        } else if (wave > 5) {
            wDifficulty = WAVE_HARD;
            waveText = "hard";
            waveColor = MINO_COLORS[1]; // red
        }

        // sync
        this->lastWave = wave;
        this->lastWaveDifficulty = wDifficulty;

        // reset counter back to 0 as a new wave begins
        this->waveKilledEnemies = 0;
        spawnPhysicsBoundText("wave " + to_string(wave) + ": " + waveText, 1600, 400, -10, 0, 300, 0, 4, 50, 15, nullptr, waveColor);
        populateLane(wDifficulty);
    }

    /**
     * Spawn enemies based on wave difficulty
     * @param difficulty
     */
    void populateLane(WaveDifficulty difficulty) {
        vector<NormalEntity*> toSpawn;
        switch (difficulty) {
            case WAVE_EASY:
                // 4 random easy DPS mob
                for (int i = 0; i < 4; ++i) {
                    int roll = rand() % 100;
                    if (roll < 60) {
                        toSpawn.push_back(new Grigga(this)); // easy
                    } else {
                        toSpawn.push_back(new Blugga(this)); // medium
                    }
                }
                break;
            case WAVE_MEDIUM: {
                // either 3dps + 1 fairy or 4dps (like easy)
                int roll = rand() % 100;
                if (roll < 60) {
                    // 3 DPS + 1 fairy (prefer medium fairies)
                    for (int i = 0; i < 3; ++i) {
                        int dpsRoll = rand() % 100;
                        if (dpsRoll < 50) {
                            toSpawn.push_back(new Blugga(this));
                        } else {
                            toSpawn.push_back(new Grigga(this));
                        }
                    }

                    FairyType fairyChoice = chooseRandomWithBias(vector<FairyType>{
                        WEAKENER, DISTRACTOR, DISTURBER
                    });
                    toSpawn.push_back(createFairy(fairyChoice));
                } else {
                    // 4 DPS
                    for (int i = 0; i < 4; ++i) {
                        int dpsRoll = rand() % 100;
                        if (dpsRoll < 10) {
                            toSpawn.push_back(new Redgga(this)); // Rare hard mob
                        }
                        else if (dpsRoll < 60) {
                            toSpawn.push_back(new Blugga(this));
                        } else {
                            toSpawn.push_back(new Grigga(this));
                        }
                    }
                }
                break;
            }
            case WAVE_HARD: {
                // 3dps + 1fairy or 2 dps + 2 fairy (prioritize hardest ones)
                int roll = rand() % 100;
                if (roll < 50) { // 50%
                    // 3 DPS + 1 fairy (only medium or hard fairies)
                    for (int i = 0; i < 3; ++i) {
                        int dpsRoll = rand() % 100;
                        if (dpsRoll < 30) { // 30%
                            toSpawn.push_back(new Nigga(this));
                        } else if (dpsRoll < 80) { // 80%
                            toSpawn.push_back(new Redgga(this));
                        } else { // 20%
                            toSpawn.push_back(new Blugga(this));
                        }
                    }

                    FairyType fairyChoice = chooseRandomWithBias(vector<FairyType>{
                        WEAKENER, DISTRACTOR, BLINDER, DISTURBER
                    });
                    toSpawn.push_back(createFairy(fairyChoice));
                } else {
                    // 2 DPS + 2 fairies
                    for (int i = 0; i < 2; ++i) {
                        int dpsRoll = rand() % 100;
                        if (dpsRoll < 30) {
                            toSpawn.push_back(new Nigga(this));
                        } else {
                            toSpawn.push_back(new Redgga(this));
                        }
                    }

                    vector<FairyType> hardFairyOptions = { WEAKENER, DISTRACTOR, BLINDER };
                    toSpawn.push_back(createFairy(chooseRandomWithBias(hardFairyOptions)));
                    toSpawn.push_back(createFairy(chooseRandomWithBias(hardFairyOptions)));
                }
                break;
            }

            default: break;
        }

        // finalize lane population
        int lane = 0;
        for (NormalEntity* entity : toSpawn) {
            this->tetrisEngine->scheduleDelayedTask(rand() % 80, [&, entity, lane] {
                this->spawnEnemyOnLane(lane, entity);
            });
            ++lane;
        }
    }

    int smallClock = 0; // this clock will tick as the board begin to fall
    int fadeTicks = 60; // 60 ticks for 1-second fade-in
    int fadeOutTicks = -1; // 60 ticks for 1-second fade-in
    /**
     * (Event) Runs every single tick (60.0TPS)
     */
    void onTetrisTick() {
        SDL_Event event;
        while (context->popEvent(event)) {
            this->processSceneInput(event);
        }

        // handle debuffs
        for (int i = 0; i < 5; ++i) {
            if (sDebuffTime[i] == INT_MIN) continue; // infinite debuff
            if (sDebuffTime[i] <= 0) {
                setDebuff(static_cast<Debuff>(i), false);
                continue;
            }
            --sDebuffTime[i];
        }

        // ARR + DAS handling
        int64_t now = System::currentTimeMillis();

        // if it is time to shift, move accordingly
        if (leftHeld && now >= nextLeftShiftTime) {
            tetrisEngine->moveLeft();
            // this could count as an "artificial" keypress, so handle it like we did before
            nextLeftShiftTime = now + ARR;
        }

        if (rightHeld && now >= nextRightShiftTime) {
            tetrisEngine->moveRight();
            // this could count as an "artificial" keypress, so handle it like we did before
            nextRightShiftTime = now + ARR;
        }

        // handle death animation
        if (boardFallAnimationCount) {
            smallClock++;
            boardDrop += 2 + (5 * log(smallClock)); // smooth fall
        }

        // begin render the scene
        SDL_RenderClear(renderer);

        // render low priority sprites first
        SpritesRenderingPipeline::renderNormal(renderer);

        // then the tetris board
        renderTetrisInterface(100, 90);

        // then the high priority ones
        SpritesRenderingPipeline::renderPriority(renderer);

        // fade-in overlay
        if (fadeTicks > 0) {
            int alpha = static_cast<int>(255.0f * (fadeTicks / 60.0f)); // fade from 255 to 0
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            // black screen overlay
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
            SDL_Rect fullScreen = {0, 0, 1720, 860};
            // fill the screen with it
            SDL_RenderFillRect(renderer, &fullScreen);
            fadeTicks--;
        }

        // fade OUT overlay
        if (fadeOutTicks >= 0) {
            int alpha = static_cast<int>(255.0f - (255.0f * (fadeOutTicks / 60.0f))); // fade from 0 to 255 over 60 frames
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            // black screen overlay
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
            SDL_Rect fullScreen = {0, 0, 1720, 860};
            // fill the screen with it
            SDL_RenderFillRect(renderer, &fullScreen);
            if (fadeOutTicks > 0) fadeOutTicks--;
        }

        // what the fuck
        if (showDebug) sprintfcdbg(this->tetrisEngine, static_cast<int>(SpritesRenderingPipeline::getSprites().size() + SpritesRenderingPipeline::getPrioritySprites().size()));
        SDL_RenderPresent(renderer); // Show updated frame
    }
};
#endif
