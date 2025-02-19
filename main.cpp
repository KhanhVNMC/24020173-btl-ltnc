#include "tetris_engine.h"
#include <bits/stdc++.h>

using namespace std;

// SevenBagGenerator implementation.
class SevenBagGenerator : public TetrominoGenerator {
public:
    // Nested RNG class modeled after TetrioRNG.
    class TetrioRNG {
    private:
        long t;
    public:
        explicit TetrioRNG(long seed) : t(seed % 2147483647) {
            if (t <= 0) {
                t += 2147483646;
            }
        }

        long next() {
            t = (16807 * t) % 2147483647;
            return t;
        }

        float nextFloat() {
            // Returns a float in [0, 1)
            return static_cast<float>(next() - 1) / 2147483646.0f;
        }

        // Shuffles a vector in place.
        template <typename T>
        void shuffleList(std::vector<T>& list) {
            if (list.empty()) return;
            for (int i = static_cast<int>(list.size()) - 1; i > 0; --i) {
                int r = static_cast<int>(nextFloat() * (i + 1));
                std::swap(list[i], list[r]);
            }
        }
    };

private:
    // The bag is mutable so that it can be modified in a const method.
    mutable std::vector<const MinoTypeEnum*> bag;
    mutable TetrioRNG random;

    /**
     * Refills the bag with the seven tetromino types and shuffles it.
     * Marked const since it may be called from const methods.
     */
    void refillBag() const {
        bag.clear();
        bag.push_back(&MinoType::Z_MINO);
        bag.push_back(&MinoType::L_MINO);
        bag.push_back(&MinoType::O_MINO);
        bag.push_back(&MinoType::S_MINO);
        bag.push_back(&MinoType::I_MINO);
        bag.push_back(&MinoType::J_MINO);
        bag.push_back(&MinoType::T_MINO);

        random.shuffleList(bag);
    }

public:
    /**
     * Constructor.
     * @param seed Seed for the RNG.
     */
    explicit SevenBagGenerator(long seed) : random(seed) {
        refillBag();
    }

    /**
     * Grab the entire bag of Tetrominoes.
     * If the bag is empty, it is refilled.
     * The bag is then copied and cleared.
     */
    std::vector<const MinoTypeEnum*> grabTheEntireBag() const override {
        if (bag.empty()) {
            refillBag();
        }
        std::vector<const MinoTypeEnum*> currentBag = bag;
        bag.clear();
        return currentBag;
    }

    /**
     * Get the next Tetromino in the bag.
     * If the bag is empty, it is refilled.
     * The first element is removed and returned.
     */
    const MinoTypeEnum* next() const override {
        if (bag.empty()) {
            refillBag();
        }
        const MinoTypeEnum* nextMino = bag.front();
        bag.erase(bag.begin());
        return nextMino;
    }
};

TetrisEngine* e;

int main() {
    SevenBagGenerator bag(123);
    TetrisEngine engine(&bag);

    e = &engine;

    e->gravity = 1;

    e->onFrameEndCallback = []{
        e->printBoard();
    };

    e->start();
}