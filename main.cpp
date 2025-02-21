#include <windows.h>

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
    mutable std::vector<MinoTypeEnum*> bag;
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
    std::vector<MinoTypeEnum*> grabTheEntireBag() const override {
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
    MinoTypeEnum* next() const override {
        if (bag.empty()) {
            refillBag();
        }
        MinoTypeEnum* nextMino = bag.front();
        bag.erase(bag.begin());
        return nextMino;
    }
};

void randomAction(TetrisEngine* e) {
    // List of possible actions
    std::vector<std::function<void()>> actions = {
            [e] { e->moveLeft(); },
            [e] { e->moveRight(); },
            [e] { e->rotateCW(); },
            [e] { e->rotateCCW(); },
            //[e] { e->hardDrop(); },
            [e] { e->hold(); },
            [e] { e->softDropToggle(true); }
    };

    // Pick a random action and execute it
    int randomIndex = std::rand() % actions.size();
    actions[randomIndex]();
}

void clearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD cellCount;
    DWORD count;
    COORD homeCoords = {0, 0};

    if (hConsole == INVALID_HANDLE_VALUE) return;

    // Get console buffer size
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire buffer with spaces
    FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);

    // Reset color attributes
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);

    // Move cursor to top-left
    SetConsoleCursorPosition(hConsole, homeCoords);
}

void handleInput(TetrisEngine* engine) {
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        engine->moveLeft();
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        engine->moveRight();
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000 || GetAsyncKeyState('X') & 0x8000) {
        engine->rotateCW();
    }
    if (GetAsyncKeyState('Z') & 0x8000) {
        engine->rotateCCW();
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        engine->softDropToggle(true);
    } else {
        engine->softDropToggle(false);
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        engine->hardDrop();
    }
    if (GetAsyncKeyState('C') & 0x8000) {
        engine->hold();
    }
}

TetrisEngine* e;

int main() {
    SevenBagGenerator bag(123);
    TetrisEngine engine(&bag);

    e = &engine;

    e->defaultGravity = 1;

    e->onFrameEndCallback = []{
        clearScreen();
        randomAction(e);
        //handleInput(e);
        e->printBoard();
    };

    e->onTopOutCallback = [] {
        e->resetPlayfield();
    };

    e->start();
}