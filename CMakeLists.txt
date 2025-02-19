cmake_minimum_required(VERSION 3.24)
project(tetisengine)

set(CMAKE_CXX_STANDARD 17)

add_executable(tetisengine main.cpp
        tetrominoes.cpp
        tetrominoes.h
        tetris_engine.cpp
        tetris_engine.h playfield_event.h tetromino_gen_blueprint.h)
