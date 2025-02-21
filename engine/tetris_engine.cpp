/**
 * Tetris Engine: C++ Edition<br>
 * This is a one-to-one, faithful port of the original "Tetris Engine: Java Edition"
 * to C++ for university project(s)<br>
 *<br>
 * Below is the description straight from the original version:
 *<br>
 * Modern, Guideline-compliant Tetris Engine
 * @see https://tetris.wiki/Tetris_Guideline
 *<br>
 * @author GiaKhanhVN (24020173)
 * @porter GiaKhanhVN (24020173)
 * @portedFrom Java (Java 16SE)
 */
#ifndef TETRIS_ENGINE_CPP
#define TETRIS_ENGINE_CPP

// stdcpp includes
#include <iostream>
#include <functional>
#include <queue>
#include <chrono>
#include <cmath>
#include <thread>
#include <map>

// internal libraries
#include "tetrominoes.h"
#include "playfield_event.h"
#include "tetromino_gen_blueprint.h"

// engine constants
/**
 * @caution The framerate (or tick rate) is tied to MANY important aspects of the Engine (gravity, timeout, intervals, ...)
 * <br><b>DO NOT CHANGE THE FRAMERATE WITHOUT ADJUSTING OTHER VALUES!</b>
 */
static constexpr float TARGETTED_FRAME_RATE = 60.0F; // 60FPS (TGM standard)
static constexpr double FRAME_INTERVAL_MS = 1000.0 / TARGETTED_FRAME_RATE; // in milliseconds

