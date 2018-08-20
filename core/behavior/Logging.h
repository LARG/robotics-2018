#include <memory/TextLogger.h>
#define tlog(level, fstring, ...) textlogger->logFromBehavior(level, fstring, ##__VA_ARGS__)
