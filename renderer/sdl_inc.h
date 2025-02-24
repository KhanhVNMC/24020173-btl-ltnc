//
// Created by GiaKhanhVN on 2/23/2025.
//

#ifndef SDL_INC_H
#define SDL_INC_H
#include "../engine/tetris_engine.cpp"
#include "disk_cache.h"

#define TETRIS_BOARD "../assets/tetris_board.bmp"
#define TETROMINOES "../assets/tetrominoes.bmp"

// because the internal Enums' ordinal and the sprite.bmp uses different indexes, we map INTERNAL -> BMP
static unordered_map<int, int> TEXTURE_MAPPER = {
    { MinoType::Z_MINO.ordinal, 0 },
    { MinoType::L_MINO.ordinal, 1 },
    { MinoType::O_MINO.ordinal, 2 },
    { MinoType::S_MINO.ordinal, 3 },
    { MinoType::I_MINO.ordinal, 4 },
    { MinoType::J_MINO.ordinal, 5 },
    { MinoType::T_MINO.ordinal, 6 },
    { MinoType::valuesLength + 1, 8} // garbage mino
};

// component 1
typedef struct {
    SDL_Rect source;
    SDL_Rect dest;
} struct_render_component;

/**
 * Render the given "struct_render_component"
 * @param renderer the target SDL renderer
 * @param texture the texture (get from CACHE, BMP)
 * @param component the struct_render_component
 * @param opacity opacity from 0.0 - 1.0 (0: transparent; 1: opaque)
 */
void render_component(SDL_Renderer* renderer, SDL_Texture* texture, const struct_render_component& component, const float opacity) {
    SDL_SetTextureAlphaMod(texture, opacity * 255);
    SDL_RenderCopy(renderer, texture, &component.source, &component.dest);
}

void render_component_tetromino(SDL_Renderer* renderer, const struct_render_component& mino, const float opacity) {
    render_component(renderer, disk_cache::bmp_load_and_cache(renderer, TETROMINOES), mino, opacity);
}

// uh, problem?
constexpr int MINO_SIZE = 30;
// the size of each mino box is 4 (2x4), we leave 2 minoes worth of gap for the HOLD rendering
constexpr int PLAYFIELD_RENDER_OFFSET = MINO_SIZE * 5;
// the NEXT queue is rendered at the end of the playfield PLUS 2 minoes worth of gap
constexpr int NEXT_RENDER_OFFSET = PLAYFIELD_RENDER_OFFSET + (MINO_SIZE * 10) + (MINO_SIZE * 1);
// the NEXT queue and HOLD indicator is shifted 4 minoes down
constexpr int Y_OFFSET = MINO_SIZE * 4;

struct_render_component render_mino_at(const int offsetX, const int offsetY, const int bx, const int by, const int color) {
    return {
            {color * (30 + 1), 0, 30, 30}, // because the tetromino sprite is 30x30 with 1 pixel gap
            // what the fuck, magic numbers
            {offsetX + (MINO_SIZE * bx), offsetY + (MINO_SIZE * by), MINO_SIZE, MINO_SIZE}
    };
}

// properties
#define BOARD_HEIGHT 22
#define BOARD_WIDTH 10
#define BORDER_WIDTH 3

// the board will turn red once the 17th row has a mino in it
#define DANGER_THRESHOLD 40 - 17

