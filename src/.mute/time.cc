#include "mute/time.h"

namespace mute
{
    auto programStartTimePoint = std::chrono::high_resolution_clock::now();

    Time Time::now() { return std::chrono::high_resolution_clock::now(); }
    Time Time::programStartTime() { return programStartTimePoint; }
    Time Time::programElapsedTime() { return now() - programStartTime(); }
}