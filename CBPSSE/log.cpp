#include <stdarg.h>
#include "log.h"

#pragma warning(disable : 4996)

// TODO make better macro
#define LOG_ON

CbpLogger::CbpLogger(const char *fname) {
#ifdef LOG_ON
	handle = fopen(fname, "a");
	if (handle) {
		fprintf(handle, "CBP Log initialized\n");
	}
#endif
}

void CbpLogger::Info(const char *fmt...) {
#ifdef LOG_ON
	if (handle) {
		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(handle, fmt, argptr);
		va_end(argptr);
		fflush(handle);
	}
#endif
}

void CbpLogger::Error(const char *fmt...) {
#ifdef LOG_ON
	if (handle) {
		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(handle, fmt, argptr);
		va_end(argptr);
		fflush(handle);
	}
#endif
}

CbpLogger logger("Data\\F4SE\\Plugins\\cbp.log");