// engine convention
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
static constexpr int GARBAGE_MINO_CONVENTION = 8; // TODO

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
    /** static config, cannot be changed within the context of the Engine **/
    bool showGhostPiece = true; // if ghost piece is displayed or not
    // how many actions can be done before the piece locks in
    int pieceMovementThreshold = 15;
    // if the SRS system should be used
    bool useSRS = true;
    // line clears delay, after a piece locks in
    // Delay duration for cleared lines to remain empty before the board updates.
    // This creates a visual effect of gravity, simulating a brief pause
    // before the remaining tetrominoes fall down.
    // The user CANNOT interact with the game during this period
    int lineClearsDelay = 0;

    /** dynamic config, CAN be changed within the context of the Engine **/
    double softDropFactor = 24; // TETR.IO replication, default is 24, max can be 1_000_000
    // gravity, in G (TGM based)
    double defaultGravity = 0.0156; // 0.0156 cells per frame
    // lock delay, 0.5s by default (half of target frame rate)
    const int lockDelay = 0.5 * 60;
    // hold toggle, different from the HOLD flag that the context uses (canHold)
    bool holdEnabled = true;
    /**** end of configurations ********/

    // playfield related stuff
    vector<vector<int> > playfield{10, vector<int>(40, 0)}; // 10x40 matrix, a tetromino should spawn on the 22nd row
    // the active piece (falling)
    Tetromino* fallingPiece = nullptr;
    // the tetrominoes generator, can be implemented using the given interface (JAVA EXCLUSIVE, IN C++, ITS VIRTUAL)
    TetrominoGenerator* pieceGenerator = nullptr;
    // if hold is available or not
    bool canHold = true;

    // external fields related to the gameplay core
    int lastSpinKickUsed = 0; // 0 is no kick, > 1 is kick
    vector<int> lastKickPositionUsed = {0, 0}; // 0 is no kick, > 1 is kick
    int comboCount = -1; // the combo amount (2+ consecutive line clears w/o break), the combo begins at 2 lines

    // pieces queue and hold piece
    MinoTypeEnum* holdPiece = nullptr; // the hold piece will be "spawned" again when recall
    queue<MinoTypeEnum*> nextQueue; // the next queue

    // actual gravity variable that will be used by the game loop, (NOT the one you SHOULD EVER FUCKING TOUCH!!!!!!!),
    // @see defaultGravity
    // IF YOU WANT TO SET IT YOU FUCKING NIGGER
    // this can be multiplied with the SDF to make soft drop
    double gravity = this->defaultGravity;

    // internal systems flags / values
    long framesPassed = 0;
    long lastFrameTime = 0;
    long startedAt = -1;
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
    function<void()> onFrameEndCallback = nullptr; // run at every frame end
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
    map<long, vector<function<void()>>> scheduledTasks;

    /**
     * Schedules a task to be executed after a certain number of frames.
     *
     * @param frames The delay in frames after which the task should be executed.
     * @param task   The task to be executed (must implement Runnable).
     * @return       The frame number at which the task is scheduled to be executed.
     * @throws IllegalArgumentException if the task is null.
     */
    long scheduleDelayedTask(const long frames, const function<void()> &task) {
        if (task == nullptr) throw invalid_argument("Task could not be null!");
        const long execOnFrame = framesPassed + frames;
        scheduledTasks[execOnFrame].push_back(task);
        return execOnFrame;
    };

    void cancelTask(const long frameExecuted) {
        scheduledTasks.erase(frameExecuted);
    }

    TetrisEngine(TetrominoGenerator* generator) {
        // initialize the piece generator with given seed
        this->pieceGenerator = generator;
        // push the entire first bag to the queue
        auto bag = this->pieceGenerator->grabTheEntireBag();

        // start the NEXT queue
        for (auto piece : bag) {
            nextQueue.push(piece);
        }
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
    int getComboCount() {
        return max(0, comboCount);
    }

    /**
     * If hold is available or not (also return false when DISABLED)
     * @return true if available
     */
    bool canUseHold() {
        return this->holdEnabled && this->canHold;
    }

    /**
	 * Get the hold piece type
	 * @return MinoTypeEnum of the current hold piece
	 */
    MinoTypeEnum* getHoldPiece() {
        return this->holdPiece;
    }

    /**
     * Get the NEXT queue
     * @return the current next queue
     */
    queue<MinoTypeEnum*> &getNextQueue() {
        return this->nextQueue;
    }

    /**
     * Get the falling tetromino type
     * @return the type of the falling tetromino
     */
    MinoTypeEnum* getFallingMinoType();

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

    /**
     * Check if there's a mino at given coordinates
     *
     * @param x x-position
     * @param y y-position
     * @return true if has a mino at that position, or, out of bounds
     */
    bool hasMinoAt(const int x, const int y) const {
        return (x < 0 || y < 0 || x >= playfield.size() || y >= this->playfield[0].size()) || (this->playfield[x][y] > 0);
    }

    /******************** INTERNAL IMPLEMENTATION OF THE TETRIS ENGINE ********************/
    // this will start after the start() method
    void onEngineStart();

    // this will run every single frame
    void onFrameRun();

    // on mino placed
    void onMinoLocked(Tetromino *locked);

    // on user hold
    void onUserHold();

    // ID of the scheduled task that locks a piece after it lands
    long pieceLockTaskId = -1;
    // counter to track how many manipulations (moves/rotations) have been made
    mutable int manipulationCount = 0;

    // called when a piece is manipulated (rotated, moved by the player)
    void onPieceManipulation();

    double cellMoved = 0.0;

    // this runs on each frame and simulates the effect of gravity on the piece
    void moveCellOnGameGravity();

    // row manipulation
    // nullify a row by setting all of its cells to empty (0)
    // this creates the "line-disappear" effect
    void nullifyRow(const int rowIndex) {
        for (auto &x: playfield) {
            x[rowIndex] = 0;
        }
    }

    // clear the row by shifting down all rows above it by one
    void clearRow(const int rowIndex) {
        for (int y = rowIndex; y > 0; --y) {
            for (auto &x: playfield) {
                x[y] = x[y - 1];
            }
        }
    }

    // true if the row is empty
    bool isRowEmpty(const int rowIndex) const {
        for (auto &x: playfield) {
            if (x[rowIndex] != 0) return false;
        }
        return true;
    }

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
        } while (this->nextQueue.size() < 7); // TODO, REPLACE CONST BY MinoType.values().length
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
    void putPieceInPlayfield(MinoTypeEnum* type);

    /**
	 * Start the game loop
	 */
    void gameLoopStart();

    /**
     * Start the Tetris Engine, beginning to accept user inputs
    */
    public: void start() {
        this->gameLoopStart();
    }

    void printBoard();
};

class Tetromino {
public:
    int x = 0, y = 0; // Current coordinates of the top-left corner of the Tetromino on the playfield grid.
    MinoTypeEnum* type = nullptr; // The type of Tetromino (e.g., T_MINO, Z_MINO, L_MINO) being represented.
    int rotationState = 0; // 0, R, 2, L represented as 0, 1, 2, 3
    int size; // The size of the Tetromino's bounding box (usually 3x3).

    bool locking = false; // if the piece is getting locked
    int lastActionDone = 0; // 0 = nothing, 1 = move left, 2 = move right, 3 = cw, 4 = ccw

    // link the parent
    TetrisEngine *parent;

