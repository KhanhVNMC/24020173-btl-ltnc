//
// Created by GiaKhanhVN on 2/21/2025.
//

#ifndef TETISENGINE_TETRIS_CONFIG_H
#define TETISENGINE_TETRIS_CONFIG_H
#pragma once

class TetrisConfig {
public:
    bool holdEnabled = true;
    bool ghostPieceEnabled = true;
    bool srsEnabled = true;
    double secondsBeforePieceLock = 0.5;
    double lineClearsDelaySecond = 0.0;
    int pieceMovementThreshold = 15;
    double gravity = 0.0156;
    double softDropFactor = 24.0;

public:
    /**
     * Sets whether the ghost piece feature is enabled.
     *
     * @apiNote This field cannot be changed after the constructor of Tetris Engine
     * @defaultValue true
     *
     * @param ghostPieceEnabled true to enable the ghost piece feature; false to
     *                          disable it.
     */
    TetrisConfig& setGhostPieceEnabled(bool enabled) {
        ghostPieceEnabled = enabled;
        return *this;
    }
    /**
	 * Sets whether the Super Rotation System (SRS) is enabled.
	 *
	 * @apiNote This field cannot be changed after the constructor of Tetris Engine
	 * @defaultValue true
	 *
	 * @param srsEnabled true to enable SRS; false to disable it.
	 */
    TetrisConfig& setSRSEnabled(bool enabled) {
        srsEnabled = enabled;
        return *this;
    }

    /**
	 * Sets the piece movement threshold (how many user inputs is accepted during the "locking" phase
	 * before the piece permanently locks in place)
	 *
	 * @apiNote This field cannot be changed after the constructor of Tetris Engine
	 * @defaultValue 15
	 *
	 * @param pieceMovementThreshold the threshold value to set.
	 */
    TetrisConfig& setPieceMovementThreshold(int threshold) {
        pieceMovementThreshold = threshold;
        return *this;
    }

    /**
     * Sets whether the hold feature is enabled.
     *
     * @defaultValue true
     *
     * @param holdEnabled true to enable the hold feature; false to disable it.
     */
    TetrisConfig& setHoldEnabled(bool enabled) {
        holdEnabled = enabled;
        return *this;

    }

    /**
     * Sets the number of seconds before a piece locks into place.
     *
     * @defaultValue 0.5d
     *
     * @param secondsBeforePieceLock the number of seconds to set.
     */
    TetrisConfig& setSecondsBeforePieceLock(double seconds) {
        secondsBeforePieceLock = seconds;
        return *this;
    }

    /**
     * Sets the delay in seconds before cleared lines are updated in the playfield.
     * This delay allows the cleared lines to remain empty for a brief moment,
     * creating a visual effect of gravity as the remaining pieces fall into place.
     *
     * @defaultValue 0d
     *
     * @param lineClearsDelaySecond the delay duration in seconds to set.
     * @apiNote This field cannot be changed after the constructor of Tetris Engine.
     */
    TetrisConfig& setLineClearsDelay(double seconds) {
        lineClearsDelaySecond = seconds;
        return *this;
    }

    /**
     * Sets the gravity, in G, affecting piece fall speed (cells per tick; aka frames)
     * {@link https://harddrop.com/wiki/Drop#Gravity}
     *
     * @defaultValue 0.156
     *
     * @param gravity the gravity amount to set.
     */
    TetrisConfig& setGravity(double g) {
        gravity = g;
        return *this;
    }

    /**
     * Sets the scalar for soft drop factor (SDF)
     *
     * @defaultValue 24
     *
     * @param softDropFactor the soft drop factor to set.
     */
    TetrisConfig& setSoftDropFactor(double factor) {
        softDropFactor = factor;
        return *this;
    }

    /**
    * Creates a new instance of TetrisConfig using default config (Modern-Guideline Tetris)
    * @warning This will allocate this object on the HEAP!
    * @return a new TetrisConfig instance.
    */
    static TetrisConfig* builder() {
        return new TetrisConfig();
    }
};

#endif //TETISENGINE_TETRIS_CONFIG_H
