cmake_minimum_required(VERSION 3.24)
project(tetisengine)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)
set(CURRENT_NAME $ENV{USERNAME})

set(SDL2_PATH C:/Users/${CURRENT_NAME}/Documents/SDL2-2.28.5/x86_64-w64-mingw32)
set(SDL2_MIXER_PATH C:/Users/${CURRENT_NAME}/Documents/SDL2_mixer-2.8.1/x86_64-w64-mingw32)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

include_directories(
        ${SDL2_INCLUDE_DIR}
        ${SDL2_MIXER_INCLUDE_DIRS}
)
link_directories(
        ${SDL2_PATH}/lib
        ${SDL2_MIXER_PATH}/lib
)

if(WIN32)
    set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc" src/engine/tetris_engine.cpp)
endif()

add_executable(tetisengine
        src/engine/tetris_engine.h
        src/engine/tetris_config.h
        src/engine/javalibs/jsystemstd.h
        src/process/bag_generator.h
        src/process/sdl2_main.cpp
        src/game/sdl_components.h
        src/game/tetris_renderer.h
        src/game/spritesystem/sprite.cpp
        src/game/spritesystem/sprite.h
        src/game/sprites/gameworld.cpp
        src/game/tetris_player.h
        src/game/tetris_player_static.cpp
        src/game/spritesystem/particles.h
        src/game/spritesystem/particles.h
        src/game/sprites/player/playerentity.cpp
        src/game/sprites/player/playerentity.h
        src/game/sprites/entity_prop.h
        src/game/sprites/entities/Redgga.h
        src/game/sprites/entities/normal_entity.h
        src/game/sprites/entities/normal_entity.cpp
        src/game/sprites/entities/Blugga.h
        src/game/sprites/entities/Grigga.h
        src/game/sprites/entities/Nigga.h
        src/game/sprites/entities/fairies/debuff_fairy.h
        src/game/sprites/entities/fairies/debuff_fairy.cpp
        src/game/sprites/entities/fairies/BlinderFairy.h
        src/game/sprites/entities/fairies/WeakenerFairy.h
        src/game/sprites/entities/fairies/DistractorFairy.h
        src/game/sprites/entities/fairies/DisturberFairy.h
        src/process/hooker.cpp src/process/hooker.h
        src/game/tetris_player_dbg.cpp
        src/game/tetris_player_class.cpp
        src/process/gamescene.h
        src/process/scenes/main_menu.h
        src/process/scenes/menu_btn.h
        src/process/scenes/game_over_screen.h
        src/engine/tetrominoes.cpp
        src/process/scenes/loading_screen.h
        src/engine/javalibs/jsystemstd.cpp
        src/game/sdl_component.cpp
        ${APP_ICON_RESOURCE}
)

# if debug then use "../assets" and -deb
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VERSION_IDF_STR "ltnc 0.1.1-demo-deb")
    target_compile_definitions(tetisengine PRIVATE DEBUG_BUILD)
else()
    set(VERSION_IDF_STR "ltnc 0.1.1-demo")
endif()

# select optimize level (O1 to prevent my jank from blowing up)
set(CMAKE_CXX_FLAGS_RELEASE "-O1 -DNDEBUG")

add_compile_definitions(VERSION_IDF="${VERSION_IDF_STR}")
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} winmm opengl32)
