#ifndef OPENJOEY_DEBUG_LOG_H
#define OPENJOEY_DEBUG_LOG_H

#include <cstdio>
#ifdef _WIN32
#include <windows.h>
#endif

static const char* const kDebugLogPath = "c:\\Users\\shsuw\\Documents\\2026\\.cursor\\debug.log";

static inline void DebugLogLine(const char* hyp, const char* loc, const char* msg, const char* dataKey, int dataVal) {
    FILE* f = NULL;
    if (fopen_s(&f, kDebugLogPath, "a") == 0 && f) {
        fprintf(f, "{\"sessionId\":\"debug-session\",\"runId\":\"run1\",\"hypothesisId\":\"%s\",\"location\":\"%s\",\"message\":\"%s\",\"data\":{\"%s\":%d},\"timestamp\":%lu}\n",
            hyp, loc, msg, dataKey, dataVal, (unsigned long)GetTickCount());
        fclose(f);
    }
}

#endif
