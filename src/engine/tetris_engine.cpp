//
// Created by GiaKhanhVN on 4/9/2025.
//
#include "tetris_engine.h"

/************** BEGIN OF MEMORY MANAGEMENT BULLSHIT ***************/
void TetrisEngine::markFallingPieceAsNull() {
    this->deletionQueue.push_back(this->fallingPiece);
    this->fallingPiece = nullptr;
}

void TetrisEngine::freeMemoryOfFallingPiece() {
    for (Tetromino* piece: deletionQueue) {
        // actually deleting the shit
        delete piece;
    }
    deletionQueue.clear();
}
/****** END OF BULLSHIT (not really, the entire code is bs) ***********/

void TetrisEngine::stop() {
    if (stopped) throw logic_error("Already stopped!");
    this->stopped = true;

    if (this->fallingPiece != nullptr) {
        this->markFallingPieceAsNull();
    }
    // because stop essentially kills the entire engine,
    // there's no point waiting when to delete anymore
    this->freeMemoryOfFallingPiece();
}

void TetrisEngine::moveLeft() {
    if (this->fallingPiece != nullptr) fallingPiece->translateHorizontally(true);
}

void TetrisEngine::moveRight() {
    if (this->fallingPiece != nullptr) fallingPiece->translateHorizontally(false);
}

void TetrisEngine::rotateCW() {
    if (this->fallingPiece != nullptr) fallingPiece->rotate(false);
}

void TetrisEngine::rotateCCW() {
    if (this->fallingPiece != nullptr) fallingPiece->rotate(true);
}

void TetrisEngine::softDropToggle(const bool on) {
    this->gravity = this->defaultGravity;
    if (on) this->gravity *= softDropFactor;
}

void TetrisEngine::hardDrop() {
    if (this->fallingPiece != nullptr) fallingPiece->hardDrop();
}

void TetrisEngine::hold() {
    if (this->fallingPiece != nullptr) this->holdButtonPressed = true;
    // Signals the main game loop to execute onUserHold()
}

MinoTypeEnum* TetrisEngine::getFallingMinoType() {
    if (fallingPiece != nullptr) return fallingPiece->type;
    return nullptr;
}

void TetrisEngine::raiseGarbage(int height, int holeIndex) {
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

const vector<vector<int> > &TetrisEngine::getBoardBuffer() const {
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
void TetrisEngine::onEngineStart() {
    this->pushNextPieceToPlayfield();
}

// this will run every single tick
void TetrisEngine::onTickRun() {
    // handle gravity
    this->moveCellOnGameGravity();
}

// on mino placed
void TetrisEngine::onMinoLocked(Tetromino *locked) {
    // allow user to hold again
    this->canHold = true;
    // new mino
    this->updatePlayfieldState(locked);
    // Place the next piece in playfield, this also generates
    // a new piece at the end of the queue
    this->pushNextPieceToPlayfield();
}

// on user hold
void TetrisEngine::onUserHold() {
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
        this->markFallingPieceAsNull();
        this->putPieceInPlayfield(this->holdPiece);
    } else {
        this->pushNextPieceToPlayfield();
    }

    // update holdPiece with the current falling piece and disable holding until the next piece is placed.
    this->holdPiece = toHold;
    this->canHold = false; // disable further holding until the next piece is placed
    // only in SDL
    SysAudio::playSoundAsync(PIECE_HOLD_AUD, SysAudio::getSFXVolume(), false);
}

// called when a piece is manipulated (moved, rotated by the player)
void TetrisEngine::onPieceManipulation() {
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
void TetrisEngine::moveCellOnGameGravity() {
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

void TetrisEngine::updatePlayfieldState(Tetromino* locked) {
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
        this->clearDelayActive = true; // activate clear delay, halting piece spawning temporarily (this should be changed to interrupt, but fuck it)
        if (lineClearsDelay > 0) {
            // schedule playfield update to clear lines after delay (if > 0)
            scheduleDelayedTask(lineClearsDelay, [this, clearedLines] {
                this->updatePlayFieldLineClears(clearedLines);
            });
        } else updatePlayFieldLineClears(clearedLines); // run instantly if 0
    }
}

// internal function
void TetrisEngine::updatePlayFieldLineClears(const vector<int> &clearedLines) {
    for (const int row: clearedLines) {
        clearRow(row);
    }
    this->clearDelayActive = false;
}

void TetrisEngine::pushNextPieceToPlayfield() {
    // append a new piece from the generator to the end
    // of the next queue
    this->appendNextQueue();
    // instead of pushing by itself, it will set it to null to signal
    // the main game loop to spawn the piece
    this->markFallingPieceAsNull();
}

void TetrisEngine::putPieceInPlayfield(MinoTypeEnum* type) {
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
    // this is the only method that can both spawn and clear at the same time
    if (!this->fallingPiece->canFitBeingAt(this->fallingPiece->x, this->fallingPiece->y)) {
        this->markFallingPieceAsNull();
        this->shouldTopOut = true; // this will signal the game loop to execute onTopOut
    }
}

void TetrisEngine::gameLoopStart(bool useCurrentThread) {
    if (this->stopped) throw logic_error("This instance has stopped! You must create a new instance!");
    if (this->started) throw logic_error("This instance is already started");

    // fire pre-start events
    this->onEngineStart();
    this->startedAt = System::currentTimeMillis();

    // the flag to look for
    this->started = true;

    if (useCurrentThread) {
        for (;;) {
            // handle the game loop, if stop signal, break
            if (!this->gameLoopBody()) break;
        }
    }
}

bool TetrisEngine::gameLoopBody() {
    // stop on break signal
    if (this->stopped) return false;
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
    // only if the clear delay period is not active and NOT interrupted
    if (this->fallingPiece == nullptr && !clearDelayActive && !interrupted) {
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
    return true;
}

void TetrisEngine::printBoard() const { /* deprecated */ }
#undef LONG