    explicit Tetromino(TetrisEngine *parent, MinoTypeEnum* type) : type() {
        this->parent = parent;
        this->type = type;
        // the size of this tetromino bounding box, ranging from 2 (O piece) to 4 (I piece)
        this->size = static_cast<int>(type->getStruct(0).size());
        parent->manipulationCount = 0; // new piece, 0 manipulation
    }

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
    * @param x the x-coordinate on the board
    * @param y the y-coordinate on the board
    * @return true if the tetromino occupies the specified coordinates, false otherwise
    */
    [[nodiscard]] bool occupyAt(const int x, const int y) const {
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
        const auto& minoStruct = this->getStruct(); // the structure of this mino with the rotation state applied
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
            const int testingY = this->y - kick[1]; // the board is upside down, index wise

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
        for (vector<int> minoPosition: getRelativeMinoCoordinates()) {
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
        // invalidate the ghost piece cache, forcing a recalculation
        this->invalidateGhostPieceCache();
        return true;
    }

    /**
     * Translate down 1 cell
     * @return true if can go down, false if not
     */
    bool translateDown() {
        if (onGround()) return false;
        ++this->y;
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

inline void TetrisEngine::stop() {
    if (stopped) throw invalid_argument("Already stopped!");
    this->stopped = true;

    delete this->fallingPiece;
    this->fallingPiece = nullptr;
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

inline MinoTypeEnum* TetrisEngine::getFallingMinoType() {
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
    for (int y = 0; y < this->playfield[0].size() - height; --y) {
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

// this will start after the start() method
inline void TetrisEngine::onEngineStart() {
    this->pushNextPieceToPlayfield();
}

// this will run every single frame
inline void TetrisEngine::onFrameRun() {
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
    MinoTypeEnum* toHold = this->fallingPiece->type; // get & store the type of the falling piece

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

// called when a piece is manipulated (rotated, moved by the player)
inline void TetrisEngine::onPieceManipulation() {
    // if the player has not exceeded the allowed manipulation count
    // (rotating or moving the piece too much)
    if (manipulationCount < pieceMovementThreshold) {
        // cancel any scheduled task that would lock the piece in place
        cancelTask(this->pieceLockTaskId);
        // reset the task ID as no lock task is active anymore
        this->pieceLockTaskId = -1;
    }
        // if there is an active lock task and a falling piece exists
    else if (this->pieceLockTaskId != -1 && fallingPiece != nullptr) {
        // force the piece to perform a hard drop, locking it instantly
        fallingPiece->hardDrop();
    }
    // increment the manipulation counter since a move/rotation just occurred
    manipulationCount++;
}

// This runs on each frame and simulates the effect of gravity on the piece
inline void TetrisEngine::moveCellOnGameGravity() {
    // accumulate the movement caused by gravity in each frame
    cellMoved += gravity;

    // once cellMoved reaches or exceeds 1 (a full cell downward movement)
    if (cellMoved >= 1) {
        // move the piece down by the number of full cells accumulated
        for (int cm = 0; cm < round(cellMoved); cm++) {
            if (fallingPiece != nullptr) {
                // try to move the piece down by one cell
                // if the piece can't move down further (landed) and no lock task is active
                if (const bool landed = !fallingPiece->translateDown(); landed && this->pieceLockTaskId == -1) {
                    Tetromino* piece = this->fallingPiece; // store a reference to the current piece
                    piece->locking = true;
                    // schedule a delayed task to lock the piece after the lockDelay (default 30frames) time
                    this->pieceLockTaskId = scheduleDelayedTask(lockDelay, [piece, this] {
                        // after the delay, reset the task ID
                        this->pieceLockTaskId = -1;

                        // check if the piece is still the same and is still on the ground
                        if (piece == this->fallingPiece && this->fallingPiece->onGround()) {
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
    if (locked->type->ordinal == MinoType::T_MINO.ordinal && (locked->lastActionDone == CW_ROTATION || locked->lastActionDone == CCW_ROTATION)) {
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
                {c1, c2, /*back*/ c3, c4}, // 0
                {c2, c3, /*back*/ c1, c4},  // 1
                {c3, c4, /*back*/ c1, c2}, // 2
                {c4, c1, /*back*/ c2, c3} // 3
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
    if (locked->type->ordinal != MinoType::T_MINO.ordinal && (locked->lastActionDone == CW_ROTATION || locked->lastActionDone == CCW_ROTATION)) {
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
        if (comboCount > 0 && onComboCallback != nullptr) onComboCallback(comboCount);
    } else if (comboCount > 0) {
        if (onComboBreaksCallback != nullptr) onComboBreaksCallback(comboCount);
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

inline void TetrisEngine::pushNextPieceToPlayfield()  {
    // append a new piece from the generator to the end
    // of the next queue
    this->appendNextQueue();
    // instead of pushing by itself, it will set it to null to signal
    // the main game loop to spawn the piece
    delete this->fallingPiece;
    this->fallingPiece = nullptr;
}

inline void TetrisEngine::putPieceInPlayfield(MinoTypeEnum* type) {
    if (type == nullptr || stopped) return; // if stopped or topped out, return

    // create the dynamic tetromino instance
    this->fallingPiece = new Tetromino(this, type);
    // set the initial X, Y position
    this->fallingPiece->x = (type->ordinal == MinoType::O_MINO.ordinal) ? 4 : 3;
    this->fallingPiece->y = static_cast<int>(playfield[0].size()) - 22; // the piece will always spawn on the 22nd row of the board
    // reset this measurement
    this->cellMoved = 0;

    // check if the user topped out
    if (!this->fallingPiece->canFitBeingAt(this->fallingPiece->x, this->fallingPiece->y)) {
        delete this->fallingPiece; // halt every user interaction
        this->fallingPiece = nullptr;
        this->shouldTopOut = true; // this will signal the game loop to execute onTopOut
    }
}

inline void TetrisEngine::gameLoopStart() {
    if (this->started) throw invalid_argument("This instance is already started");

    // fire pre-start events
    this->onEngineStart();
    this->startedAt = static_cast<long>(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());

    // the flag to look for
    this->started = true;

    for (;;) {
        // stop on break signal
        if (this->stopped) break;

        auto frameTimeBegin = chrono::high_resolution_clock::now();

        // if the falling piece is null, spawns a new one
        // only if the clear delay period is not active
        if (this->fallingPiece == nullptr && !clearDelayActive) {
            if (!nextQueue.empty()) {
                MinoTypeEnum* nextMino = nextQueue.front();
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
        onFrameRun();

        // run the external call
        if (this->onFrameEndCallback != nullptr) {
            try {
                this->onFrameEndCallback();
            } catch (exception& e) {
                cerr << e.what() << endl;
            }
        }

        // increment frame counter, used for scheduling
        framesPassed++;

        // scheduled task handling (this is more primitive than Java because of c++ libs)
        // get this frame's scheduled tasks
        auto tasks = scheduledTasks[framesPassed];
        scheduledTasks.erase(framesPassed);
        if (!tasks.empty()) {
            // execute all tasks assigned to this frame
            for (auto& task: tasks) {
                task();
            }
        }

        // lost-frames compensation mechanism
        auto frameTimeEnd = chrono::high_resolution_clock::now();
        this->lastFrameTime = static_cast<long>(chrono::duration_cast<chrono::milliseconds>(frameTimeEnd - frameTimeBegin).count());
        double parkPeriod = max(0.0, FRAME_INTERVAL_MS - lastFrameTime);

        // if top out, execute the onTopOut external call
        // Usually, onTopOut will be assigned to TetrisEngine#stop(), which will
        // stop the main game loop, thus trigger the `if (this.stopped) break;` above
        if (shouldTopOut) {
            if (this->onTopOutCallback != nullptr) this->onTopOutCallback(); // execute one last time
            this->shouldTopOut = false; // the user may be creative and do something else with this
        }

        // it could be 0, which means: No sleep, execute immediately
        if (parkPeriod > 0) {
            this_thread::sleep_for(chrono::milliseconds(static_cast<long>(parkPeriod)));
        } // frame cap
    }
}

inline void TetrisEngine::printBoard() {
    // Copy pointer to the falling piece.
    Tetromino* fp = fallingPiece;
    bool hasFallingPiece = (fp != nullptr);

    // Print top border
    std::cout << "<" << std::string(30, '=') << ">" << std::endl;

    // Loop over each row (y-coordinate)
    for (size_t y = 0; y < playfield[0].size(); ++y) {
        std::cout << "|";

        // Loop over each column (x-coordinate)
        for (size_t x = 0; x < playfield.size(); ++x) {
            bool has = false;

            // Check if the falling piece overlaps with the current position.
            if (hasFallingPiece) {
                has = fp->occupyAt(x, y);
            }

            // If playfield[x][y] > 0 or the falling piece occupies the cell,
            // print "[#]" if the falling piece is present; otherwise print "[ ]".
            // If neither, print " - ".
            if (playfield[x][y] > 0 || has) {
                std::cout << (has ? "[#]" : "[ ]");
            } else {
                std::cout << " - ";
            }
        }
        std::cout << "|" << std::endl;
    }

    // Print bottom border
    std::cout << "<" << std::string(30, '=') << ">" << std::endl;
}

#endif //TETRIS_ENGINE_CPP
