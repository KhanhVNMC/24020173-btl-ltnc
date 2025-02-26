/**
 * Tetris Engine: C++ Edition<br>
 * This is a one-to-one, faithful port of the original "Tetris Engine: Java Edition"
 * to C++ for university project(s) [also made by me]<br>
 *<br>
 * Below is the description straight from the original version:
 *<br>
 * Modern, Guideline-compliant Tetris Engine
 * @see https://tetris.wiki/Tetris_Guideline
 *<br>
 * @author GiaKhanhVN (24020173)
 * @porter GiaKhanhVN (24020173)
 * @portedFrom Java (Java 16 LTS)
 */
#ifndef TETRIS_ENGINE_CPP
#define TETRIS_ENGINE_CPP

// stdcpp includes
#include <iostream>
#include <functional>
#include <queue>
#include <cmath>
#include <thread>
#include <map>
#include <utility>

// java mimick
#include "javalibs/jsystemstd.h"

// internal libraries
#include "tetrominoes.h"
#include "playfield_event.h"
#include "tetromino_gen_blueprint.h"
#include "tetris_config.h"

/**
 * @caution The tick rate is tied to MANY important aspects of the Engine (gravity, timeout, intervals, ...)
 * <br><b>DO NOT CHANGE THE TICK-RATE WITHOUT ADJUSTING OTHER VALUES!</b>
 */
namespace EngineTimer {
    static constexpr float TARGETTED_TICK_RATE = 144.0F; // 60 TPS (aka 60 FPS in "Tetris: The Grand Master")
    static constexpr double TICK_INTERVAL_MS = 1000.0F / TARGETTED_TICK_RATE; // in milliseconds
}

/**
 * Constant representing the value used to indicate ghost pieces on the playfield.
 * Ghost pieces show the potential landing position of the currently falling tetromino
 * without affecting the actual state of the playfield. This constant is a random negative
 * number to differentiate it from regular pieces.
 */
static constexpr int GHOST_PIECE_CONVENTION = -121;
/**
 * Constant representing the value used to indicate garbage minos in the playfield.
 * Garbage minos are typically used in modes where players receive pieces that may
 * obstruct their playfield, such as in multiplayer Tetris games. This constant is set
 * to a value greater than the maximum number of existing MinoType values to avoid
 * collision with regular pieces.
 */
static constexpr int GARBAGE_MINO_CONVENTION = MinoType::valuesLength + 1;

class Tetromino;
/*************** BEGIN SRS KICK TABLE *****************/
/** @see https://harddrop.com/wiki/SRS **/
static vector<vector<vector<int> > > I_KICK_TABLE = {
        // 0 -> R
        {{0, 0}, {-2, 0}, {1,  0}, {-2, -1}, {1,  2}},
        // R -> 0
        {{0, 0}, {2,  0}, {-1, 0}, {2,  1},  {-1, -2}},
        // R -> 2
        {{0, 0}, {-1, 0}, {2,  0}, {-1, 2},  {2,  -1}},
        // 2 -> R
        {{0, 0}, {1,  0}, {-2, 0}, {1,  -2}, {-2, 1}},
        // 2 -> L
        {{0, 0}, {2,  0}, {-1, 0}, {2,  1},  {-1, -2}},
        // L -> 2
        {{0, 0}, {-2, 0}, {1,  0}, {-2, -1}, {1,  2}},
        // L -> 0
        {{0, 0}, {1,  0}, {-2, 0}, {1,  -2}, {-2, 1}},
        // 0 -> L
        {{0, 0}, {-1, 0}, {2,  0}, {-1, 2},  {2,  -1}}
};

static const vector<vector<vector<int> > > OTHERS_KICK_TABLE = {
        // 0 -> R
        {{0, 0}, {-1, 0}, {-1, 1},  {0, -2}, {-1, -2}},
        // R -> 0
        {{0, 0}, {1,  0}, {1,  -1}, {0, 2},  {1,  2}},
        // R -> 2
        {{0, 0}, {1,  0}, {1,  -1}, {0, 2},  {1,  2}},
        // 2 -> R
        {{0, 0}, {-1, 0}, {-1, 1},  {0, -2}, {-1, -2}},
        // 2 -> L
        {{0, 0}, {1,  0}, {1,  1},  {0, -2}, {1,  -2}},
        // L -> 2
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 2},  {-1, 2}},
        // L -> 0
        {{0, 0}, {-1, 0}, {-1, -1}, {0, 2},  {-1, 2}},
        // 0 -> L
        {{0, 0}, {1,  0}, {1,  1},  {0, -2}, {1,  -2}}
};

/* LAST ACTION */
static constexpr int MOVE_LEFT = 1, MOVE_RIGHT = 2, CW_ROTATION = 3, CCW_ROTATION = 4;

/*************** END OF SRS KICK TABLE *****************/

class TetrisEngine {
public:
    double dExpectedSleepTime = 0.0;
    double dActualSleepTime = 0.0;
private:
    /**** configurations ********/
    TetrisConfig *config;

    /** static config, cannot be changed within the context of the Engine **/
    bool showGhostPiece = true; // if ghost piece is displayed or not
    // how many actions can be done before the piece locks in
    int pieceMovementThreshold = 15;
    // if the SRS system should be used
    public: bool useSRS = true;
    // line clears delay, after a piece locks in
    // Delay duration for cleared lines to remain empty before the board updates.
    // This creates a visual effect of gravity, simulating a brief pause
    // before the remaining tetrominoes fall down.
    // The user CANNOT interact with the game during this period
    private: int lineClearsDelay = 0;

    /** dynamic config, CAN be changed within the context of the Engine **/
    double softDropFactor = 24; // TETR.IO replication, default is 24, max can be 1_000_000
    // gravity, in G (TGM based)
    double defaultGravity = 0.0156; // 0.0156 cells per tick
    // lock delay, 0.5s by default (half of target tick-rate)
    int lockDelay = 0.5 * 60;
    // hold toggle, different from the HOLD flag that the context uses (canHold)
    bool holdEnabled = true;
    /**** end of configurations ********/

    // playfield related stuff
    public: vector<vector<int> > playfield{10, vector<int>(40, 0)}; // 10x40 matrix, a tetromino should spawn on the 22nd row

    // the active piece (falling)
    private: Tetromino *fallingPiece = nullptr;

    // the tetrominoes generator, can be implemented using the given interface (JAVA EXCLUSIVE, IN C++, ITS VIRTUAL)
    TetrominoGenerator *pieceGenerator = nullptr;

