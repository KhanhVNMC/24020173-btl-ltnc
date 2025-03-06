//
// Created by GiaKhanhVN on 3/3/2025.
//
#include "tetris_player.h"

void TetrisPlayer::sprintfcdbg(TetrisEngine *tetris, int spriteCount) {
    constexpr int offset = 140;
    constexpr int xPos = 1440;
    constexpr int fontSize = 26;

    char buffer[50];

    snprintf(buffer, sizeof(buffer), "tps: %.2f", tetris->ticksPassed / ((System::currentTimeMillis() - tetris->startedAt) / 1000.0));
    render_component_string(renderer, xPos, 670 + offset, buffer, 2, 1, fontSize);

    snprintf(buffer, sizeof(buffer), "cpu: %.2f", tetris->lastTickTime);
    render_component_string(renderer, xPos, 630 + offset, buffer, 2, 1, fontSize);

    snprintf(buffer, sizeof(buffer), "asl: %.0f ms", tetris->dActualSleepTime);
    render_component_string(renderer, xPos, 590 + offset, buffer, 2, 1, fontSize);

    snprintf(buffer, sizeof(buffer), "esl: %.2f", tetris->dExpectedSleepTime);
    render_component_string(renderer, xPos, 550 + offset, buffer, 2, 1, fontSize);

    snprintf(buffer, sizeof(buffer), "spr: %d", spriteCount);
    render_component_string(renderer, xPos, 510 + offset, buffer, 2, 1, fontSize);

    render_component_string(renderer, 1420, 470 + offset, "engine metrics", 1.5, 1, 20);
}

void TetrisPlayer::process_input(SDL_Event &event, TetrisEngine *engine) {
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
                garbageQueue.push_back(1 + std::rand() % 6);
                break;
            case SDLK_y:
                break;
            case SDLK_e:
                for (int i = 0; i < 3; i++) {
                    tetrisEngine->scheduleDelayedTask(5 * i, [&] {
                        tetrisEngine->raiseGarbage(1, 2);
                    });
                }
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