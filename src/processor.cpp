#include "processor.h"

#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  float active =  static_cast<float>(LinuxParser::ActiveJiffies());
  float total = static_cast<float>(LinuxParser::Jiffies());
  return active / total;
}