    // if hold is available or not
    bool canHold = true;

public:
    // external fields related to the gameplay core
    int lastSpinKickUsed = 0; // 0 is no kick, > 1 is kick
    vector<int> lastKickPositionUsed = {0, 0}; // 0 is no kick, > 1 is kick

public:
    int comboCount = -1; // the combo amount (2+ consecutive line clears w/o break), the combo begins at 2 lines

    // pieces queue and hold piece
    MinoTypeEnum *holdPiece = nullptr; // the hold piece will be "spawned" again when recall
    queue<MinoTypeEnum *> nextQueue; // the next queue

    // actual gravity variable that will be used by the game loop, (NOT the one you SHOULD EVER FUCKING TOUCH!!!!!!!),
    // @see defaultGravity
    // IF YOU WANT TO SET IT YOU FUCKING NIGGER
    // this can be multiplied with the SDF to make soft drop
    double gravity = this->defaultGravity;

    // internal systems flags / values
    LONG ticksPassed = 0;
    LONG lastTickTime = 0;
    LONG startedAt = -1;

    // indicates whether the engine is currently started or not
    bool started = false;
    // indicates if the engine has been stopped
    bool stopped = false;

    // indicates if the user should top out (WARNING: This value is FUNCTIONALLY different from 'stopped'
    // and will be reset to "false" after executing the onTopOut() block)
    bool shouldTopOut = false;

    // indicates whether the clear delay task is currently active
    // this will temporarily halt the piece spawning process
    bool clearDelayActive = false;

    // external calls for various engine events
    function<void()> onTickBeginCallback = nullptr; // run at every tick end
    function<void()> onTickEndCallback = nullptr; // run at every tick end
    function<void()> onTopOutCallback = [this] {
        this->stop();
    }; // game over, duh
    function<void(int)> onMinoLockedCallback = nullptr; // runs on a mino locked
    function<void(PlayfieldEvent)> onPlayfieldEventCallback = nullptr; // on special actions
    function<void(int)> onComboCallback = nullptr; // on user do a combo
    function<void(int)> onComboBreaksCallback = nullptr; // on user broke the combo

    // input buffering
    bool holdButtonPressed = false; /* HOLD button buffering */

    // task manager
    map<LONG, vector<function<void()>>> scheduledTasks;

public:
    /**
     * Schedules a task to be executed after a certain number of ticks.
     *
     * @apiNote A delay of 0 <b>not be executed immediately</b>; the task will be scheduled for the next tick!
     *
     * @param ticks The delay in ticks after which the task should be executed.
     * @param task   The task to be executed (must implement Runnable).
     * @return       The tick number at which the task is scheduled to be executed.
     * @throws IllegalArgumentException if the task is null.
     */
    LONG scheduleDelayedTask(const LONG ticks, const function<void()> &task) {
        if (task == nullptr) throw invalid_argument("Task could not be null!");
        const LONG execOnTick = ticksPassed + ticks;
        scheduledTasks[execOnTick].push_back(task);
        return execOnTick;
    };

    /**
     * Cancel every tasks scheduled for this tick, use with care
     * @param tickExecuted The tick number at which your tasks are expected to be executed
     */
    void cancelTask(const LONG tickExecuted) {
        scheduledTasks.erase(tickExecuted);
    }

    /**
     * Initialize a Modern, Guideline-compliant Tetris Engine
     * @see https://tetris.wiki/Tetris_Guideline
     *
     * @param config     the configuration instance of engine behaviors
     * @param generator  the pieces generator to use
     */
    TetrisEngine(TetrisConfig *config, TetrominoGenerator *generator) {
        this->config = config;

        // configuration: static config will be set ONCE but dynamic ones (can be changed after TetrisConfig build)
        // can be updated on demand
        // static fields (cant be changed after the initial build)
        this->showGhostPiece = config->ghostPieceEnabled;
        this->useSRS = config->srsEnabled;
        this->pieceMovementThreshold = abs(config->pieceMovementThreshold);
        this->lineClearsDelay = (int) round(abs(config->lineClearsDelaySecond) * EngineTimer::TARGETTED_TICK_RATE);

        // dynamic field will be set using a method (can be used outside the engine too)
        this->updateMutableConfig();

        // initialize the piece generator with given seed
        this->pieceGenerator = generator;
        // push the entire first bag to the queue
        auto bag = this->pieceGenerator->grabTheEntireBag();

        // start the NEXT queue
        for (auto piece: bag) {
            nextQueue.push(piece);
        }
    }

    /**
     * Updates mutable configuration settings for the Tetris Engine based on the current configuration
     * core {@link TetrisConfig}\endlink
     */
    void updateMutableConfig() {
        // if the user can press HOLD
        this->holdEnabled = config->holdEnabled;
        // lock delay = |seconds| * tickrate
        this->lockDelay = (int) round(abs(config->secondsBeforePieceLock) * EngineTimer::TARGETTED_TICK_RATE);
        // the soft drop scalar (soft-drop factor)
        this->softDropFactor = abs(config->softDropFactor);
        // update gravity amount
        this->defaultGravity = abs(config->gravity);
        this->gravity = defaultGravity;
    }

    /**
	 * Registers a runnable to execute at the end of each tick. If the total time
	 * of a tick exceeds 16.6ms, the game logic WILL slow down.
	 * @apiNote Tick compensation may occur, use this defensively
	 *
	 * @param runnable The code to execute at the end of a tick.
	 */
    void runOnTickEnd(function<void()> runnable) {
        this->onTickEndCallback = std::move(runnable);
    }

    /**
     * Registers a runnable to execute at the end the game, when the player
     * lost
     *
     * @warning Ensure to either <b>KILL the game loop</b> using stop() or clear the board
     * before spawning the next mino. Failing to do so can result in the next mino
     * being blocked upon spawn, leading to undefined behavior. This includes, but
     * is not limited to, collisions on spawn, game freezes, or invalid board states.
     *
     * @apiNote Default behaviour: []{ this->stop(); }
     *
     * @param runnable When the game ends (top out).
     */
    void runOnGameOver(function<void()> runnable) {
        this->onTopOutCallback = std::move(runnable);
    }

    /**
     * Registers a runnable to execute when a tetromino is locked to
     * the playfield
     *
     * @param onMinoEvent The code to execute, accepting the lines cleared
     * by that action
     */
    void runOnMinoLocked(function<void(int)> onMinoEvent) {
        this->onMinoLockedCallback = std::move(onMinoEvent);
    }

    /**
     * Registers a consumer to handle playfield events. The provided consumer will
     * be invoked whenever a playfield event occurs. (e.g. A T-Spin, a line clear...)
     *
     * @param onPlayfieldEvent The consumer to handle the playfield event.
     */
    void onPlayfieldEvent(function<void(PlayfieldEvent)> onPlayfieldEvent) {
        this->onPlayfieldEventCallback = std::move(onPlayfieldEvent);
    }

