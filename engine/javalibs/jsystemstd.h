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
#define SND_ASYNC      0x0001
#define SND_FILENAME   0x00020000

// no fucking windows.h, it fucked up my code
extern "C" __declspec(dllimport) void WINAPI Sleep(DWORD dwMilliseconds);
extern "C" __declspec(dllimport) void WINAPI timeBeginPeriod(DWORD period);
extern "C" __declspec(dllimport) void WINAPI timeEndPeriod(DWORD period);
extern "C" __declspec(dllimport) BOOL WINAPI AllocConsole(void);
#endif

/* Java's long, signed 64-bit integer */
#define LONG int64_t

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
}

#endif //TETISENGINE_JSYSTEMSTD_H
