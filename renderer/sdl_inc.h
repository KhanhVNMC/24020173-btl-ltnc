//
// Created by GiaKhanhVN on 2/23/2025.
//

#ifndef SDL_INC_H
#define SDL_INC_H
#include "../engine/tetris_engine.cpp"
#include "disk_cache.h"
#define TETRIS_BOARD "assets/tetris_board.bmp"
#define TETROMINOES "assets/tetrominoes.bmp"

static int TETROMINOES_TO_SPRITES[] = {
    0, // empty
    MinoType::Z_MINO.ordinal,
    MinoType::L_MINO.ordinal,
    MinoType::O_MINO.ordinal,
    MinoType::S_MINO.ordinal,
    MinoType::I_MINO.ordinal,
    MinoType::J_MINO.ordinal,
    MinoType::T_MINO.ordinal,
    MinoType::valuesLength + 1 // garbage mino
};

typedef struct {
    SDL_Rect source;
    SDL_Rect dest;
} mino_render_component;

mino_render_component render_mino_at(const int offsetX, const int offsetY, const int bx, const int by, const int color) {
    const mino_render_component mino = {
        {color * 31, 0, 30, 30}, // because the tetromino sprite is 30x30 with 1 pixel gap
        {offsetX + 5 + (34 * bx), offsetY - 25 + (34 * by), 34, 34}
    };
    return mino;
}

void render_component(SDL_Renderer* renderer, const mino_render_component &mino) {
    SDL_Texture* tetrominoesSprites = disk_cache::bmp_load_and_cache(renderer, TETROMINOES);
    SDL_RenderCopy(renderer, tetrominoesSprites, &mino.source, &mino.dest);
}

void render_tetris_board(const int ox, const int oy, SDL_Renderer* renderer, TetrisEngine* engine) {
    SDL_Texture* texture = disk_cache::bmp_load_and_cache(renderer, TETRIS_BOARD);
    SDL_Texture* tetrominoesSprites = disk_cache::bmp_load_and_cache(renderer, TETROMINOES);

    const int holdPieceOffsetX = 60;

    // render the board
    const SDL_Rect boardDest = {0, 0, 350, 660};
    const SDL_Rect dstRect = {ox + holdPieceOffsetX, oy, 350, 660}; // Same size as srcRect

    SDL_RenderCopy(renderer, texture, &boardDest, &dstRect);
    auto& buffer = engine->getBoardBuffer();
    //    SDL_RenderCopy(renderer, tetrominoesSprites, &pieceSrc, &pieceDest);
    for (int y = 0; y < 20; y++) {
        for (int x = 0; x < 10; x++) {
            int color = abs(buffer[x][20 + y]);
            if (color == 0 || color == GHOST_PIECE_CONVENTION) continue;
            render_component(renderer, render_mino_at(ox + holdPieceOffsetX, oy, x, y, color));
        }
    }
}

#endif //SDL_INC_H
