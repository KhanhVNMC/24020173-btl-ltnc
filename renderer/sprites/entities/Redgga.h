#ifndef REDGGA_H
#define REDGGA_H
#include "normal_entity.h"

class Redgga final : public NormalEntity {
public:
    Redgga(const void* tetrisPlayer) : NormalEntity(tetrisPlayer) {
        this->setTextureFile("../assets/enemy_01.bmp"); // red nigga
        this->setDamageThresholds(2, 3); // 3-5 dmg
        this->setAttackSpeed(5); // 15s between attacks
        this->setMaxHealth(30);
    }
};

#endif //REDGGA_H