    /**
     * Registers a consumer to handle combo events. The provided consumer will be
     * invoked whenever the user performs a combo.
     *
     * @param onCombo The consumer to handle the combo event, accepting the current
     *                combo count as an argument.
     */
    void onCombo(function<void(int)> onCombo) {
        this->onComboCallback = std::move(onCombo);
    }

    /**
     * Registers a consumer to handle combo break events. The provided consumer will
     * be invoked whenever the user breaks a combo.
     *
     * @param onComboBreaks The consumer to handle the combo break event, accepting
     *                      the last combo count before the break.
     */
    void onComboBreaks(function<void(int)> onComboBreaks) {
        this->onComboBreaksCallback = std::move(onComboBreaks);
    }

    /**
     * Stop the gameloop (this instance cannot recover from this)
     *
     * This will stop the gameloop and invalidate every user interactions
     * after this method call
     */
    void stop();

    /**
     * Moves the falling piece one unit to the left if it exists.
     * This method delegates the horizontal movement to the falling piece's
     * translation method.
     */
    void moveLeft();

    /**
     * Moves the falling piece one unit to the right if it exists.
     * This method delegates the horizontal movement to the falling piece's
     * translation method.
     */
    void moveRight();

    /**
     * Rotates the falling piece clockwise if it exists.
     *
     * This method delegates the rotation to the falling piece's rotate method.
     */
    void rotateCW();

    /**
     * Rotates the falling piece counterclockwise if it exists.
     *
     * This method delegates the rotation to the falling piece's rotate method.
     */
    void rotateCCW();

    /**
     * Toggles the soft drop feature, which increases the falling speed of the
     * current piece. The speed increase is determined by the softDropFactor.
     *
     * @param on A boolean indicating whether to enable or disable soft drop.
     */
    void softDropToggle(bool on);

    /**
     * Instantly drops the falling piece to the lowest possible position.
     *
     * This method calls the hardDrop method of the falling piece,
     * which moves it directly to the bottom of the playfield.
     */
    void hardDrop();

    /**
     * Holds the current falling piece, if it exists.
     *
     * This method calls the onUserHold() method indirectly to handle the logic
     * of saving the piece and potentially placing a new piece.
     */
    void hold();

    /**
     * Get the current combo count
     * @return combo count
     */
    int getComboCount() const {
        return max(0, comboCount);
    }

    /**
     * If hold is available or not (also return false when DISABLED)
     * @return true if available
     */
    bool canUseHold() const {
        return this->holdEnabled && this->canHold;
    }

    /**
	 * Get the hold piece type
	 * @return MinoTypeEnum of the current hold piece
	 */
    MinoTypeEnum* getHoldPiece() const {
        return this->holdPiece;
    }

    /**
     * Get the NEXT queue
     * @return the current next queue
     */
    queue<MinoTypeEnum* > &getNextQueue() {
        return this->nextQueue;
    }

    /**
     * Get the falling tetromino type
     * @return the type of the falling tetromino
     */
    MinoTypeEnum *getFallingMinoType();

    /**
     * Get the amount of ticks passed since the start
     */
     LONG getTicksPassed() const {
         return this->ticksPassed;
     }

    /**
     * Resets the playfield of this instance (matrix).
     * The Hold piece and Next queue are left intact, preserving their state.
     *
     * @apiNote Use with caution.
     */
    void resetPlayfield() {
        playfield.assign(10, std::vector<int>(40, 0));
    }

    /**
	 * Raises garbage lines on the playfield by shifting cells upwards and filling
	 * new lines
	 *
	 * @param height    the number of garbage lines to rise
	 * @param holeIndex the index of the column that will have a hole (empty cell)
	 *                  in the new lines
	 */
    void raiseGarbage(int height, int holeIndex);

private:
    mutable std::vector<std::vector<int>> clonedPlayfield;

public:
    /**
     * Returns a copy of the current playfield with the falling piece, if any,
     * merged into it. The cloned playfield includes the type of the falling
     * piece placed at its current position.
     *
     * @apiNote Ghost pieces - if enabled, will be drawn using a specific
     * convention (e.g., a constant value defined as GHOST_PIECE_CONVENTION).
     * These "ghost" pieces will be placed on the board where the piece would
     * land if it continued to fall without any rotation.<br>
     * <br>
     * Falling pieces - while still falling (not locked to the board yet),
     * will be represented by their negative type value. This negative value
     * differentiates falling pieces from those that are already placed on the
     * board. If you need to ignore this distinction and treat all pieces
     * equally, consider using `abs()` (from `cmath`) to retrieve the absolute value of
     * the piece's type.
     *
     * @return A cloned 2D array representing the game board with the current
     *         falling piece incorporated.
     */
    const vector<vector<int>> &getBoardBuffer() const;

    /**
     * Check if there's a mino at given coordinates
     *
     * @param x x-position
     * @param y y-position
     * @return true if has a mino at that position, or, out of bounds
     */
    bool hasMinoAt(const int x, const int y) const {
        return (x < 0 || y < 0 || x >= playfield.size() || y >= this->playfield[0].size()) ||
               (this->playfield[x][y] > 0);
    }

    /******************** INTERNAL IMPLEMENTATION OF THE TETRIS ENGINE ********************/
private:
    // this will start after the start() method
    void onEngineStart();

    // this will run every single tick
    void onTickRun();

    // on user hold
    void onUserHold();

public:
    // on mino placed
    void onMinoLocked(Tetromino *locked);

private:
    // ID of the scheduled task that locks a piece after it lands
    LONG pieceLockTaskId = -1;
public:
    // counter to track how many manipulations (moves/rotations) have been made
    mutable int manipulationCount = 0;

    // called when a piece is manipulated (rotated, moved by the player)
    void onPieceManipulation();

private:
    // accumulated cell move
    double cellMoved = 0.0;

    // this runs on each tick and simulates the effect of gravity on the piece
    void moveCellOnGameGravity();

    // row manipulation
    // nullify a row by setting all of its cells to empty (0)
    // this creates the "line-disappear" effect
    void nullifyRow(const int rowIndex) {
        // makes an animation to "wipe" the line (this should not be included in
        // the base engine, this is for university project only)
        for (int x = 0; x < 10; ++x) {
            int minoDelay;
            if ((minoDelay = lineClearsDelay / 10) <= 1) {
                this->playfield[x][rowIndex] = 0;
                continue;
            }
            // only play animation if the time budget is > 1 frames
            scheduleDelayedTask(x * minoDelay, [this, rowIndex, x]() {
               this->playfield[x][rowIndex] = 0;
            });
        }
    }

