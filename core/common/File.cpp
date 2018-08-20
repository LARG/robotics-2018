#include <common/File.h>

std::string generateTimestamp() {
  std::string timestamp = "_%y_%m_%d-%H_%M_%S";
  time_t rawtime;
  struct tm *timeInfo;
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  char buffer[80];
  strftime(buffer, 80, timestamp.c_str(), timeInfo);
  timestamp = buffer;
  return timestamp;
}
