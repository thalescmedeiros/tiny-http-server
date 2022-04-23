#ifndef ARDUINO

#include <memory>
#include <iostream>
#include <thread>
#include "HttpServerControl.h"
#include "TimerControl.h"

int main(int argc, char const *argv[])
{
    auto httpServer = std::unique_ptr<HttpServerControl>(new HttpServerControl());
    httpServer->Initialize();

    CUSTOM_TIME lastMessageSent = GetCustomCurrentTime();
    while (true)
    {
        // Mandatory TICK to handle everything
        httpServer->Tick();

        // Sleeping for 10ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Adding a message to be sent to all WebSockets every 5 seconds
        CUSTOM_TIME timeNow = GetCustomCurrentTime();
        double timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(timeNow - lastMessageSent);
        if (timeEllapsed > 5000)
        {
            lastMessageSent = timeNow;
            httpServer->SendTextToWSClients(EVENT_WS_CUSTOM_1, std::string("{\"Text\":\"hi again!\"}"));
        }
    }

    return 0;
}

#endif
