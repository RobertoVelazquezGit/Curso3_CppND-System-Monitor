#include "format.h"

#include <iomanip>
#include <sstream>
#include <string>

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long times) {
  long hour, seconds_over_hour, minutes, seconds;
  std::stringstream ss;

  hour = times / 3600;
  seconds_over_hour = times - hour * 3600;
  minutes = seconds_over_hour / 60;
  seconds = seconds_over_hour % 60;
  ss << std::setw(2) << std::setfill('0') << hour << std::setw(1) << ":"
     << std::setw(2) << std::setfill('0') << minutes << std::setw(1) << ":"
     << std::setw(2) << std::setfill('0') << seconds;
  return ss.str();
}