    // clear the row by shifting down all rows above it by one
    void clearRow(const int rowIndex) {
        for (int y = rowIndex; y > 0; --y) {
            for (auto &x: this->playfield) {
                x[y] = x[y - 1]; // drag the block ABOVE it down, replacing itself
            }
        }
    }

public:
    // true if the row is empty
    bool isRowEmpty(const int rowIndex) const {
        for (auto &x: this->playfield) {
            if (x[rowIndex] != 0) return false;
        }
        return true;
    }

private:
    // this will run whenever a piece is locked in the playfield
    void updatePlayfieldState(Tetromino *locked);

    // internal function
    void updatePlayFieldLineClears(const vector<int> &clearedLines);

    /**
	 * Appends a new piece generated by the piece generator to the next queue.
	 */
    void appendNextQueue() {
        // Adds the next piece to the NEXT queue, ensuring that the queue contains
        // at least the total number of available tetromino types.
        do {
            this->nextQueue.push(this->pieceGenerator->next());
        } while (this->nextQueue.size() < MinoType::valuesLength);
    }

    /**
     * Places the next piece from the queue into the playfield.
     * This method will append a new piece generated by the piece
     * generator to the next queue and signals the main game loop
     * to "put" the next piece to the playfield instead of doing it
     * by itself
     */
    void pushNextPieceToPlayfield();

    /**
     * Spawn a Tetromino in the playfield, movable by the player
     * @param type
     */
    void putPieceInPlayfield(MinoTypeEnum *type);

    /**
	 * Start the game loop
	 */
    void gameLoopStart();

    /**
     * Memory management bullshit, don't use
     */
    vector<Tetromino *> deletionQueue; // queue to delete objects when they're out of scope
    /**
     * Mark falling piece as null, will be removed once
     * <code>freeMemoryOfFallingPiece()</code> is called
     */
    void markFallingPieceAsNull();

    /**
     * Should only be called when it is CRUCIAL to free
     * the memory occupied by <code>this->prepareToDelete</code>
     */
    void freeMemoryOfFallingPiece();

public:
    /**
     * Start the Tetris Engine, beginning to accept user inputs
    */
    void start() {
        this->gameLoopStart();
    }

public:
    /**
     * Internal debugging bullshit, dont use
     */
    void printBoard() const;
};

class Tetromino {
public:
    int x = 0, y = 0; // Current coordinates of the top-left corner of the Tetromino on the playfield grid.
    MinoTypeEnum *type = nullptr; // The type of Tetromino (e.g., T_MINO, Z_MINO, L_MINO) being represented.
    int rotationState = 0; // 0, R, 2, L represented as 0, 1, 2, 3
    int size; // The size of the Tetromino's bounding box (usually 3x3).

    bool locking = false; // if the piece is getting locked
    int lastActionDone = 0; // 0 = nothing, 1 = move left, 2 = move right, 3 = cw, 4 = ccw

    // link the parent
    TetrisEngine *parent;

    explicit Tetromino(TetrisEngine *parent, MinoTypeEnum *type) : type() {
        this->parent = parent;
        this->type = type;
        // the size of this tetromino bounding box, ranging from 2 (O piece) to 4 (I piece)
        this->size = static_cast<int>(type->getStruct(0).size());
        parent->manipulationCount = 0; // new piece, 0 manipulation
    }

    ~Tetromino() = default;

    /**
     * Gets the matrix structure of this tetromino in the current rotation state.
     *
     * @return a 2D array representing the current structure of the tetromino
     */
    [[nodiscard]] const vector<vector<int> > &getStruct() const {
        return type->getStruct(rotationState);
    }

    /**
    * Checks if this tetromino occupies the specified x, y position on the board.
     *
    * @deprecated In favor of getRelativeMinoCoordinates()
    *
    * @param x the x-coordinate on the board
    * @param y the y-coordinate on the board
    * @return true if the tetromino occupies the specified coordinates, false otherwise
    */
    [[maybe_unused]] [[nodiscard]] bool occupyAt(const int x, const int y) const {
        if (this->x <= x && x < this->x + size // b1 <= x < b1 + size_t
            && this->y <= y && y < this->y + size) {
            // b1 <= y < b1 + size_t
            return getStruct()[y - this->y][x - this->x] >= 1;
        }
        return false;
    }

    /**
    * Gets the relative positions of each mino of this tetromino.
    *
    * @return a 2D array representing the x, y coordinates of each mino relative to the board
    */
    [[nodiscard]] vector<vector<int> > getRelativeMinoCoordinates() const {
        return this->getRelativeMinoCoordinates(this->x, this->y);
    }

    /**
     * Gets the relative positions of each mino of this tetromino from a specified offset.
     *
     * @param x the x-coordinate offset
     * @param y the y-coordinate offset
     * @return a 2D array representing the x, y coordinates of each mino relative to the board with the specified offset
     */
    [[nodiscard]] vector<vector<int> > getRelativeMinoCoordinates(const int x, const int y) const {
        const auto &minoStruct = this->getStruct(); // the structure of this mino with the rotation state applied
        // a Mino can have as many "minoes" inside them as you want, each has an x, y coordinate pair
        vector<vector<int> > relative;
        relative.reserve(type->blockCount);

        for (int ry = 0; ry < minoStruct.size(); ry++) {
            for (int rx = 0; rx < minoStruct[ry].size(); rx++) {
                if (const int mx = minoStruct[ry][rx]; mx != 0) {
                    // skip empty spaces (0s)
                    relative.push_back({x + rx, y + ry}); // add transformed coordinates
                }
            }
        }
        return relative;
    }

    /**
    * Checks if the tetromino can fit at the specified position on the board.
    *
    * @param ax the x-coordinate where the tetromino is to be placed
    * @param ay the y-coordinate where the tetromino is to be placed
    * @return true if the tetromino can fit, false otherwise
    */
    [[nodiscard]] bool canFitBeingAt(const int ax, const int ay) const {
        for (const auto &xyPair: getRelativeMinoCoordinates(ax, ay)) {
            const int x = xyPair[0], y = xyPair[1];
            // Check if the mino is out of bounds or collides with another mino
            if (parent->hasMinoAt(x, y)) {
                return false;
            }
        }
        return true;
    }

    static constexpr int R = 1; // right
    static constexpr int L = 3; // left
    // compass for rotation
    //      [0]
    // [3]  rot  [1]
    //      [2]

