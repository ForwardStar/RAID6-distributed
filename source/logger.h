#ifndef LOGGER
#define LOGGER
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <chrono>

enum logger_level {
    INFO,
    DEBUG,
    WARN,
    ERROR
};

class Logger {
    public:
        void log(int msg_type, const char* msg);
        void log(int msg_type, std::string msg);

        Logger() {};
        ~Logger() {};
};

#endif