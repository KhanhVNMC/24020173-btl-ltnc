//
// Created by GiaKhanhVN on 4/8/2025.
//

#ifndef TETISENGINE_GAME_OVER_MENU_H
#define TETISENGINE_GAME_OVER_MENU_H

#include <SDL_render.h>
#include <cmath>
#include "../gamescene.h"
#include "../hooker.h"
#include "../../renderer/sdl_components.h"
#include "../../renderer/spritesystem/sprite.h"
#include "../../engine/javalibs/jsystemstd.h"
#include "menu_btn.h"

typedef struct {
    long long score;
    int killedEnemies;
    int damageSent;
    int wavesCleared;
    bool lost;
    long long gameLength;
    bool endless;
} GameOverInfo;

class GameOverScreen : public GameScene {
    ExecutionContext* context;
    SDL_Renderer* renderer;

    int hookId;
    GameOverInfo info;

    public: GameOverScreen(GameOverInfo ginfo, ExecutionContext* executionContext, SDL_Renderer* sdlRenderer) {
        this->context = executionContext;
        this->renderer = sdlRenderer;
        this->info = ginfo;
    }

    #define CENTER_X_POS 600
    #define CENTER_Y_POS 420

    int clock;
    void menuLoop() {
        string text = info.lost ? "game over!" : "stage clear!";
        renderWavyString(CENTER_X_POS + 260, CENTER_Y_POS - 280, text, 4, 60);

        const int infoTitleX = CENTER_X_POS - 70;
        const int infoTitleY = CENTER_Y_POS - 80;

        // render mode
        Button::renderString(renderer, infoTitleX + 120, CENTER_Y_POS - 200, (info.endless ? "endless " : "campaign") + std::string(" mode"), 2, 1, 30);

        // render info - score
        Button::renderString(renderer, infoTitleX, infoTitleY, "score:", 2, 1, 30);
        Button::renderStringReverse(renderer, infoTitleX + 600, infoTitleY, to_string(info.score), 3, 1, 40);

        // render info - waves cleared
        Button::renderString(renderer, infoTitleX + 3, infoTitleY + 50, "waves:", 2, 1, 30);
        Button::renderStringReverse(renderer, infoTitleX + 600, infoTitleY + 50, to_string(info.wavesCleared), 3, 1, 40);

        // render info - killed
        Button::renderString(renderer, infoTitleX, infoTitleY + 100, "kills:", 2, 1, 30);
        Button::renderStringReverse(renderer, infoTitleX + 600, infoTitleY + 100, to_string(info.killedEnemies), 3, 1, 40);

        // render info - total dmg
        Button::renderString(renderer, infoTitleX, infoTitleY + 150, "attacks:", 2, 1, 30);
        Button::renderStringReverse(renderer, infoTitleX + 600, infoTitleY + 150, to_string(info.damageSent), 3, 1, 40);

        // render info - time
        Button::renderString(renderer, infoTitleX, infoTitleY + 200, "time:", 2, 1, 30);
        const int64_t timePassedMs = info.gameLength;
        const int64_t timePassedS  = timePassedMs / 1000;

        const int64_t displayMs    = timePassedMs % 1000;
        const int64_t displayMin   = timePassedS / 60;
        const int64_t displaySec   = timePassedS % 60;

        const auto timeString = str_printf("%02d:%02d.%02d", displayMin, displaySec, (displayMs / 10));
        Button::renderStringReverse(renderer, infoTitleX + 600, infoTitleY + 200, timeString, 3, 1, 40);
    }

    void renderWavyString(float centerX, float centerY, const std::string& text, int scale, int strgap) {
        float startX = centerX - (text.length() * strgap) / 2.0f;
        for (size_t i = 0; i < text.length(); ++i) {
            float x = startX + i * strgap;
            float y = centerY + sin(i + ((clock++) / 20) * 0.25) * 10;

            Button::renderString(renderer, x, y, std::string(1, text[i]), scale, 1, 0);
        }
    }

    function<void(ExecutionContext*, SDL_Renderer*)> onSwitchContextCallback = nullptr;
    void stopScene() override {
        context->unhook(this->hookId, [&]() {
            if (onSwitchContextCallback) onSwitchContextCallback(this->context, this->renderer);
            delete this;
        });
    }

    void startScene() override {
        // clean current rendering context to begin a new life
        SpritesRenderingPipeline::stopAndCleanCurrentContext();
        this->hookId = context->hook([&]() { onContextTick(); });

        // return to menu button
        Button* backButton = (new Button(600, 50, "return to menu", 40, -5));
        backButton->onButtonClick([&](int m) {
            // return to menu by switching context
            onSwitchContextCallback = [&](ExecutionContext* ctx, SDL_Renderer* rend) {
                // back to main menu
                context->contextReturnMainMenu();
            };
            this->stopScene();
        });
        backButton->onButtonHover([]{});
        backButton->teleport(550, 660);
        backButton->spawn();

        // rage quit button
        Button* quitButton = (new Button(600, 50, "quit game", 100, -5));
        quitButton->onButtonClick([&](int m) {
            exit(0);
        });
        quitButton->onButtonHover([]{});
        quitButton->teleport(550, 720);
        quitButton->spawn();
    }

    SDL_Texture* cachedTexture = nullptr;
    void onContextTick() {
        // begin render the scene
        SDL_RenderClear(renderer);
        // render background
        if (cachedTexture == nullptr) cachedTexture = disk_cache::bmp_load_and_cache(renderer, BACKGROUND_SHEET);
        const struct_render_component bkgComponent = {
                0, 0, 1720, 860,
                0, 0, 1720, 860
        };
        render_component(renderer, cachedTexture, bkgComponent, 0.25);

        // render low priority sprites first
        SpritesRenderingPipeline::renderNormal(renderer);

        // then the tetris board
        this->menuLoop();

        Thread::sleep(16);

        // what the fuck
        SDL_RenderPresent(renderer); // Show updated frame
    }
};


#endif //TETISENGINE_GAME_OVER_MENU_H
