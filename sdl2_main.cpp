#include "sbg.h"
#include "renderer/sdl_inc.h"
#include "renderer/tetris_player.cpp"

int main(int argc, char* argv[]) {
    initFontSystem();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Tetris: Imaginary War",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1720, 860, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Main loop
    int running = 1;
    SDL_Event event;

    auto *generator = new SevenBagGenerator(123);
    TetrisConfig* config = TetrisConfig::builder();
    config->setLineClearsDelay(0.35);

    Sprite* background = new BackgroundScroll(SDL_FLIP_NONE, 0, 0, 2);
    background->spawn();
    Sprite* background2 = new BackgroundScroll(SDL_FLIP_NONE, 2048, 0, 2);
    background2->spawn();

    Sprite* background3 = new BackgroundScroll(SDL_FLIP_HORIZONTAL, 0, 0, 3);
    background3->spawn();

    Sprite* background4 = new BackgroundScroll(SDL_FLIP_HORIZONTAL, 2048, 0, 3);
    background4->spawn();

    auto *tetris = new TetrisEngine(config, generator);
    new TetrisPlayer(renderer, tetris);

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
