//
// Created by GiaKhanhVN on 2/26/2025.
//

#include "sprite.h"
#include <cmath>
#include "../sdl_components.h"

long RENDER_PASSES = 0;
std::map<long, Sprite*> ACTIVE_SPRITES;

void Sprite::setTextureFile(const std::string& textureSpriteFile) {
    this->textureSheetPath = textureSpriteFile;
}

void Sprite::setupTexture(const SpriteTexture texture, const std::string& textureSpriteFile) {
    this->setTextureFile(textureSpriteFile);
    this->texture = texture;
    this->originalTextureX = texture.textureX;
    this->originalTextureY = texture.textureY;
}

void Sprite::flipSprite(const int newState) {
    this->sdlFlipState = newState;
}

SpriteLoc Sprite::getLocation() const {
    return { x, y, rotationState };
}

void Sprite::spawn() {
    if (!heapAllocated) {
        throw logic_error("YOU CANNOT SPAWN STACK-ALLOCATED OBJECTS!!");
    }
    SpritesRenderingPipeline::getSprites().insert({this->spriteId, this});
}

void Sprite::discard() const {
    if (!heapAllocated) return;
    SpritesRenderingPipeline::getSprites().erase(this->spriteId);
    delete this;
}

SpriteTexture* Sprite::getTexture() {
    return &texture;
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

[[maybe_unused]] void Sprite::setDirection(const int x, const int y) {
    rotate(static_cast<int>(atan2(y - this->y, x - this->x) * 180.0 / PI) + 90);
}

void Sprite::render(SDL_Renderer* renderer) {
    this->onDrawCall();

    const SDL_Rect source = { this->texture.textureX, this->texture.textureY, this->texture.width, this->texture.height };
    const SDL_Rect dest = { this->x, this->y, static_cast<int>(this->width * scalar), static_cast<int>(this->height * scalar) };

    const auto _textureSDL = disk_cache::bmp_load_and_cache(renderer, textureSheetPath);
    onBeforeTextureDraw(_textureSDL);

    if (this->rotationState == 0 && this->sdlFlipState == SDL_FLIP_NONE) {
        SDL_RenderCopy(renderer, _textureSDL, &source, &dest);
        return;
    }
    SDL_RenderCopyEx(renderer, _textureSDL, &source, &dest, this->rotationState, nullptr,static_cast<const SDL_RendererFlip>(this->sdlFlipState));
    // reset color
    SDL_SetTextureColorMod(_textureSDL, 0xFF, 0xFF, 0xFF);
}


