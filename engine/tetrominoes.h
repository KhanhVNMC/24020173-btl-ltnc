#ifndef TETROMINOES_H
#define TETROMINOES_H
#include <vector>
#include <stdexcept>

using namespace std;

namespace RenderMatrixMino {
    const vector<vector<int> > T_PIECE = {
            { 0, 1, 0, 0 },
            { 1, 1, 1, 0 }
    };
    const vector<vector<int> > Z_PIECE = {
            { 1, 1, 0, 0 },
            { 0, 1, 1, 0 }
    };
    const vector<vector<int> > S_PIECE = {
            { 0, 1, 1, 0 },
            { 1, 1, 0, 0 }
    };
    const vector<vector<int> > L_PIECE = {
            { 0, 0, 1, 0 },
            { 1, 1, 1, 0 }
    };
    const vector<vector<int> > J_PIECE = {
            { 1, 0, 0, 0 },
            { 1, 1, 1, 0 }
    };
    const vector<vector<int> > I_PIECE = {
            { 0, 0, 0, 0 },
            { 1, 1, 1, 1 }
    };
    const vector<vector<int> > O_PIECE = {
            { 0, 1, 1, 0 },
            { 0, 1, 1, 0 }
    };
}

class MinoTypeEnum {
    vector<vector<vector<int>>> rotations;

    /**
     * The amount of individual minoes in a Mino
     */
public:
    int blockCount = -1;

    /**
     * Replicate of Java's enum
     */
    int ordinal = -1;

    /**
     * The 2x4 version of the mino (in Java it's LegacyMino)
     */
    vector<vector<int>> renderMatrix;

    /**
     * Replicate of Java's name()
     */
    string name_;
    string& name() {
        return name_;
    }

    /**
     * Construct an enum for MinoType
     * @param minoShape
     * @param legacyStruct
     * @param ordinal
     */
    MinoTypeEnum(const string enumName, const vector<vector<int>> &minoShape, const vector<vector<int> > &legacyStruct, int ordinal) {
        if (minoShape.size() != minoShape[0].size()) {
            throw invalid_argument("Shape is not square");
        }
        this->name_ = enumName;
        this->blockCount = 0;

        // count the minoes
        for (size_t i = 0; i < minoShape.size(); ++i) {
            for (size_t j = 0; j < minoShape.size(); ++j) {
                if (minoShape[i][j] != 0) {
                    this->blockCount++;
                }
            }
        }

        // assign legacy shapes
        this->renderMatrix = legacyStruct;

        auto shape = minoShape;
        // default rotation state 0
        this->rotations.push_back(shape);

        //precomputes rotations
        for (int i = 0; i < 3; ++i) {
            shape = rotateClockwise(shape);
            this->rotations.push_back(shape);
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
    public: [[nodiscard]] const vector<vector<int> > &getStruct(const int rotation) const {
        return rotations[rotation];
    }

    /**
     * Rotate a matrix by 90 degrees clockwise
     *
     * @param a the matrix
     * @return the rotated matrix
    */
    private: static vector<vector<int>> rotateClockwise(const vector<vector<int>> &a) {
        const size_t m = a.size();
        vector rotated(m, vector<int>(m, 0));
        for (size_t x = 0; x < m; ++x) {
            for (size_t y = 0; y < m; ++y) {
                rotated[y][m - 1 - x] = a[x][y];
            }
        }
        return rotated;
    }
};

// MinoType.h
namespace MinoType {
    extern MinoTypeEnum T_MINO;
    extern MinoTypeEnum Z_MINO;
    extern MinoTypeEnum S_MINO;
    extern MinoTypeEnum L_MINO;
    extern MinoTypeEnum J_MINO;
    extern MinoTypeEnum I_MINO;
    extern MinoTypeEnum O_MINO;
    inline constexpr int valuesLength = 7;
}

#endif //TETROMINOES_H
