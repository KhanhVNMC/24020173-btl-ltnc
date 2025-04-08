//
// Created by GiaKhanhVN on 2/23/2025.
//

#ifndef TETISENGINE_BAG_GENERATOR_H
#define TETISENGINE_BAG_GENERATOR_H

#include <cmath>
#include "../engine/tetromino_gen_blueprint.h"

class SevenBagGenerator : public TetrominoGenerator {
public:
    class TetrioRNG {
    private:
        mutable long t;
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
            float result = static_cast<float>(next() - 1) / 2147483646.0f;
            // replicates java
            // result is strictly < 1.0 (c++ is so fucking bruh)
            if (result >= 1.0f) {
                result = std::nextafter(1.0f, 0.0f);
            }
            return result;
        }

        template <typename T>
        std::vector<T>& shuffleList(std::vector<T>& list) {
            if (list.empty()) return list;
            for (std::size_t i = list.size() - 1; i > 0; --i) {
                std::size_t r = static_cast<std::size_t>(nextFloat() * (i + 1));
                if (r > i) r = i; // safety net, prevent c++ float from being a fucking idiot
                std::swap(list[i], list[r]);
            }

            return list;
        }
    };

private:
    // The bag is mutable so that it can be modified in a const method.
    mutable std::vector<MinoTypeEnum*> bag;
    mutable TetrioRNG random;

    /**
     * Refills the bag with the seven tetromino types and shuffles it.
     * Marked const since it may be called from const methods.
     */
    void refillBag() {
        bag.clear();
        bag.push_back(&MinoType::Z_MINO);
        bag.push_back(&MinoType::L_MINO);
        bag.push_back(&MinoType::O_MINO);
        bag.push_back(&MinoType::S_MINO);
        bag.push_back(&MinoType::I_MINO);
        bag.push_back(&MinoType::J_MINO);
        bag.push_back(&MinoType::T_MINO);

        srand(System::currentTimeMillis());
        random.shuffleList(this->bag);
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
    std::vector<MinoTypeEnum*> grabTheEntireBag() override {
        if (bag.empty()) {
            refillBag();
        }
        std::vector<MinoTypeEnum*> currentBag = bag;
        bag.clear();
        return currentBag;
    }

    /**
     * Get the next Tetromino in the bag.
     * If the bag is empty, it is refilled.
     * The first element is removed and returned.
     */
    MinoTypeEnum* next() override {
        if (bag.empty()) {
            refillBag();
        }
        MinoTypeEnum* nextMino = bag.front();
        bag.erase(bag.begin());
        return nextMino;
    }
};

#endif //TETISENGINE_BAG_GENERATOR_H
