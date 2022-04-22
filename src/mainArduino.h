#ifdef ARDUINO

#ifndef H_MAINARDUINO
#define H_MAINARDUINO

#include <memory>
#include "HttpServerControl.h"

class ArduinoApp
{
    public:
        static bool Setup();
        static void Loop();

    private:
        std::unique_ptr<HttpServerControl> httpServer;
};

#endif

#endif
