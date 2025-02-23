#include "sbg.h"
#include "renderer/sdl_inc.h"

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("BMP Partial Render",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          980, 720, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    // Main loop
    int running = 1;
    SDL_Event event;

    auto *generator = new SevenBagGenerator(123);
    TetrisConfig* config = TetrisConfig::builder();
    config->setLineClearsDelay(0.5);

    auto *tetris = new TetrisEngine(config, generator);

    tetris->runOnTickEnd([&] {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(1);
            }
        }

        SDL_RenderClear(renderer); // Clear screen
        render_tetris_board(220, 50, renderer, tetris);

        SDL_RenderPresent(renderer); // Show updated frame
    });

    tetris->start();

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
