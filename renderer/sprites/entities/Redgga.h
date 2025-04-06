#ifndef REDGGA_H
#define REDGGA_H
#include "normal_entity.h"

class Redgga final : public NormalEntity {
public:
    Redgga(const void* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->setTextureFile("../assets/enemy_01.bmp"); // red nigga
        this->setDamageThresholds(2, 5); // 2-5 dmg
        this->setAttackSpeed(20); // 20s between attacks
        this->setDifficulty(HARD); // this is a hard boss
        this->setMaxHealth(16); // 16HP (4 tetris(s) to clear)
    }
};

#endif //REDGGA_H
