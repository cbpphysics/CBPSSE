#pragma once
#include <stdio.h>

class CbpLogger {
public:
    CbpLogger(const char* fname);
    void info(const char* args...);	
    void error(const char* args...);

    FILE *handle;
};

extern CbpLogger logger;