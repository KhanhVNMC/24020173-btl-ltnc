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
    // attach console to this game window
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-console") == 0) {
            AttachConsoleToSDL();
            break;
        }
    }

    // SDL initialize for window first
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(("Tetris VS: Concept Demo (" + std::string(VERSION_IDF) + ")").c_str(),SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // create the execution context for the entire lifecycle
    auto* context = new ExecutionContext();
    initFontSystem(); // the loading screen uses font, too (this is fast)
    srand(System::currentTimeMillis()); // main thread rng

    // register main menu hook (will be invoked multiple times later)
    context->contextReturnMainMenu = [&, context, renderer]() {
        GameScene* menu = new MainMenu(context, renderer);
        menu->startScene();
    };

    // load shit in
    auto* loadingScreen = new LoadingScreen(context, renderer);
    loadingScreen->fakeLoadFor = 160 + (rand() % 60); // lmfao, fake loading
    // starts to load
    loadingScreen->onLoadingScreenInit = [&, renderer]() {
        SysAudio::initSoundSystem();
        SysAudio::preloadDefinedAudioFiles();
        disk_cache::preload_defined_sheets(renderer);
        // play the intro audio
        SysAudio::playSoundAsync(INTRO_AUD, SysAudio::getSFXVolume(), false);
    };

    // on screen complete
    loadingScreen->onLoadingScreenComplete = [&](ExecutionContext* context, SDL_Renderer*) {
        // show the main menu once loaded everything
        context->contextReturnMainMenu();
    };

    // start the first scene of the game, the loading screen.
    loadingScreen->startScene();

    // start context executor's internal thread
    cout << "[THREAD: EXEC CONTEXT] Starting ExecutionContext internal Thread..." << endl;
    thread worker([&]() {
        // each thread has its own fucking RNG (init one here)
        srand(System::currentTimeMillis());
        // context executor will run as long as its alive
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
        Thread::sleep(16); // 60.0FPS, std of the thing
    }

    // wait for thread to complete
    if (worker.joinable()) {
        worker.join();
    }

    // Clean up heap allocation
    delete context;

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
