/*
 * timers.cpp
 *
 * Implementation for some generic timers defined mostly in timers.h.
 */

#include <chrono>

#include "timers.hpp"
#include "stats.hpp"

extern "C" {
/* execute a 1-cycle loop 'iters' times */
bench2_f add_calibration;
}

using namespace Stats;
using namespace std::chrono;

/*
 * Calculate the frequency of the CPU based on timing a tight loop that we expect to
 * take one iteration per cycle.
 *
 * ITERS is the base number of iterations to use: the calibration routine is actually
 * run twice, once with ITERS iterations and once with 2*ITERS, and a delta is used to
 * remove measurement overhead.
 */
template <size_t ITERS, typename CLOCK, size_t TRIES = 10, size_t WARMUP = 100>
double CalcCpuFreq() {
    const char* mhz;
    if ((mhz = getenv("UARCH_BENCH_CLOCK_MHZ"))) {
        double ghz = std::stoi(mhz) / 1000.0;
        return ghz;
    }

    std::array<nanoseconds::rep, TRIES> results;

    for (size_t w = 0; w < WARMUP + 1; w++) {
        for (size_t r = 0; r < TRIES; r++) {
            auto t0 = CLOCK::nanos();
            add_calibration(ITERS, nullptr);
            auto t1 = CLOCK::nanos();
            add_calibration(ITERS * 2, nullptr);
            auto t2 = CLOCK::nanos();
            results[r] = (t2 - t1) - (t1 - t0);
        }
    }

    DescriptiveStats stats = get_stats(results.begin(), results.end());

    double ghz = ((double)ITERS / stats.getMedian());
    return ghz;
}

template <typename CLOCK>
double ClockTimerT<CLOCK>::getGHz() {
    static double ghz = CalcCpuFreq<10000,CLOCK,1000>();
    return ghz;
}

// explicit instantiation for the default clock
template double DefaultClockTimer::getGHz();




