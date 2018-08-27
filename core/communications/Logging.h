#include <memory/TextLogger.h>
#define tlog(level, fstring, ...) textlogger->logFromCommunication(level, fstring, ##__VA_ARGS__)
