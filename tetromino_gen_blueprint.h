//
// Created by GiaKhanhVN on 2/20/2025.
//

#ifndef TETISENGINE_TETROMINO_GEN_BLUEPRINT_H
#define TETISENGINE_TETROMINO_GEN_BLUEPRINT_H

#pragma once
#include <vector>
#include "tetrominoes.h"

// Assumes MinoType is defined elsewhere.

/**
 * Represents a generator for Tetrominoes.
 */
class TetrominoGenerator {
public:
    /**
     * Grab the entire bag of Tetrominoes.
     * @return a vector of Tetrominoes.
     */
    virtual std::vector<MinoTypeEnum*> grabTheEntireBag() const = 0;

    /**
     * Get the next Tetromino in the bag.
     * @return the next Tetromino.
     */
    virtual MinoTypeEnum* next() const = 0;

    virtual ~TetrominoGenerator() = default;
};


#endif //TETISENGINE_TETROMINO_GEN_BLUEPRINT_H
