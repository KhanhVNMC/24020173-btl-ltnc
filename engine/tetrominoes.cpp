//
// Created by GiaKhanhVN on 4/8/2025.
//
#include "tetrominoes.h"

namespace MinoType {
    MinoTypeEnum T_MINO("T_MINO", {{0, 1, 0}, {1, 1, 1}, {0, 0, 0}}, RenderMatrixMino::T_PIECE, 0);
    MinoTypeEnum Z_MINO("Z_MINO", {{1, 1, 0}, {0, 1, 1}, {0, 0, 0}}, RenderMatrixMino::Z_PIECE, 1);
    MinoTypeEnum S_MINO("S_MINO", {{0, 1, 1}, {1, 1, 0}, {0, 0, 0}}, RenderMatrixMino::S_PIECE, 2);
    MinoTypeEnum L_MINO("L_MINO", {{0, 0, 1}, {1, 1, 1}, {0, 0, 0}}, RenderMatrixMino::L_PIECE, 3);
    MinoTypeEnum J_MINO("J_MINO", {{1, 0, 0}, {1, 1, 1}, {0, 0, 0}}, RenderMatrixMino::J_PIECE, 4);
    MinoTypeEnum I_MINO("I_MINO", {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, RenderMatrixMino::I_PIECE, 5);
    MinoTypeEnum O_MINO("O_MINO", {{1, 1}, {1, 1}}, RenderMatrixMino::O_PIECE, 6);
}