#ifndef H_GlobalSettings
#define H_GlobalSettings

#define HTTP_PORT 8080
#define HTTP_READ_BUFFER_SIZE 1024
#define HTTP_WRITE_BUFFER_SIZE 4096
#define MAX_HTTP_REQUEST_SIZE 1 * 1024 * 1024  // 1 MB
#define MAX_HTTP_CLIENTS 20
#define MAX_TIMEOUT_DISCONNECT 60000 // miliseconds
#define MAX_PROCESSING_TIME 200 // ms - max time for each processing TICK per client

#endif
