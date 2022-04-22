#ifndef H_HttpClient
#define H_HttpClient

#ifdef _WIN64
#include "HttpClientWinsock.h"
#else
#ifdef ARDUINO
#include "HttpClientArduino.h"
#else
#include "HttpClientSocket.h"
#endif
#endif

#endif
