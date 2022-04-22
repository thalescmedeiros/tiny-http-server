#ifndef H_TimerControl
#define H_TimerControl

#ifdef ARDUINO
typedef unsigned long CUSTOM_TIME;
typedef unsigned long TIME_DIFF;
CUSTOM_TIME GetCustomCurrentTime();
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff);
std::string CustomCurrentTimeToJsonFormat(CUSTOM_TIME time);
#else
#include <chrono>
#include <iomanip>
typedef std::chrono::time_point<std::chrono::system_clock> CUSTOM_TIME;
typedef std::chrono::duration<double> TIME_DIFF;
CUSTOM_TIME GetCustomCurrentTime();
double TIME_DIFF_TO_MSECS_DOUBLE(TIME_DIFF diff);
std::string CustomCurrentTimeToJsonFormat(CUSTOM_TIME time);
#endif

#endif
