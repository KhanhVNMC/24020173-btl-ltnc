#include "../sbg.h"
#include "../renderer/tetris_renderer.h"
#include "../renderer/tetris_player.h"
#include "hooker.h"
#include <iostream>

void AttachConsoleToSDL() {
    AllocConsole();  // Create a console
    freopen("CONOUT$", "w", stdout);  // Redirect stdout
    freopen("CONOUT$", "w", stderr);  // Redirect stderr
    freopen("CONIN$", "r", stdin);    // Redirect stdin
}

int main(int argc, char* argv[]) {
    AttachConsoleToSDL();
    std::srand(std::time(0));
    AttachConsoleToSDL();
    initFontSystem();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Tetris: Imaginary War",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1720, 860, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Main loop
    auto *generator = new SevenBagGenerator(123);
    TetrisConfig* config = TetrisConfig::builder();
    config->setLineClearsDelay(0.35);

    ExecutionContext* context = new ExecutionContext();

    auto *tetris = new TetrisEngine(config, generator);
    (new TetrisPlayer(context, renderer, tetris))->startEngineAndGame();

    thread worker([&]() {
        while (context->isRunning()) {
            context->execute();
        }
        cout << "[EC: Thread] ExecutionContext halted!\n";
    });

    // wait for thread to complete
    if (worker.joinable()) {
        worker.join();
    }

    // Clean up heap allocation
    delete context;

    cout << "[MC: Main] Execution complete.\n";
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
