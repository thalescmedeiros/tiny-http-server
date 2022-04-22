#ifndef H_HttpServer
#define H_HttpServer

#ifdef _WIN64
#include "HttpServerWinsock.h"
#else
#ifdef ARDUINO
#include "HttpServerArduino.h"
#else
#include "HttpServerSocket.h"
#endif
#endif

#endif
