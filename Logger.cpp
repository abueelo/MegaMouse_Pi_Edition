#include "Logger.h"

#include <cstdio>
#include <cstdarg>

void Logger::Log(const char *message)
{
    printf("%s\n", message);
}

void Logger::Logf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}
