#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>  // std::isinf

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Constructor
Process::Process(int pid) : pid_(pid) {}

// TODO: Return this process's ID
int Process::Pid() const { return pid_; }

// TODO: Return this process's CPU utilization
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat
float Process::CpuUtilization() const {
  float system_uptime_sec = static_cast<float>(LinuxParser::UpTime());
  float proc_starttime_jiffies =
      static_cast<float>(LinuxParser::StartTimeJiffies(Pid()));
  float proc_totaltime_jiffies =
      static_cast<float>(LinuxParser::ActiveJiffies(Pid()));
  float sc_clk_tck = static_cast<float>(sysconf(_SC_CLK_TCK));
  float seconds = system_uptime_sec - (proc_starttime_jiffies / sc_clk_tck);
  float proc_cpu_usage = (proc_totaltime_jiffies / sc_clk_tck) / seconds;
  if(std::isinf(proc_cpu_usage)){
    proc_cpu_usage = 1.0F;  // Normalized to 1, see usage * 100
  }
  return proc_cpu_usage;
}

// TODO: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(Pid()); }

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Pid()); }

// TODO: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Pid()); }

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a) const {
  return (this->CpuUtilization() > a.CpuUtilization());  // > to begin with larger
}