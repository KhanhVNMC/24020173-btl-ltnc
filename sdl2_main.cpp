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

[[maybe_unused]] void sprintfcdbg(SDL_Renderer* renderer, TetrisEngine* tetris) {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "tps: %.2f", ((float) tetris->ticksPassed / ((System::currentTimeMillis() - tetris->startedAt) / 1000.0)));
    std::string str(buffer);
    render_component_string(renderer, 1000, 670, str, 2, 1, 26);

    char buffer2[50];
    snprintf(buffer2, sizeof(buffer2), "etl: %.2f", ((float) tetris->lastTickTime));
    std::string str2(buffer2);
    render_component_string(renderer, 1000, 630, str2, 2, 1, 26);

    char buffer3[50];
    snprintf(buffer3, sizeof(buffer3), "cpu: %.0f ms", tetris->dActualSleepTime);
    std::string str3(buffer3);
    render_component_string(renderer, 1000, 590, str3, 2, 1, 26);

    char buffer4[50];
    snprintf(buffer4, sizeof(buffer4), "tsl: %.2f", ((float) tetris->dExpectedSleepTime));
    std::string str4(buffer4);
    render_component_string(renderer, 1000, 550, str4, 2, 1, 26);

    render_component_string(renderer, 980, 510, "engine metrics", 1.5, 1, 20);
}

int main(int argc, char* argv[]) {
    initFontSystem();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Tetris - Nigga Edition",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Main loop
    int running = 1;
    SDL_Event event;

    auto *generator = new SevenBagGenerator(123);
    TetrisConfig* config = TetrisConfig::builder();
    config->setLineClearsDelay(0.35);

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

        sprintfcdbg(renderer, tetris);

        SDL_RenderPresent(renderer); // Show updated frame
    });

    tetris->start();

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
