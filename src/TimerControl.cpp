#include "TimerControl.h"

#ifdef ARDUINO
CUSTOM_TIME GetCustomCurrentTime() { return millis(); }
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff) { return (double)diff; }
#else
#include <chrono>
CUSTOM_TIME GetCustomCurrentTime() { return std::chrono::system_clock::now(); }
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff) { return diff.count() * 1000; }
#endif