    /**
     * Gets the kick sequence based on the initial and target rotation states.
     *
     * The kick sequence provides a series of potential moves to attempt when a rotation
     * collides with another piece or wall, trying to "kick" the tetromino into a valid position.
     *
     * @param initialState the origin state
     * @param finalState the target state
     * @return kick sequence
     */
    [[nodiscard]] vector<vector<int> > getKickSequenceCheck(const int initialState, const int finalState) const {
        // if SRS is not enabled, ignore the kick sequence, only allow basic rotation
        if (!parent->useSRS || type->ordinal == MinoType::O_MINO.ordinal) {
            // O Tetromino does not kick (how do u rotate an O)
            return {{0, 0}};
        }

        int index;
        // get which set of kick data to use based on the initial and final state
        if (initialState == 0 && finalState == R) index = 0;
        else if (initialState == R && finalState == 0) index = 1;
        else if (initialState == R && finalState == 2) index = 2;
        else if (initialState == 2 && finalState == R) index = 3;
        else if (initialState == 2 && finalState == L) index = 4;
        else if (initialState == L && finalState == 2) index = 5;
        else if (initialState == L && finalState == 0) index = 6;
        else if (initialState == 0 && finalState == L) index = 7;
        else index = -1;

        // I-pieces use a different kick table because they're longer
        return (type->ordinal == MinoType::I_MINO.ordinal ? I_KICK_TABLE : OTHERS_KICK_TABLE)[index];
    }

    /**
    * Rotates the tetromino.
    *
    * If a rotation is not possible due to a collision, the function tries applying a kick.
    * If none of the kicks succeed, the rotation is reverted.
    *
    * @param ccw true if the rotation is counter-clockwise, false if clockwise
    */
    void rotate(const bool ccw) {
        // store the variables to reverse the changes when needed
        const int initialRotation = this->rotationState;

        // update the rotation state (range 0-3)
        this->rotationState = (initialRotation + (ccw ? -1 : 1) + 4) % 4;

        // kick sequence based on initial and target rotation states
        auto kickSequence = this->getKickSequenceCheck(initialRotation, this->rotationState);

        bool validMove = false; // if any kick results in a valid move

        int kickUsed = -1;
        // try each kick offset in the sequence
        for (vector<int> kick: kickSequence) {
            // "kick" the tetromino to the new position
            const int testingX = this->x + kick[0];
            const int testingY = this->y - kick[1]; // the board is upside down, so i subtract instead of add bruh, index wise

            // increment the kick identifier
            ++kickUsed;

            // this will return false if the tetromino won't fit
            if (canFitBeingAt(testingX, testingY)) {
                // set the new position
                this->x = testingX;
                this->y = testingY;
                validMove = true; // A valid kick has been found
                break; // Exit the loop as the rotation succeeded
            }
        }

        // If no valid move was found, revert to the initial position
        if (!validMove) {
            this->rotationState = initialRotation;
        } else {
            // if the kick used is NOT 0 (initial kick), then it was a valid "kick"
            parent->lastSpinKickUsed = kickUsed;
            // the kick offset used (this will be used for T-Spin detection)
            parent->lastKickPositionUsed = kickSequence[kickUsed];

            // a successful move
            parent->onPieceManipulation();
            // last action of this piece
            this->lastActionDone = ccw ? CCW_ROTATION : CW_ROTATION;

            // invalidate the ghost piece cache, forcing a recalculation
            this->invalidateGhostPieceCache();
        }
    }

    /**
    * Locks the tetromino in place, overriding the occupied playfield
    * positions
    */
    void lockIn() {
        for (vector<int>& minoPosition : getRelativeMinoCoordinates()) {
            // get the position relative to the playfield and set the cell
            // to this tetromino color (type). This step is very important
            // because the color presents itself as the "presence" of a piece (color > 0 == present)
            parent->playfield[minoPosition[0]][minoPosition[1]] = type->ordinal + 1;
        }
        parent->manipulationCount = 0; // reset everything all over
        parent->onMinoLocked(this); // fire the event
    }

    /**
     * Yank the piece to the bottom of the stack
     */
    void hardDrop() {
        do {
        } while (translateDown());
        this->lockIn();
    }

    /**
     * Translate left or right by 1 cell
     * @param left the side to translate to
     * @return true if can move, false if not
     */
    bool translateHorizontally(const bool left) {
        if (!this->canFitBeingAt(x + (left ? -1 : 1), y)) return false;

        this->x += left ? -1 : 1;
        parent->onPieceManipulation();

        // last action of this piece, 1 = left, 2 = right movement
        this->lastActionDone = left ? MOVE_LEFT : MOVE_RIGHT;

        // invalidate the ghost piece cache, forcing a recalculation (ghost pieces do not care about Y)
        this->invalidateGhostPieceCache();
        return true;
    }

    /**
     * Translate down 1 cell
     * @return true if can go down, false if not
     */
    bool translateDown() {
        if (onGround()) return false;
        ++this->y; // this feels cursed right? the board is upside down, so live with it
        return true;
    }

    /**
     * Checks if the falling piece is on the ground.
     *
     * @return True if the piece cannot fit one unit down, indicating it is on the ground;
     *         otherwise, returns false.
     */
    [[nodiscard]] bool onGround() const {
        return !this->canFitBeingAt(x, y + 1);
    }

    vector<vector<int> > cachedGhostPiecePosition;

    /**
     * Calculates the position of the ghost piece for the current tetromino.
     * @apiNote The ghost piece will only be re-calculated during movement along the X-axis
     *
     * @return a 2D array of [x, y] coordinates representing the position of the ghost piece's minos
     */
    vector<vector<int> > getGhostPiecePosition() {
        if (!cachedGhostPiecePosition.empty()) return this->cachedGhostPiecePosition;
        int ghostY = this->y; // Start with the current y position of the tetromino
        // keep moving the ghost down until it can't move any further
        while (canFitBeingAt(this->x, ghostY + 1)) {
            ghostY++;
        }
        return cachedGhostPiecePosition = this->getRelativeMinoCoordinates(this->x, ghostY);
    }

    /**
     * Invalidate the ghost piece cache, forcing a recalc
     */
    void invalidateGhostPieceCache() {
        this->cachedGhostPiecePosition.clear();
    }
};

/************** BEGIN OF MEMORY MANAGEMENT BULLSHIT ***************/
inline void TetrisEngine::markFallingPieceAsNull() {
    this->deletionQueue.push_back(this->fallingPiece);
    this->fallingPiece = nullptr;
}

inline void TetrisEngine::freeMemoryOfFallingPiece() {
    for (Tetromino* piece: deletionQueue) {
        // actually deleting the shit
        delete piece;
    }
    deletionQueue.clear();
}
/****** END OF BULLSHIT (not really, the entire code is bs) ***********/

