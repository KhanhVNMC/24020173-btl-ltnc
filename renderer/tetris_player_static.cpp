//
// Created by GiaKhanhVN on 3/3/2025.
//
#include "tetris_player.h"
#include "spritesystem/particles.h"

void TetrisPlayer::spawnPhysicsBoundText(
        string str, int x, int y, double randVelX, double randVelY,
        int lifetime, double gravity, double scalar,
        int strgap, int width, const int* colors,
        const int applyThisColorToAll,
        bool priority
) {
    for (int i = 0; i < str.length(); i++) {
        auto [source, dest] = puts_component_char(x + (strgap * i), y, scalar, str[i], width);
        const auto part = new Particle(
                // texture
                {source.x, source.y, source.w, source.h},
                // destination
                dest.w, dest.h, dest.x, dest.y,
                randVelX, randVelY, lifetime, gravity
        );
        part->setTextureFile(FONT_SHEET);
        if (applyThisColorToAll != -1) part->setTint(applyThisColorToAll);
        else if (colors != nullptr) part->setTint(colors[i]);
        part->spawn(priority);
    }
}

void TetrisPlayer::spawnDamageIndicator(const int x, const int y, const int damage, bool offensive) {
    const string damageStr = std::to_string(damage);
    const double scalar = 3;
    const int strgap = 40, width = 18;
    const double randDmgVelX = randomFloat(-2, 4), randDmgVelY = -randomFloat(-1, 2);

    spawnPhysicsBoundText(damageStr, x, y, randDmgVelX, randDmgVelY, 80, 0.05, scalar, strgap, width, nullptr, offensive ? MINO_COLORS[6] : MINO_COLORS[1]);
}

void TetrisPlayer::spawnBoardTitle(const int x, const int y, string title, const int *colors) {
    spawnPhysicsBoundText(title, x, y, -0.1, 0, 60, 0.03, 2.5, 30, 15, colors);
}

void TetrisPlayer::spawnBoardSubtitle(const int x, const int y, string title, const int color) {
    spawnPhysicsBoundText(title, x, y, -0.1, 0, 60, 0.03, 2, 20, 15, nullptr, color);
}

void TetrisPlayer::spawnBoardMiniSubtitle(const int x, const int y, string title, const int color) {
    spawnPhysicsBoundText(title, x, y, -0.1, 0, 60, 0.03, 1.5, 20, 15, nullptr, color);
}

void TetrisPlayer::spawnMiscIndicator(const int x, const int y, string indicator, const int color) {
    spawnPhysicsBoundText(indicator, x, y, 0.5, 0, 60, 0.03, 2, 20, 15, nullptr, color);
}

void TetrisPlayer::spawnPriorityIndicator(const int x, const int y, string indicator, const int color) {
    spawnPhysicsBoundText(indicator, x, y, 0, 0.0, 90, -0.03, 3, 20, 15, nullptr, color, true);
}

void TetrisPlayer::renderThunderbolt(SDL_Renderer* renderer, const int x, const int y, const int offset) {
    auto cached = disk_cache::bmp_load_and_cache(renderer, MAIN_SPRITE_SHEET);
    const struct_render_component component = {
            58 + (23 * offset), 0, 22, 30,
            x, y, static_cast<int>(22 * 1.25), static_cast<int>(30 * 1.25)
    };
    render_component(renderer, cached, component, 1);
}

void TetrisPlayer::spawnMiddleScreenText(const int x, const int y, string title, const int color, const int life) {
    spawnPhysicsBoundText(title, x, y, 0.0, 0, life, 0.01, 5, 50, 15, nullptr, color, true);
}

void TetrisPlayer::renderArmor(SDL_Renderer* renderer, const int x, const int y, const int offset) {
    auto cached = disk_cache::bmp_load_and_cache(renderer, MAIN_SPRITE_SHEET);
    const struct_render_component component = {
            230 + (19 * offset), 0, 18, 18,
            x, y, static_cast<int>(18 * 1.25), static_cast<int>(18 * 1.25)
    };
    render_component(renderer, cached, component, 1);
}

void TetrisPlayer::renderDebuffIcon(SDL_Renderer* renderer, const int x, const int y, const int offset) {
    auto cached = disk_cache::bmp_load_and_cache(renderer, MAIN_SPRITE_SHEET);
    const struct_render_component component = {
            340 + (21 * offset), 0, 20, 22,
            x, y, static_cast<int>(20 * 2.25), static_cast<int>(22 * 2.25)
    };
    render_component(renderer, cached, component, 1);
}