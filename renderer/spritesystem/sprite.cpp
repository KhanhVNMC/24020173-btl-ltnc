//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "sprite.h"
#include <cmath>
#include "../disk_cache.h"

long RENDER_PASSES = 0;
long OBJECT_POOL = 0;
std::unordered_map<long, Sprite*> ACTIVE_SPRITES;

SpriteLoc Sprite::getLocation() const {
    return { x, y, rotationState };
}

void Sprite::spawn() {
    SpritesRenderingPipeline::getSprites().insert({this->spriteId, this});
}

void Sprite::discard() const {
    SpritesRenderingPipeline::getSprites().erase(this->spriteId);
    delete this;
}

SpriteTexture* Sprite::getTexture() const {
    return texture;
}

void Sprite::teleport(const int x, const int y) {
    this->x = x;
    this->y = y;
}

void Sprite::scale(const double newScale) {
    this->scalar = newScale;
}

void Sprite::rotate(const int newRotation) {
    this->rotationState = (newRotation + 360) % 360;
}

constexpr double PI = 3.14159265358979323846;
void Sprite::setDirection(int x, int y) {
    rotate(static_cast<int>(atan2(y - this->y, x - this->x) * 180.0 / PI) + 90);
}

void Sprite::render(SDL_Renderer* renderer) {
    this->onDrawCall();

    const SDL_Rect source = { this->texture->textureX, this->texture->textureY, this->texture->width, this->texture->height };
    const SDL_Rect dest = { this->x, this->y, static_cast<int>(this->width * scalar), static_cast<int>(this->height * scalar) };

    const auto _textureSDL = disk_cache::bmp_load_and_cache(renderer, "../assets/SPRITES.bmp");

    if (this->rotationState == 0) {
        SDL_RenderCopy(renderer, _textureSDL, &source, &dest);
        return;
    }
    SDL_RenderCopyEx(renderer, _textureSDL, &source, &dest, rotationState, nullptr, SDL_FLIP_NONE);
}

