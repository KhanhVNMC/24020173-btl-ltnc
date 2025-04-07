#include "hooker.h"
#include "scenes/main_menu.h"
#include "scenes/loading_screen.h"
#include <iostream>

void AttachConsoleToSDL() {
#ifdef WIN32
    AllocConsole();  // Create a console
    freopen("CONOUT$", "w", stdout);  // Redirect stdout
    freopen("CONOUT$", "w", stderr);  // Redirect stderr
    freopen("CONIN$", "r", stdin);    // Redirect stdin
#endif
}

#define WINDOW_HEIGHT 860
#define WINDOW_WIDTH 1720

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-console") == 0) {
            AttachConsoleToSDL();
            break;
        }
    }
    initFontSystem();
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Tetris VS: Demo",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // Main loop
    ExecutionContext* context = new ExecutionContext();

    // register main menu hook
    context->contextReturnMainMenu = [&, context, renderer]() {
        GameScene* menu = new MainMenu(context, renderer);
        menu->startScene();
    };

    // load shit in
    auto* loadingScreen = new LoadingScreen(context, renderer);
    loadingScreen->fakeLoadFor = 90 + (::rand() % 60);
    loadingScreen->onLoadingScreenInit = [&, renderer]() {
        SysAudio::initSoundSystem();
        SysAudio::preloadDefinedAudioFiles();
        disk_cache::preload_defined_sheets(renderer);
    };

    // on screen complete
    loadingScreen->onLoadingScreenComplete = [&](ExecutionContext* context, SDL_Renderer*) {
        context->contextReturnMainMenu();
    };
    loadingScreen->startScene();

    thread worker([&]() {
        // each thread has its own fucking RNG
        srand(System::currentTimeMillis());
        // start context
        while (context->isRunning()) {
            context->execute();
        }
    });

    SDL_Event event;
    bool stopped = false;
    while (context->isRunning()) {
        // Poll events in the main thread
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                context->stop();  // Gracefully stop the context if the user closes the window
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F1 && !stopped) {
                    //player->stopScene();
                    stopped = true;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                // detect click and do callbacks
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                for (auto& [id, sprite] : SpritesRenderingPipeline::getSprites()) {
                    sprite->checkIfClicked(mouseX, mouseY, event.button.button);
                }
                for (auto& [id, sprite] : SpritesRenderingPipeline::getPrioritySprites()) {
                    sprite->checkIfClicked(mouseX, mouseY, event.button.button);
                }
            } else if (event.type == SDL_MOUSEMOTION) {
                // detect hover and do callbacks
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                for (auto& [id, sprite] : SpritesRenderingPipeline::getSprites()) {
                    sprite->checkIfHovered(mouseX, mouseY);
                }
                for (auto& [id, sprite] : SpritesRenderingPipeline::getPrioritySprites()) {
                    sprite->checkIfHovered(mouseX, mouseY);
                }
            }
            context->pushEvent(event);
        }
        Thread::sleep(16);
    }

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
