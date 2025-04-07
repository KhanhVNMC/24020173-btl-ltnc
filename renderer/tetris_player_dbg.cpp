//
// Created by GiaKhanhVN on 4/7/2025.
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

    render_component_string(renderer, 1420, 470 + offset, "teteng metrics", 1.5, 1, 20);
}

void TetrisPlayer::processSceneInput(SDL_Event &event) {
    if (event.type == SDL_KEYDOWN && !event.key.repeat) {  // avoid key repeat events
        // uh wtf
        switch (event.key.keysym.sym) {
            case SDLK_LEFT: {
                // move left one time first
                tetrisEngine->moveLeft();
                // and then do DAS
                leftHeld = true;
                leftPressTime = System::currentTimeMillis();
                nextLeftShiftTime = leftPressTime + DAS; // the next time it "repeats" the moving action again
                break;
            }
            case SDLK_RIGHT: {
                // move right one time first
                tetrisEngine->moveRight();
                // DAS right
                rightHeld = true;
                rightPressTime = System::currentTimeMillis();
                nextRightShiftTime = rightPressTime + DAS;  // the next time it "repeats" the moving action again
                break;
            }
            case SDLK_UP: // ^ + x = the same
            case SDLK_x: { tetrisEngine->rotateCW(); break; }
            case SDLK_z: { tetrisEngine->rotateCCW(); break; }
            // gravity *= SDF const
            case SDLK_DOWN: { tetrisEngine->softDropToggle(true); break; }
            case SDLK_SPACE: { tetrisEngine->hardDrop(); break; }
            case SDLK_c: { tetrisEngine->hold(); break; }

            /** HANDLE LANE SWITCHING **/
            case SDLK_1:
                moveToLane(0);
                break;
            case SDLK_2:
                moveToLane(1);
                break;
            case SDLK_3:
                moveToLane(2);
                break;
            case SDLK_4:
                moveToLane(3);
                break;
            default: break;
            /* BULLSHIT */
            case SDLK_F3:
                showDebug = !showDebug;
                break;
        }
    }
    else if (event.type == SDL_KEYUP) {  // KEY UP
        switch (event.key.keysym.sym) {
            case SDLK_LEFT: {
                // no longer holding LEFT
                leftHeld = false;
                break;
            }
            case SDLK_RIGHT: {
                // no longer holding RIGHT
                rightHeld = false;
                break;
            }
            case SDLK_DOWN: {
                // no speed boost (SDF)
                tetrisEngine->softDropToggle(false);
                break;
            }
            default: break;
        }
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x = event.button.x;
        int y = event.button.y;
        std::cout << "[DEBUG] Mouse " << x << "," << y << std::endl;
    }
}