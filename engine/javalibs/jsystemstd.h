//
// Created by GiaKhanhVN on 2/21/2025.
//

/**
 * Mimic some Java features
 */
#ifndef TETISENGINE_JSYSTEMSTD_H
#define TETISENGINE_JSYSTEMSTD_H
#include <chrono>
#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#endif

#define long int64_t

#define System_currentTimeMillis() time_since_epoch_ms()
#define System_nanoTime() time_since_epoch_ns()
#define Thread_sleep(ms) low_resolution_sleep(ms)

inline static long time_since_epoch_ns() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

inline static long time_since_epoch_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

static void preciseSleep(double seconds) {
    using namespace std;
    using namespace std::chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}
inline static void low_resolution_sleep(long milliseconds) {
#ifndef _WIN32
    preciseSleep(milliseconds / 1000.0);
#else
    timeBeginPeriod(1); // Increase resolution to 1ms
    Sleep(milliseconds); // Perform the sleep
    timeEndPeriod(1);   // Restore original resolution
#endif
}
#endif //TETISENGINE_JSYSTEMSTD_H
