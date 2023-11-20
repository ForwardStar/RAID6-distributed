#include "logger.h"

void Logger::log(int msg_type, const char* msg) {
    switch (msg_type) {
        case INFO:
            printf("[INFO]");
            break;
        case DEBUG:
            printf("[DEBUG]");
            break;
        case WARN:
            printf("[WARN]");
            break;
        case ERROR:
            printf("[ERROR]");
            break;
    }
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char* now_str = std::ctime(&now);
    now_str[strlen(now_str) - 1] = '\0';
    printf(" [%s] %s\n", now_str, msg);
}

void Logger::log(int msg_type, std::string msg) {
    switch (msg_type) {
        case INFO:
            printf("[INFO]");
            break;
        case DEBUG:
            printf("[DEBUG]");
            break;
        case WARN:
            printf("[WARN]");
            break;
        case ERROR:
            printf("[ERROR]");
            break;
    }
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char* now_str = std::ctime(&now);
    now_str[strlen(now_str) - 1] = '\0';
    printf(" [%s] %s\n", now_str, msg.c_str());
}