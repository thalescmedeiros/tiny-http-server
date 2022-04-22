#ifndef H_TimerControl
#define H_TimerControl

#ifdef ARDUINO
typedef unsigned long CUSTOM_TIME;
typedef unsigned long TIME_DIFF;
CUSTOM_TIME GetCustomCurrentTime();
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff);
#else
#include <chrono>
typedef std::chrono::time_point<std::chrono::system_clock> CUSTOM_TIME;
typedef std::chrono::duration<double> TIME_DIFF;
CUSTOM_TIME GetCustomCurrentTime();
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff);
#endif

#endif
