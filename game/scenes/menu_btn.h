//
// Created by GiaKhanhVN on 4/8/2025.
//

#ifndef TETISENGINE_MENU_BTN_H
#define TETISENGINE_MENU_BTN_H
#include "../../renderer/spritesystem/sprite.h"
#include "../../renderer/sdl_components.h"

class Button : public Sprite {
public:
    /**
     * copied from `tetris_renderer.h`
     */
    static void renderString(SDL_Renderer* renderer, const int x, const int y, const string& str, const double scalar = 5, const float opacity = 1.0f, const int strgap = 28, const int width = 18) {
        const auto texture = disk_cache::bmp_load_and_cache(renderer, FONT_SHEET);
        for (int i = 0; i < str.length(); i++) {
            render_component(renderer, texture, puts_component_char(x + (strgap * i), y, scalar, str[i], width), opacity);
        }
    }

    static void renderStringReverse(SDL_Renderer* renderer, const int x, const int y, const string& str, const double scalar = 5, const float opacity = 1.0f, const int strgap = 40, const int width = 18) {
        const auto texture = disk_cache::bmp_load_and_cache(renderer, FONT_SHEET);
        for (int i = 0; i < str.length(); i++) {
            render_component(renderer, texture, puts_component_char(x + (-strgap * i), y, scalar, str[str.length() - i - 1], width), opacity);
        }
    }
private:
    bool clicked = false;
    int clickedTimeFrame = 0;
public:
    std::string textContent;
    int textXPad = 0;
    int textYPad = 0;

    explicit Button(int width, int height, const std::string text = "button", const int textXPad = 0, const int textYPad = 0)
            : Sprite({ 0, 145, 124, 45 }, width, height, 0) {
        this->setTextureFile("../assets/SPRITES.bmp");
        this->flipSprite(SDL_FLIP_NONE);
        this->textContent = text;
        this->textXPad = textXPad;
        this->textYPad = textYPad;
    }

    void onButtonHover(std::function<void()> onHover) {
        this->onhover([&, onHover]() {
            if (onHover != nullptr) onHover();
        });
    }

    void onButtonClick(std::function<void(int)> onClick) {
        this->onclick([&, onClick](int mouseBtn) {
            clicked = true;
            clickedTimeFrame = 10;
            if (onClick != nullptr) onClick(mouseBtn);
        });
    }

    void onDrawCall() { /* ignored */ }

    void onAfterDrawCall(SDL_Renderer* renderer) override {
        // render text
        Dimension dim = this->getDimension();
        // middle of the button
        renderString(renderer, (x + (dim.width / 10)) + textXPad, (y + (dim.height / 3)) + textYPad, textContent, 2);
    }

    void onDrawCallExtended(SDL_Renderer* renderer) override {
        int textColor = 0xFFFFFF; // white
        // render button state
        if (clicked) { // if clicked, dim it
            if (clickedTimeFrame <= 0) {
                clicked = false;
            } else {
                this->texture.textureY = 237;
                clickedTimeFrame--; // allowed time frame (default 10frames, 1/6 of a sec)
            }
        } else if (hovering) { // if hovering, brighten texture
            this->texture.textureY = 191;
        } else { // otherwise, use default
            this->texture.textureY = 145;
        }
    }
};

#endif //TETISENGINE_MENU_BTN_H
