#include "TimerControl.h"

#ifdef ARDUINO
CUSTOM_TIME GetCustomCurrentTime() { return millis(); }
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff) { return (double)diff; }
std::string CustomCurrentTimeToJsonFormat(CUSTOM_TIME time)
{
    return "0000-00-00T00:00:00Z";
}
#else
#include <chrono>
#include <iomanip>
#include <sstream>
CUSTOM_TIME GetCustomCurrentTime() { return std::chrono::system_clock::now(); }
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff) { return diff.count() * 1000; }
std::string CustomCurrentTimeToJsonFormat(CUSTOM_TIME time)
{
    auto in_time_t = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}
#endif