inline void TetrisEngine::stop() {
    if (stopped) throw logic_error("Already stopped!");
    this->stopped = true;

    if (this->fallingPiece != nullptr) {
        this->markFallingPieceAsNull();
    }
    // because stop essentially kills the entire engine,
    // there's no point waiting when to delete anymore
    this->freeMemoryOfFallingPiece();
}

inline void TetrisEngine::moveLeft() {
    if (this->fallingPiece != nullptr) fallingPiece->translateHorizontally(true);
}

inline void TetrisEngine::moveRight() {
    if (this->fallingPiece != nullptr) fallingPiece->translateHorizontally(false);
}

inline void TetrisEngine::rotateCW() {
    if (this->fallingPiece != nullptr) fallingPiece->rotate(false);
}

inline void TetrisEngine::rotateCCW() {
    if (this->fallingPiece != nullptr) fallingPiece->rotate(true);
}

inline void TetrisEngine::softDropToggle(const bool on) {
    this->gravity = this->defaultGravity;
    if (on) this->gravity *= softDropFactor;
}

inline void TetrisEngine::hardDrop() {
    if (this->fallingPiece != nullptr) fallingPiece->hardDrop();
}

inline void TetrisEngine::hold() {
    if (this->fallingPiece != nullptr) this->holdButtonPressed = true;
    // Signals the main game loop to execute onUserHold()
}

inline MinoTypeEnum *TetrisEngine::getFallingMinoType() {
    if (fallingPiece != nullptr) return fallingPiece->type;
    return nullptr;
}

inline void TetrisEngine::raiseGarbage(int height, int holeIndex) {
    // because the board is ACTUALLY not physically shifted during the clear delay active period
    // raising garbage during this time will cause the board to fracture, leaving behind empty lines
    if (clearDelayActive) {
        cerr << "You should NOT interact with the board during clear delay, it will fuck the board up!" << endl;
        return;
    }

    // prerequisites, if height is too high or holeIndex is out of bounds, fuck off
    if (height >= this->playfield[0].size() || holeIndex >= this->playfield.size()) {
        throw invalid_argument("What is wrong with you?");
    }

    // if there is no height to raise, return (probably user error)
    if (height <= 0 || holeIndex < 0) return;

    // from top to bottom
    // for each row, starting from the top to where the garbage starts rising
    // shift everything upwards by "height" units
    for (int y = 0; y < this->playfield[0].size() - height; ++y) {
        for (int x = 0; x < this->playfield.size(); ++x) {
            // for each cell in the current row, copy the cell "height" rows below it
            playfield[x][y] = playfield[x][y + height];
        }
    }

    // now fill the new garbage lines with blocks, leaving a hole at "holeIndex"
    for (int y = this->playfield[0].size() - height; y < this->playfield[0].size(); ++y) {
        for (int x = 0; x < this->playfield.size(); ++x) {
            playfield[x][y] = holeIndex == x ? 0 : GARBAGE_MINO_CONVENTION; // 0 for the "air"
        }
    }

    if (this->fallingPiece != nullptr) {
        this->fallingPiece->invalidateGhostPieceCache();
    }
}

inline const vector<vector<int> > &TetrisEngine::getBoardBuffer() const {
    if (fallingPiece == nullptr)
        return playfield; // no copy if there's nothing to modify (this is important for memory)
    clonedPlayfield = playfield; // shallow copy (no deep)

    int fallingPieceType = fallingPiece->type->ordinal + 1; // the piece type (ordinal + 1), because 0 is air
    if (showGhostPiece) {
        // ghost pieces will have a specific convention in the array
        for (const auto &ghostPos: fallingPiece->getGhostPiecePosition()) {
            clonedPlayfield[ghostPos[0]][ghostPos[1]] = GHOST_PIECE_CONVENTION;
        }
    }
    // the falling piece
    for (const auto &minoPos: fallingPiece->getRelativeMinoCoordinates()) {
        // if the piece is "falling" (not locked to the board yet)
        // the color index will be the negative version of normal minos.
        // To ignore this, use abs()
        clonedPlayfield[minoPos[0]][minoPos[1]] = -fallingPieceType;
    }
    return clonedPlayfield;
}

/******************** INTERNAL IMPLEMENTATION OF THE TETRIS ENGINE ********************/
// this will start after the start() method
inline void TetrisEngine::onEngineStart() {
    this->pushNextPieceToPlayfield();
}

// this will run every single tick
inline void TetrisEngine::onTickRun() {
    // handle gravity
    this->moveCellOnGameGravity();
}

// on mino placed
inline void TetrisEngine::onMinoLocked(Tetromino *locked) {
    // allow user to hold again
    this->canHold = true;
    // new mino
    this->updatePlayfieldState(locked);
    // Place the next piece in playfield, this also generates
    // a new piece at the end of the queue
    this->pushNextPieceToPlayfield();
}

// on user hold
inline void TetrisEngine::onUserHold() {
    if (!canUseHold()) return; // return if holding is not allowed or disabled altogether
    MinoTypeEnum *toHold = this->fallingPiece->type; // get & store the type of the falling piece

    /*
     * When the player presses HOLD, if there is no currently held piece, the system takes the falling piece and
     * places it in the HOLD slot. It then spawns a new piece from the queue, as if the held piece had been placed.
     * If a piece is already held, the system swaps the currently falling piece with the one in the HOLD slot.
     */

    // if a hold piece exists, place it into the playfield and push a new block from the generator into the queue.
    // otherwise, move the currently held piece into the playfield, effectively swapping it.
    if (this->holdPiece != nullptr) {
        this->putPieceInPlayfield(this->holdPiece);
    } else {
        this->pushNextPieceToPlayfield();
    }

    // update holdPiece with the current falling piece and disable holding until the next piece is placed.
    this->holdPiece = toHold;
    this->canHold = false; // disable further holding until the next piece is placed
}

// called when a piece is manipulated (moved, rotated by the player)
inline void TetrisEngine::onPieceManipulation() {
    // if the player has not exceeded the allowed manipulation count
    // (rotating or moving the piece too much)
    if (manipulationCount < pieceMovementThreshold) {
        // cancel any scheduled task that would lock the piece in place
        cancelTask(this->pieceLockTaskId);
        // reset the task ID as no lock task is active anymore
        this->pieceLockTaskId = -1;
    }
        // otherwise,
        // if there is an active lock task and a falling piece exists
    else if (this->pieceLockTaskId != -1 && fallingPiece != nullptr) {
        // force the piece to perform a hard drop, locking it instantly
        hardDrop();
    }

    // increment the manipulation counter since a rotation just occurred
    manipulationCount++;
}

