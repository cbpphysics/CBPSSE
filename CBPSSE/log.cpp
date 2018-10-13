#include <stdarg.h>
#include "log.h"

#pragma warning(disable : 4996)

Logger::Logger(const char *fname) {
	handle = fopen(fname, "a");
	if (handle) {
		fprintf(handle, "CBP Log initialized\n");
	}
}

void Logger::info(const char *fmt...) {
	if (handle) {
		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(handle, fmt, argptr);
		va_end(argptr);
		fflush(handle);
	}
}

void Logger::error(const char *fmt...) {
	if (handle) {
		va_list argptr;
		va_start(argptr, fmt);
		vfprintf(handle, fmt, argptr);
		va_end(argptr);
		fflush(handle);
	}
}

Logger logger("Data\\SKSE\\Plugins\\cbp.log");
