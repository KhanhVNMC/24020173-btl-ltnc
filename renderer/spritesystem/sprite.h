//
// Created by GiaKhanhVN on 2/26/2025.
//

#ifndef SPRITE_H
#define SPRITE_H
#include <SDL_render.h>
#include <string>
#include <map>
#include <iostream>
#include <vector>

typedef struct {
    int x, y, rot;
} SpriteLoc;

static long SPRITES_OBJECT_POOL = 0;

typedef struct {
    int textureX, textureY;
    int width, height;
} SpriteTexture;

typedef struct {
    int width;
    int height;
} Dimension;

class Sprite {
public:
    virtual ~Sprite() = default;
protected:
    int x = 0, y = 0;
    int width = 0, height = 0;
    SpriteTexture texture{0, 0, 0, 0};

    int rotationState = 0;
    double scalar = 1;

    int originalTextureX = 0, originalTextureY = 0;
public:
    const long spriteId;
protected:
    std::string textureSheetPath = "../assets/SPRITES.bmp";
    bool isPriority = false;
    int sdlFlipState = SDL_FLIP_NONE;
public:
    Sprite(SpriteTexture texture, const int width, const int height, const int initialRotation = 0) : spriteId(SPRITES_OBJECT_POOL++) {
        this->width = width;
        this->height = height;
        this->rotationState = initialRotation;
        this->setupTexture(texture); // set up the texture for later use
        this->x = this->y = 0; // default the location to 0, 0 (top left)
    }

    void setTextureFile(const std::string& textureSpriteFile);
    void setupTexture(SpriteTexture texture, const std::string& textureSpriteFile = "../assets/SPRITES.bmp");

    /**
     * @return the sprite Location object (x,y,rot)
     */
    [[nodiscard]] SpriteLoc getLocation() const;

    /**
     * Insert this sprite to the global renderer proc
     */
    void spawn(bool priority = false);

    /**
     * Remove this sprite from the global renderer process, YOU SHOULD
     * NOT USE ANY REFERENCES/POINTERS TO THIS THIS CLASS AFTER discard()
     */
    void discard();

    /**
     * @return the texture obj (pointer)
     */
    SpriteTexture* getTexture();

    /**
     * @return the dimension after scaling of the sprite
     */
    Dimension getDimension();

    /**
     * Move this sprite to a specific location on the screen
     * @param x x coordinate
     * @param y y coordinate
     */
    void teleport(int x, int y);

    /**
     * 360 degrees rotation
     * @param newRotation
     */
    void rotate(int newRotation);

    /**
     * Scale the width and height of the
     * output sprite (not the texture)
     * @param newScale
     */
    void scale(double newScale);

    /**
     * Make the sprite face a specific coordinate
     * @param x x-coord
     * @param y y-coord
     */
    void setDirection(int x, int y);

    /**
     * SDL Flip
     * @param newState
     */
    void flipSprite(int newState);

    // event callers
    virtual void onDrawCall() = 0;
    virtual void onDrawCallExtended(SDL_Renderer* renderer) {};
    virtual void onBeforeTextureDraw(SDL_Texture* sdlTexture) {};

    /**
     * Internal method
     * @param renderer the SDL renderer
     */
    void render(SDL_Renderer* renderer);

private:
    // if this sprite is allocated on the heap
    bool heapAllocated;
public:
    static void* operator new(const size_t size) {
        void* ptr = ::operator new(size);
        // do not allow this sprite to enter the rendering pipeline if it was NOT on the heap
        ((Sprite*)ptr)->heapAllocated = true;
        return ptr;
    }

    static void operator delete(void* ptr) {
        ::operator delete(ptr);
    }
};

extern long RENDER_PASSES;
extern std::map<long, Sprite*> ACTIVE_SPRITES;
extern std::vector<Sprite*> deletionQueue;

extern std::map<long, Sprite*> PRIORITY_ACTIVE_SPRITES;
extern std::vector<Sprite*> priorityDeletionQueue;

namespace SpritesRenderingPipeline {
    /**
     * @return normal, non prioritized sprites
     */
    static std::map<long, Sprite*>& getSprites() {
        return ACTIVE_SPRITES;
    }

    /**
     * @return prioritized sprites
     */
    static std::map<long, Sprite*>& getPrioritySprites() {
        return PRIORITY_ACTIVE_SPRITES;
    }

    /**
     * Render the first layer of sprites, overridden by the tetris playfield
     * @param renderer
     */
    static void renderNormal(SDL_Renderer* renderer) {
        for (auto&[fst, snd]: ACTIVE_SPRITES) {
            snd->render(renderer);
        }
        // clean up the garbage (prevent crashes)
        for (Sprite* sprite: deletionQueue) {
            SpritesRenderingPipeline::getSprites().erase(sprite->spriteId);
            // actually deleting the shit
            delete sprite;
        }
        deletionQueue.clear();
        RENDER_PASSES++;
    }

    /**
     * Render the priority layer of sprites, stay on top of everything (Except debug menu)
     * @param renderer
     */
    static void renderPriority(SDL_Renderer* renderer) {
        for (auto&[fst, snd]: PRIORITY_ACTIVE_SPRITES) {
            snd->render(renderer);
        }
        // clean up the garbage (prevent crashes)
        for (Sprite* sprite: priorityDeletionQueue) {
            SpritesRenderingPipeline::getPrioritySprites().erase(sprite->spriteId);
            // actually deleting the shit
            delete sprite;
        }
        priorityDeletionQueue.clear();
    }

    /**
     * @return How much frames have passed
     */
    static long renderPasses() {
        return RENDER_PASSES;
    }

    // this could leak memory but fuck it.
    static void stopAndCleanCurrentContext() {
        for (auto& sprite : SpritesRenderingPipeline::getSprites()) {
            sprite.second->discard();
        }
        for (auto& sprite : SpritesRenderingPipeline::getPrioritySprites()) {
            sprite.second->discard();
        }
    }
}

#endif //SPRITE_H
