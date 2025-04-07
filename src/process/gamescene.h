//
// Created by GiaKhanhVN on 4/7/2025.
//

#ifndef TETISENGINE_GAMESCENE_H
#define TETISENGINE_GAMESCENE_H

class GameScene {
public:
    virtual void stopScene() = 0;
    virtual void startScene() = 0;

    virtual ~GameScene() = default;
};

#endif //TETISENGINE_GAMESCENE_H
