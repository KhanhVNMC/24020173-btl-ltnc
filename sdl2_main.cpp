#include "sbg.h"
#include "renderer/sdl_inc.h"

void process_input(SDL_Event& event, TetrisEngine* engine) {
    if (event.type == SDL_KEYDOWN && !event.key.repeat) {  // Avoid key repeat events
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                engine->moveLeft();
                break;
            case SDLK_RIGHT:
                engine->moveRight();
                break;
            case SDLK_UP:
            case SDLK_x:
                engine->rotateCW();
                break;
            case SDLK_z:
                engine->rotateCCW();
                break;
            case SDLK_DOWN:
                engine->softDropToggle(true);
                break;
            case SDLK_SPACE:
                engine->hardDrop();
                break;
            case SDLK_c:
                engine->hold();
                break;
            case SDLK_t:
                engine->raiseGarbage(3, 9);
                break;
            default:
                break;
        }
    }
    else if (event.type == SDL_KEYUP) {  // Handle key releases
        if (event.key.keysym.sym == SDLK_DOWN) {
            engine->softDropToggle(false);
        }
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("BMP Partial Render",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

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
            process_input(event, tetris);
        }

        SDL_RenderClear(renderer);
        //SDL_RenderDrawLine(renderer, 100, 100, 100, 100);
        render_tetris_board(60, 20, renderer, tetris);
        //render_tetris_board(660, 50, renderer, tetris2);

        SDL_RenderPresent(renderer); // Show updated frame
    });

    tetris->start();
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