// This runs on each tick and simulates the effect of gravity on the piece
inline void TetrisEngine::moveCellOnGameGravity() {
    // accumulate the movement caused by gravity in each tick
    cellMoved += gravity;

    // once cellMoved reaches or exceeds 1 (a full cell downward movement)
    if (cellMoved >= 1) {
        // move the piece down by the number of full cells accumulated
        for (int cm = 0; cm < round(cellMoved); cm++) {
            if (fallingPiece != nullptr) {
                // try to move the piece down by one cell
                // if the piece can't move down further (landed) and no lock task is active
                if (const bool landed = !fallingPiece->translateDown(); landed && this->pieceLockTaskId == -1) {
                    Tetromino *piece = this->fallingPiece; // store a reference to the current piece
                    piece->locking = true;

                    // schedule a delayed task to lock the piece after the lockDelay (default 30 ticks; half a sec) time
                    this->pieceLockTaskId = scheduleDelayedTask(lockDelay, [piece, this] {
                        // after the delay, reset the task ID
                        this->pieceLockTaskId = -1;

                        // check if the piece is still the same and is still on the ground
                        if (piece != nullptr && piece == this->fallingPiece && this->fallingPiece->onGround()) {
                            // if so, lock the piece in place
                            fallingPiece->lockIn();
                        }
                    });
                    break; // stop moving the piece down after scheduling the lock
                }
            }
        }
        // reset cellMoved to 0 after applying downward movement
        cellMoved = 0;
    }
}

inline void TetrisEngine::updatePlayfieldState(Tetromino *locked) {
    // flags for the event
    bool isSpin = false;
    bool isMiniSpin = false;

    // tea-spin detection
    // only trigger if the locked piece is a T mino AND the last action was ROTATE (CCW and CW)
    if (locked->type->ordinal == MinoType::T_MINO.ordinal &&
        (locked->lastActionDone == CW_ROTATION || locked->lastActionDone == CCW_ROTATION)) {
        // Assuming you track whether the last move was a rotation:
        const int lockedX = locked->x, lockedY = locked->y;

        // Check the 3x3 grid around the center of the T piece
        // Because the Tetromino#x and #y are not relative to the center
        // we gonna treat it as relative to 0, 0
        bool c1 = hasMinoAt(lockedX, lockedY) // upper left
        , c2 = hasMinoAt(lockedX + 2, lockedY) // upper right
        , c3 = hasMinoAt(lockedX, lockedY + 2) // lower left
        , c4 = hasMinoAt(lockedX + 2, lockedY + 2); // lower right

        // the lookup table
        vector<vector<bool> > tMinoRelative = {
                {c1, c2, /*back*/ c3, c4},  // 0
                {c2, c3, /*back*/ c1, c4},  // 1
                {c3, c4, /*back*/ c1, c2},  // 2
                {c4, c1, /*back*/ c2, c3}   // 3
        };

        // the front and back of the T mino relative to the rotation state
        vector<bool> pair = tMinoRelative[locked->rotationState];
        // 2 minoes in the front stem [0*0] filled
        //							  [***]
        // and at least 1 in the back [1-1] filled
        if (pair[0] && pair[1] && (pair[2] || pair[3])) {
            isSpin = true; // flag for T-Spin exclusive
            isMiniSpin = false; // foolproof
        } else
            // 2 minoes in the back 	  [1*1] filled
            //							  [***]
            // and  2 in the back 		  [0-0] filled
        if (pair[2] && pair[3] && (pair[0] || pair[1])) {
            // for ALL mini T-spin that moves the piece 1 by 2 (https://tetris.wiki/T-Spin)
            // "upgrade" it to a NORMAL T-Spin
            if (lastSpinKickUsed != 0 && lastKickPositionUsed[0] == 1 && lastKickPositionUsed[1] == 2) {
                isSpin = true; // flag for T-Spin exclusive
                isMiniSpin = false; // foolproof
            } else {
                isSpin = false; // foolproof
                isMiniSpin = true; // flag for any type of mini spin
            }
        }
    }

    // DETECT mini spins (except T) [mimicking the TETR.IO All-Spin ruleset]
    // only if not a T piece AND last action is ROTATE
    if (locked->type->ordinal != MinoType::T_MINO.ordinal &&
        (locked->lastActionDone == CW_ROTATION || locked->lastActionDone == CCW_ROTATION)) {
        // if the last rotation was a kick, it is considered a mini spin
        if (lastSpinKickUsed != 0) {
            isSpin = false; // foolproof
            isMiniSpin = true; // flag for any type of mini spin
        }
    }

    // line clears
    vector<int> clearedLines;
    bool perfectClear = true; // pc flag

    // 0 = up; 39 = bottom
    // top -> down
    for (int y = 0; y < this->playfield[0].size(); ++y) {
        // if the row is empty, skip this check altogether
        if (isRowEmpty(y)) continue;
        bool hasHoles = false;

        // Check if the row has any holes
        for (auto &x: playfield) {
            if (x[y] == 0) {
                hasHoles = true;
                break;
            }
        }

        // if the row has no holes, clear the row and move rows above it down
        if (!hasHoles) {
            // turns the cleared row empty first, this won't shift the board, however.
            // the board is shifted in bulk at the end
            nullifyRow(y);
            clearedLines.push_back(y); // tell the listener which line got cleared
        } else { // even if a single row has a mino, this ain't a PC
            perfectClear = false;
        }
    }

    // update the combo counter
    if (clearedLines.size() > 0) {
        comboCount++; // increment a combo count
        if (comboCount > 0 && onComboCallback != nullptr) {
            onComboCallback(comboCount);
        }
    } else if (comboCount > 0) {
        if (onComboBreaksCallback != nullptr) {
            onComboBreaksCallback(comboCount);
        }
        comboCount = -1; // broke
    }

    // fire the event for user
    if (this->onMinoLockedCallback != nullptr) onMinoLockedCallback(clearedLines.size());

    // the playfield event emitter
    if (this->onPlayfieldEventCallback != nullptr &&
        (isMiniSpin || isSpin || perfectClear || clearedLines.size() > 0)) {
        // fire the event
        this->onPlayfieldEventCallback(
                PlayfieldEvent(
                        clearedLines,
                        perfectClear,
                        locked->type,
                        isSpin, isMiniSpin
                )
        );
    }

    // if the playfield event triggered a line clear (or more)
    // This block of code is the one that ACTUALLY updates the playfield
    // by shifting all rows down for each cleared line
    if (!clearedLines.empty()) {
        this->clearDelayActive = true; // activate clear delay, halting piece spawning temporarily
        if (lineClearsDelay > 0) {
            // schedule playfield update to clear lines after delay (if > 0)
            scheduleDelayedTask(lineClearsDelay, [this, clearedLines] {
                this->updatePlayFieldLineClears(clearedLines);
            });
        } else updatePlayFieldLineClears(clearedLines); // run instantly if 0
    }
}

