//
// Created by GiaKhanhVN on 2/26/2025.
//

#ifndef SPRITE_H
#define SPRITE_H
#include <SDL_render.h>
#include <string>
#include <map>
#include <iostream>

typedef struct {
    int x, y, rot;
} SpriteLoc;

static long SPRITES_OBJECT_POOL = 0;

typedef struct {
    int textureX, textureY;
    int width, height;
} SpriteTexture;

class Sprite {
public:
    virtual ~Sprite() = default;
protected:
    int y = 0;
    int width = 0, height = 0;
    SpriteTexture texture{0, 0, 0, 0};

    int rotationState = 0;
    double scalar = 1;

    int originalTextureX = 0, originalTextureY = 0;

    const long spriteId;
    std::string textureSheetPath = "../assets/SPRITES.bmp";

    int sdlFlipState = SDL_FLIP_NONE;
    int x = 0;
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
    void spawn();

    /**
     * Remove this sprite from the global renderer process, YOU SHOULD
     * NOT USE ANY REFERENCES/POINTERS TO THIS THIS CLASS AFTER discard()
     */
    void discard() const;

    /**
     * @return the texture obj (pointer)
     */
    [[nodiscard]] SpriteTexture* getTexture();

    // basic movement & scaling
    void teleport(int x, int y);
    void rotate(int newRotation);
    void scale(double newScale);

    // advanced
    [[maybe_unused]] void setDirection(int x, int y);
    void flipSprite(int newState);

    // event callers
    virtual void onDrawCall() = 0;
    virtual void onBeforeTextureDraw(SDL_Texture* texture) {};
    void render(SDL_Renderer* renderer);

private:
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

namespace SpritesRenderingPipeline {
    static std::map<long, Sprite*>& getSprites() {
        return ACTIVE_SPRITES;
    }

    static void renderEverything(SDL_Renderer* renderer) {
        for (auto&[fst, snd]: ACTIVE_SPRITES) {
            snd->render(renderer);
        }
        RENDER_PASSES++;
    }

    static long renderPasses() {
        return RENDER_PASSES;
    }
}

#endif //SPRITE_H
