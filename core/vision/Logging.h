#include <memory/TextLogger.h>
#define tlog(level, fstring, ...) textlogger->logFromVision(level, fstring, ##__VA_ARGS__)