// render the entire fucking shit
void render_tetris_board(const int ox, const int oy, SDL_Renderer* renderer, TetrisEngine* engine) {
    // render the board
    // white border around the playfield
    const SDL_Rect borders[3] = {
            // left border
            { ox + PLAYFIELD_RENDER_OFFSET - BORDER_WIDTH, oy + (MINO_SIZE * 2), BORDER_WIDTH, MINO_SIZE * (BOARD_HEIGHT - 2) },
            // bottom border
            { ox + PLAYFIELD_RENDER_OFFSET - BORDER_WIDTH, oy + (MINO_SIZE * BOARD_HEIGHT), MINO_SIZE * BOARD_WIDTH + (2 * BORDER_WIDTH), BORDER_WIDTH },
            // right border
            { ox + PLAYFIELD_RENDER_OFFSET + (MINO_SIZE * BOARD_WIDTH), oy + (MINO_SIZE * 2), BORDER_WIDTH, MINO_SIZE * (BOARD_HEIGHT - 2) }
    };

    // render the NEXT and HOLD headers
    const SDL_Rect utilities[6] = {
            // HOLD HEADER
            {ox - (MINO_SIZE), oy + (Y_OFFSET / 2), (MINO_SIZE * 6), 40},
            // HOLD BORDERS
            {ox - (MINO_SIZE), oy + (Y_OFFSET / 2) + 40, BORDER_WIDTH, 100}, // LEFT
            {ox - (MINO_SIZE), oy + (Y_OFFSET / 2) + 140, (MINO_SIZE * 6), BORDER_WIDTH}, // BOTTOM

            // NEXT HEADER
            {ox + PLAYFIELD_RENDER_OFFSET + (MINO_SIZE * BOARD_WIDTH), oy + (Y_OFFSET / 2), (MINO_SIZE * 6), 40},
            // NEXT BORDERS
            {ox + PLAYFIELD_RENDER_OFFSET + (MINO_SIZE * BOARD_WIDTH) + (MINO_SIZE * 6) - BORDER_WIDTH, oy + (Y_OFFSET / 2) + 40, BORDER_WIDTH, 460}, // RIGHT
            {ox + PLAYFIELD_RENDER_OFFSET + (MINO_SIZE * BOARD_WIDTH), oy + (Y_OFFSET / 2) + 500, (MINO_SIZE * 6), BORDER_WIDTH} // BTM
    };

    // the board will pulse red once the 17th row has a mino in it
    if (!engine->isRowEmpty(DANGER_THRESHOLD)) {
        // pulsing red (based on tick rate)
        double pulseStrength = ((sin(engine->ticksPassed * 0.1) + 2) / 4) + 0.25; // what the fuck
        SDL_SetRenderDrawColor(renderer, 255 * pulseStrength, 0, 0, 255); // red
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white
    }
    SDL_RenderFillRects(renderer, borders, 3); // BOARD
    SDL_RenderFillRects(renderer, utilities, 6); // UTILS
    // reset the color to make sure the entire screen isn't white / red
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);

    render_component(renderer, disk_cache::bmp_load_and_cache(renderer, "../assets/hold_sam.bmp"), {
            {0, 0, 180, 40},
            {ox - (MINO_SIZE), oy + (Y_OFFSET / 2), 180, 40}
        }, 1);

    // render the HOLD piece
    MinoTypeEnum* heldPiece = engine->getHoldPiece();
    if (heldPiece != nullptr) {
        auto& renderMatrix = heldPiece->renderMatrix;
        for (int y = 0; y < renderMatrix.size(); ++y) {
            for (int x = 0; x < renderMatrix[0].size(); ++x) {
                if (renderMatrix[y][x] == 0) continue;
                // if the user cant hold, grayscale the mino
                int color = !engine->canUseHold() ? 8 : TEXTURE_MAPPER[engine->getHoldPiece()->ordinal];
                // we don't need to offset X because the HOLD slot is rendered first
                render_component_tetromino(renderer,render_mino_at(ox, oy + Y_OFFSET, x, y, color), 1);
            }
        }
    }

    // render the entire NEXT queue (5 pieces visible at once)
    // the X offset of the queue = holdPieceOffsetX + (board width = MINO_SIZE * 10) + padding
    int index = 0;
    std::queue<MinoTypeEnum *> nextQueue = engine->getNextQueue();
    while (!nextQueue.empty()) {
        MinoTypeEnum *piece = nextQueue.front();
        nextQueue.pop();
        if (index > 4) break; // only renders up to 5 pieces
        auto& renderMatrix = piece->renderMatrix;
        for (int y = 0; y < renderMatrix.size(); ++y) {
            for (int x = 0; x < renderMatrix[0].size(); ++x) {
                if (renderMatrix[y][x] == 0) continue;
                int color = TEXTURE_MAPPER[piece->ordinal];
                // we offset the NEXT by the playfield width + 2 minoes gap
                // for each tetromino, we move down by 3 minoes
                render_component_tetromino(renderer,render_mino_at(ox + NEXT_RENDER_OFFSET, oy + Y_OFFSET, x, y + (index * 3), color), 1);
            }
        }
        index++;
    }

    // render the playfield (22x10)
    auto& buffer = engine->getBoardBuffer();
    for (int y = 0; y < BOARD_HEIGHT; ++y) { // we render 22 rows and 10 columns, hiding 18 lines
        for (int x = 0; x < BOARD_WIDTH; ++x) {
            int rawBuffer = buffer[x][18 + y]; // hide the buffer zone (18 lines above actual playfield)
            // we do not render minoes at 0es (or we must?)
            if (rawBuffer == 0) {
                // render empty mino (garbage mino with 10% opacity /shrug/)
                if (y >= 2) { // only render the grid if the thing is lower than the buffer zone
                    render_component_tetromino(renderer,render_mino_at(ox + PLAYFIELD_RENDER_OFFSET, oy, x, y, 8), 0.1);
                }
                continue;
            }

            // if the raw buffer is a ghost piece, we will handle it accordingly (ghost pieces DO NOT have color data built in)
            bool ghostPiece = rawBuffer == GHOST_PIECE_CONVENTION;
            // if the thing is garbage mino (garbage mino has its own color code), we don't do |x| - 1
            // for other colors, we need to do |x| - 1, because 0 is "empty", so the ordinals start at 1
            int colorMinoes = rawBuffer == GARBAGE_MINO_CONVENTION ? rawBuffer : abs(rawBuffer) - 1;

            // the color that will be rendered
            // if ghost piece (as I mentioned above, there is no color data for us to get from the buffer, so we need to get it from the current falling piece)
            int finalColor = TEXTURE_MAPPER[ghostPiece ? engine->getFallingMinoType()->ordinal : colorMinoes];

            // render the mino (if ghost piece, render at 35% opacity)
            render_component_tetromino(renderer,render_mino_at(ox + PLAYFIELD_RENDER_OFFSET, oy, x, y, finalColor), ghostPiece ? 0.35 : 1);
        }
    }
}

#endif //SDL_INC_H
