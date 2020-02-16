#pragma once
#include <stdio.h>

class CbpLogger {
public:
    CbpLogger(const char* fname);
    void Info(const char* args...);	
    void Error(const char* args...);

    FILE *handle;
};

extern CbpLogger logger;