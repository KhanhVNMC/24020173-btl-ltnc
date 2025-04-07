cmake_minimum_required(VERSION 3.24)
project(tetisengine)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)
set(CURRENT_NAME $ENV{USERNAME})

set(SDL2_PATH C:/Users/${CURRENT_NAME}/Documents/SDL2-2.28.5/x86_64-w64-mingw32)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(tetisengine
        engine/tetris_engine.H
        engine/tetris_config.h
        engine/javalibs/jsystemstd.h
        sbg.h
        game/sdl2_main.cpp
        renderer/sdl_components.h
        renderer/tetris_renderer.h
        renderer/spritesystem/sprite.cpp
        renderer/spritesystem/sprite.h
        renderer/sprites/gameworld.cpp
        renderer/tetris_player.h renderer/tetris_player_static.cpp renderer/spritesystem/particles.h renderer/spritesystem/particles.h renderer/sprites/player/playerentity.cpp renderer/sprites/player/playerentity.h renderer/sprites/entity_prop.h
        renderer/sprites/entities/Redgga.h
        renderer/sprites/entities/normal_entity.h
        renderer/sprites/entities/normal_entity.cpp renderer/sprites/entities/Blugga.h renderer/sprites/entities/Grigga.h renderer/sprites/entities/Nigga.h renderer/sprites/entities/fairies/debuff_fairy.h renderer/sprites/entities/fairies/debuff_fairy.cpp renderer/sprites/entities/fairies/BlinderFairy.h renderer/sprites/entities/fairies/WeakenerFairy.h renderer/sprites/entities/fairies/DistractorFairy.h renderer/sprites/entities/fairies/DisturberFairy.h game/hooker.cpp game/hooker.h renderer/tetris_player_dbg.cpp renderer/tetris_player_class.cpp)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} winmm opengl32)
