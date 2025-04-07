//
// Created by GiaKhanhVN on 2/21/2025.
//

/**
 * Replicate some of Java's standard libraries (namely System, Thread, etc)
 */
#ifndef TETISENGINE_JSYSTEMSTD_H
#define TETISENGINE_JSYSTEMSTD_H
#include <chrono>
#include <cstdint>
#include <string>

#ifdef _WIN32
typedef unsigned long DWORD;
typedef int BOOL;
#define WINAPI __stdcall
typedef const char* LPCSTR;
typedef void* HMODULE;
typedef int BOOL;
typedef unsigned long DWORD;

// no fucking windows.h, it fucked up my code
extern "C" __declspec(dllimport) void WINAPI Sleep(DWORD dwMilliseconds);
extern "C" __declspec(dllimport) void WINAPI timeBeginPeriod(DWORD period);
extern "C" __declspec(dllimport) void WINAPI timeEndPeriod(DWORD period);
extern "C" __declspec(dllimport) BOOL WINAPI AllocConsole(void);
extern "C" __declspec(dllimport) void* WINAPI GetStdHandle(unsigned int);
extern "C" __declspec(dllimport) int WINAPI GetConsoleMode(void* hConsole, unsigned int* lpMode);
extern "C" __declspec(dllimport) int WINAPI SetConsoleMode(void* hConsole, unsigned int dwMode);
#endif

/* Java's long, signed 64-bit integer */
#define LONG int64_t

/* AUDIO */
#define BGM_AUD "bgm.mp3"
#define UI_CLICK "click.mp3"
#define TOP_OUT_AUD "topout.wav"
#define ROTATE_AUD "rotate.wav"
#define HARD_DROP_AUD "harddrop.wav"
#define TETRO_MOVE_AUD "move.wav"
#define GAME_OVER_AUD "gameover.wav"
#define ENTITY_ATTACK_AUD "e_attack.mp3"
#define ENTITY_ATTACK_MAGIC_AUD "e_cursed.ogg"
#define PIECE_HOLD_AUD "hold.wav"
#define PLAYER_ATTACK_AUD "p_attack.ogg"
#define LC_SPIN_AUD "clearspin.wav"
#define LC_QUAD_AUD "clearquad.wav"
#define LC_NORM_AUD "clearline.wav"
#define LC_PERFC_AUD "allclear.wav"
#define WAVE_CLEAR_AUD "wave_done.ogg"
#define COUNTDOWN_AUD "countdown.mp3"

/* SPRITE SHEETS */ //"../assets/SPRITES.bmp"
#define MAIN_SPRITE_SHEET "SPRITES.bmp"
#define GAME_LOGO_SHEET "logo.bmp"
#define FONT_SHEET "font.bmp"
#define PLAYER_SHEET "flandre.bmp"
#define BACKGROUND_SHEET "load_scr.bmp"
#define TETROMINOES "tetrominoes.bmp"
#define PARALLAX_SHEET "starlight.bmp"

#define REDGGA_SHEET "enemy_01.bmp"
#define BLUGGA_SHEET "enemy_02.bmp"
#define GRIGGA_SHEET "enemy_03.bmp"
#define NIGGA_SHEET "enemy_04.bmp"

#define FAIRY_DISTRACTOR_SHEET "f_distractor.bmp"
#define FAIRY_BLINDER_SHEET "f_blinder.bmp"
#define FAIRY_DISTURBER_SHEET "f_disturber.bmp"
#define FAIRY_WEAKENER_SHEET "f_weakener.bmp"

/* BUILD SYSTEMS */
#define DEBUG_BUILD

namespace System {
    /**
     * Returns the current value of the running NOT Java Virtual Machine's
     * high-resolution time source, in nanoseconds.
     *
     * @return the current value of the NOT running Java Virtual Machine's
     *         high-resolution time source, in nanoseconds
     * @warning I copied this from Java's official doc
     */
    inline static LONG nanoTime() {
        const auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    }

    /**
    * Returns the current time in milliseconds. Note that
    * while the unit of time of the return value is a millisecond,
    * the granularity of the value depends on the underlying
    * operating system and may be larger.  For example, many
    * operating systems measure time in units of tens of
    * milliseconds.
    *
    * @return  the difference, measured in milliseconds, between
    *          the current time and midnight, January 1, 1970 UTC.
    * @warning I copied this from Java's official doc
    */
    inline static LONG currentTimeMillis() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    }
}

namespace Thread {
    /**
     * Causes the currently executing thread to sleep (temporarily cease
     * execution) for the specified number of milliseconds, subject to
     * the precision and accuracy of system timers and schedulers.
     *
     * @param  milliseconds the length of time to sleep in milliseconds
     * @warning I copied this from Java's official doc
     */
    inline static void sleep(LONG milliseconds) {
    #ifndef _WIN32
        // for macos and linux, they dont have bullshit timing slice problems like windows
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    #else
        timeBeginPeriod(1); // Increase resolution to 1ms
        Sleep(milliseconds); // Perform the sleep
        timeEndPeriod(1);   // Restore original resolution
    #endif
    }
}

namespace SysAudio {
    bool initSoundSystem();
    void playSoundAsync(const std::string& path, int volume, bool repeat);
    void stopAudio();
    void shutdownSoundSystem(); // optional
    int getBGMVolume();
    int getSFXVolume();
    void preloadDefinedAudioFiles();
}

#endif //TETISENGINE_JSYSTEMSTD_H
