#include <stdarg.h>
#include "log.h"

#pragma warning(disable : 4996)

// TODO make better macro
#define LOG_ON 1

Logger::Logger(const char *fname) {
#ifdef LOG_ON
	handle = fopen(fname, "a");
	if (handle) {
		fprintf(handle, "CBP Log initialized\n");
	}
#endif
}

void Logger::info(const char *fmt...) {
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

void Logger::error(const char *fmt...) {
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

Logger logger("Data\\F4SE\\Plugins\\cbp.log");