// internal function
inline void TetrisEngine::updatePlayFieldLineClears(const vector<int> &clearedLines) {
    for (const int row: clearedLines) {
        clearRow(row);
    }
    this->clearDelayActive = false;
}

inline void TetrisEngine::pushNextPieceToPlayfield() {
    // append a new piece from the generator to the end
    // of the next queue
    this->appendNextQueue();
    // instead of pushing by itself, it will set it to null to signal
    // the main game loop to spawn the piece
    this->markFallingPieceAsNull();
}

inline void TetrisEngine::putPieceInPlayfield(MinoTypeEnum *type) {
    if (type == nullptr || stopped) return; // if stopped or topped out, return

    // this is when it's safe to delete older objects (because a new one is initialized below)
    // temu garbage collector
    this->freeMemoryOfFallingPiece();

    // create the dynamic tetromino instance (this will be placed on the heap)
    this->fallingPiece = new Tetromino(this, type);

    // set the initial X, Y position
    this->fallingPiece->x = (type->ordinal == MinoType::O_MINO.ordinal) ? 4 : 3;
    this->fallingPiece->y = static_cast<int>(playfield[0].size()) - 22; // the piece will always spawn on the 22nd row of the board

    // reset this measurement
    this->cellMoved = 0;

    // check if the user topped out
    if (!this->fallingPiece->canFitBeingAt(this->fallingPiece->x, this->fallingPiece->y)) {
        this->markFallingPieceAsNull();
        this->shouldTopOut = true; // this will signal the game loop to execute onTopOut
    }
}

inline void TetrisEngine::gameLoopStart() {
    if (this->stopped) throw logic_error("This instance has stopped! You must create a new instance!");
    if (this->started) throw logic_error("This instance is already started");

    // fire pre-start events
    this->onEngineStart();
    this->startedAt = System::currentTimeMillis();

    // the flag to look for
    this->started = true;

    for (;;) {
        // stop on break signal
        if (this->stopped) break;
        // count nanoseconds passed for tick compensation if needed
        auto tickTimeBegin = System::nanoTime();

        // run the external callback
        if (this->onTickBeginCallback != nullptr) {
            try {
                this->onTickBeginCallback();
            } catch (exception &e) {
                cerr << e.what() << endl;
            }
        }

        // if the falling piece is null, spawns a new one
        // only if the clear delay period is not active
        if (this->fallingPiece == nullptr && !clearDelayActive) {
            if (!nextQueue.empty()) {
                MinoTypeEnum *nextMino = nextQueue.front();
                nextQueue.pop();
                this->putPieceInPlayfield(nextMino);
            }
        }

        // hold handling
        if (this->holdButtonPressed) {
            if (this->fallingPiece != nullptr) {
                // c++ bullshittery
                this->onUserHold();
            }
            this->holdButtonPressed = false;
        }

        // run the main internal logic
        onTickRun();

        // scheduled task handling (this is more primitive than Java because of c++ libs)
        // find the first key that is strictly greater than ticksPassed
        auto it = scheduledTasks.upper_bound(ticksPassed);
        // if it is not the beginning, step back to get the greatest key <= ticksPassed
        if (it != scheduledTasks.begin()) {
            --it; // this is so fucking bad, why c++ don't have treemap
            // ensure that this key is indeed <= ticksPassed
            if (it->first <= ticksPassed) {
                // c++ debugger bullshitery
                LONG key = it->first;

                // remove the key from the map
                auto node = scheduledTasks.extract(key);
                vector<function<void()>> tasks = move(node.mapped());

                // execute one by one
                for (auto &task: tasks) {
                    task();
                }
            }
        }

        // run the external call
        if (this->onTickEndCallback != nullptr) {
            try {
                this->onTickEndCallback();
            } catch (exception &e) {
                cerr << e.what() << endl;
            }
        }

        // if top out, execute the onTopOut external call
        // Usually, onTopOut will be assigned to TetrisEngine#stop(), which will
        // stop the main game loop, thus trigger the `if (this.stopped) break;` above
        if (shouldTopOut) {
            // execute one last time (or not, the user can do sth to prevent `stop()` from being called
            if (this->onTopOutCallback != nullptr) {
                this->onTopOutCallback();
            }
            this->shouldTopOut = false; // the user may be creative and do something else with this

            // since the last tetromino caused a top-out, we clear memory right now
            this->freeMemoryOfFallingPiece();
        }

        // increment tick counter, used for scheduling
        ticksPassed++;

        // lost-ticks compensation mechanism
        this->lastTickTime = (System::nanoTime() - tickTimeBegin) / 1000000;
        double parkPeriod = max(0.0, EngineTimer::TICK_INTERVAL_MS - lastTickTime);

        this->dExpectedSleepTime = parkPeriod;
        LONG prev = System::currentTimeMillis();
        // it could be 0, which means: no sleep, execute immediately
        if (parkPeriod > 0) {
            Thread::sleep(parkPeriod);
        } // tick-rate cap
        this->dActualSleepTime = System::currentTimeMillis() - prev;
    }
}

inline void TetrisEngine::printBoard() const {
    cout << "Milliseconds-Per-Tick (last; included renderer ::stdout): " << lastTickTime << "ms" << endl;
    auto timePassed = (System::currentTimeMillis() - startedAt) / 1000.0;
    cout << "Ticks Per Second (approx.): " << (ticksPassed / timePassed) << " | target-tps: " << EngineTimer::TARGETTED_TICK_RATE << " " << "(ts: " << ticksPassed << " / tp: " << timePassed << ")" << endl;
    cout << "CPU Time: EXPECTED_SLEEP[" << dExpectedSleepTime << "] ACTUAL_SLEEP[" << dActualSleepTime << "] (Overshot: " << ((dActualSleepTime /  dExpectedSleepTime) * 100) << "%)" << endl;
    cout << "Tetrominoes Pending Deletion: " << deletionQueue.size() << " * " << sizeof(Tetromino) << " bytes" << endl;

    auto buf = getBoardBuffer();
    std::cout << "<" << std::string(20, '=') << ">" << std::endl;
    for (size_t y = 40 - 22; y < buf[0].size(); ++y) {
        std::cout << "|";

        // Loop over each column (x-coordinate)
        for (size_t x = 0; x < buf.size(); ++x) {
            int mino = buf[x][y];
            if (mino != 0) {
                std::cout << (mino > 0 ? "[]" : (mino == GHOST_PIECE_CONVENTION ? "::" : "[]"));
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "<" << std::string(20, '=') << ">" << std::endl;
}

#undef LONG
#endif //TETRIS_ENGINE_CPP
