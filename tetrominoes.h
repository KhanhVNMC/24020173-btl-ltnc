#ifndef TETROMINOES_H
#define TETROMINOES_H
#include <bits/stdc++.h>
using namespace std;

namespace LegacyMino {
    const vector<vector<int> > T_PIECE = {{0, 1, 0}, {1, 1, 1}, {0, 0, 0}};
    const vector<vector<int> > Z_PIECE = {{1, 1, 0}, {0, 1, 1}, {0, 0, 0}};
    const vector<vector<int> > S_PIECE = {{0, 1, 1}, {1, 1, 0}, {0, 0, 0}};
    const vector<vector<int> > L_PIECE = {{0, 0, 1}, {1, 1, 1}, {0, 0, 0}};
    const vector<vector<int> > J_PIECE = {{1, 0, 0}, {1, 1, 1}, {0, 0, 0}};
    const vector<vector<int> > I_PIECE = {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}};
    const vector<vector<int> > O_PIECE = {{1, 1}, {1, 1}};
}

class MinoTypeEnum {
    vector<vector<vector<int>>> rotations;
    vector<vector<int>> legacyShape;

    /**
     * The amount of individual minoes in a Mino
     */
public:
    int blockCount;

    /**
     * Replicate of Java's enum
     */
    int ordinal;

    /**
     * Construct an enum for MinoType
     * @param minoShape
     * @param legacyStruct
     * @param ordinal
     */
    MinoTypeEnum(const vector<vector<int>> &minoShape, const vector<vector<int> > &legacyStruct, int ordinal) {
        if (minoShape.size() != minoShape[0].size()) throw invalid_argument("Shape is not square");
        blockCount = 0;

        // count the minoes
        for (size_t i = 0; i < minoShape.size(); ++i) {
            for (size_t j = 0; j < minoShape.size(); ++j) {
                if (minoShape[i][j] != 0) {
                    blockCount++;
                }
            }
        }

        // assign legacy shapes
        legacyShape = legacyStruct;

        auto shape = minoShape;
        // default rotation state 0
        rotations.push_back(shape);

        //precomputes rotations
        for (int i = 0; i < 3; ++i) {
            shape = rotateClockwise(shape);
            rotations.push_back(shape);
        }

        // enum action baby
        this->ordinal = ordinal;
    }

    MinoTypeEnum() = default;

    /**
     * JAVA CODE!!
     * The 2D array representing this tetromino with given rotation
     * state
     *
     * @param rotation state of this tetromino
     * @return the 2d structure
     */
public:
    [[nodiscard]] const vector<vector<int>> &getStruct(const int rotation) const {
        return rotations[rotation];
    }

    /**
     * Rotate a matrix by 90 degrees clockwise
     *
     * @param a the matrix
     * @return the rotated matrix
     */
    static vector<vector<int>> rotateClockwise(const vector<vector<int>> &a) {
        const size_t m = a.size();
        vector<vector<int>> rotated(m, vector<int>(m, 0));
        for (size_t x = 0; x < m; ++x) {
            for (size_t y = 0; y < m; ++y) {
                rotated[y][m - 1 - x] = a[x][y];
            }
        }
        return rotated;
    }
};

namespace MinoType {
    static const MinoTypeEnum T_MINO({{0, 1, 0}, {1, 1, 1}, {0, 0, 0}}, LegacyMino::T_PIECE, 0);
    static const MinoTypeEnum Z_MINO({{1, 1, 0}, {0, 1, 1}, {0, 0, 0}}, LegacyMino::Z_PIECE, 1);
    static const MinoTypeEnum S_MINO({{0, 1, 1}, {1, 1, 0}, {0, 0, 0}}, LegacyMino::S_PIECE, 2);
    static const MinoTypeEnum L_MINO({{0, 0, 1}, {1, 1, 1}, {0, 0, 0}}, LegacyMino::L_PIECE, 3);
    static const MinoTypeEnum J_MINO({{1, 0, 0}, {1, 1, 1}, {0, 0, 0}}, LegacyMino::J_PIECE, 4);
    static const MinoTypeEnum I_MINO({{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}}, LegacyMino::I_PIECE, 5);
    static const MinoTypeEnum O_MINO({{1, 1}, {1, 1}}, LegacyMino::O_PIECE, 6);
}

#endif //TETROMINOES_H
