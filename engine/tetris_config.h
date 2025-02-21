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
    // Chainable setters
    TetrisConfig& setGhostPieceEnabled(bool enabled) {
        ghostPieceEnabled = enabled;
        return *this;
    }

    TetrisConfig& setSRSEnabled(bool enabled) {
        srsEnabled = enabled;
        return *this;
    }

    TetrisConfig& setPieceMovementThreshold(int threshold) {
        pieceMovementThreshold = threshold;
        return *this;
    }

    TetrisConfig& setHoldEnabled(bool enabled) {
        holdEnabled = enabled;
        return *this;
    }

    TetrisConfig& setSecondsBeforePieceLock(double seconds) {
        secondsBeforePieceLock = seconds;
        return *this;
    }

    TetrisConfig& setLineClearsDelay(double seconds) {
        lineClearsDelaySecond = seconds;
        return *this;
    }

    TetrisConfig& setGravity(double g) {
        gravity = g;
        return *this;
    }

    TetrisConfig& setSoftDropFactor(double factor) {
        softDropFactor = factor;
        return *this;
    }

    // Builder method to create a new instance with default values
    /**
     * @warning This will allocate this object on the HEAP!
     * @return builder for Tetris Engine
     */
    static TetrisConfig* builder() {
        return new TetrisConfig();
    }
};

#endif //TETISENGINE_TETRIS_CONFIG_H
