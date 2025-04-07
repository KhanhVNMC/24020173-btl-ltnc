//
// Created by GiaKhanhVN on 2/19/2025.
//

#ifndef TETISENGINE_PLAYFIELD_EVENT_H
#define TETISENGINE_PLAYFIELD_EVENT_H
#include <vector>
#include "tetrominoes.h"

/**
 * Represents an event that occurs on the playfield during a game.
 */
class PlayfieldEvent {
private:
    std::vector<int> linesCleared_;
    bool isPerfectClear_;
    MinoTypeEnum* lastMino_;
    bool isSpin_;
    bool isMiniSpin_;

public:
    /**
     * Constructs a PlayfieldEvent object.
     *
     * @param linesCleared   The indices of lines cleared in this event.
     * @param isPerfectClear Whether the field was perfectly cleared.
     * @param lastMino       The last tetromino used before the event.
     * @param isSpin         Whether a spin move was performed.
     * @param isMiniSpin     Whether a mini-spin move was performed.
     */
    PlayfieldEvent(const std::vector<int>& linesCleared, bool isPerfectClear, MinoTypeEnum* lastMino, bool isSpin, bool isMiniSpin)
            : linesCleared_(linesCleared), isPerfectClear_(isPerfectClear), lastMino_(lastMino), isSpin_(isSpin), isMiniSpin_(isMiniSpin) {}

    /**
     * @return A constant reference to a vector containing the indices of the lines cleared in this event.
     * @apiNote This event is triggered after the lines have been nullified but before gravity has been applied.
     *          As a result, the line indices may all be set to 0 or another value, depending on the
     *          implementation of the engine's internal nullifyRow(int rowIndex) function.
     */
    const std::vector<int>& getLinesCleared() const {
        return linesCleared_;
    }

    /**
     * @return Whether the field was perfectly cleared.
     */
    bool isPerfectClear() const {
        return isPerfectClear_;
    }

    /**
     * @return The last tetromino used before the event.
     */
    MinoTypeEnum* getLastMino() const {
        return lastMino_;
    }

    /**
     * @return Whether a spin move was performed.
     */
    bool isSpin() const {
        return isSpin_;
    }

    /**
     * @return Whether a mini-spin move was performed.
     */
    bool isMiniSpin() const {
        return isMiniSpin_;
    }
};


#endif //TETISENGINE_PLAYFIELD_EVENT_H
