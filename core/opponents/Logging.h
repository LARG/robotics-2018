#include <memory/TextLogger.h>
#define tlog(level, fstring, ...) textlogger->logFromOpp(level, fstring, ##__VA_ARGS__)
