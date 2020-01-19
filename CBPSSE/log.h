#pragma once
#include <stdio.h>

class Logger {
public:
    Logger(const char* fname);
    void info(const char* args...);	
    void error(const char* args...);

    FILE *handle;
};

extern Logger logger;