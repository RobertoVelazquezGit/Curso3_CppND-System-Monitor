#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  float memtotal;
  float memfree;
  string value;
  string kb;
  bool memtotalcaptured = false;
  bool memfreecaptured = false;
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value >> kb) {
        // Validate before convert
        if (key == "MemTotal:") {
          memtotal = std::stof(value);
          memtotalcaptured = true;
        }
        if (key == "MemFree:") {
          memfree = std::stof(value);
          memfreecaptured = true;
        }
      }
      if (memtotalcaptured && memfreecaptured) {
        break;
      }
    }
  }
  return (memtotal - memfree) / memtotal;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  string line, uptime_str, dummy;
  long uptime = 0;
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kUptimeFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> uptime_str >> dummy) {
        // Validate before convert
        if (uptime_str != "") {
          uptime = std::stol(uptime_str);
        }
      }
    }
  }
  return uptime;  // seconds
}

// TODO: Read and return the number of jiffies for the system
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string value_str;
  long utime, stime, cutime, cstime;
  long total_time = 0U;
  const unsigned int utime_index = 13U;
  const unsigned int stime_index = 14U;
  const unsigned int cutime_index = 15U;
  const unsigned int cstime_index = 16U;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      unsigned int value_index = 0U;
      bool must_iterate = true;
      while (must_iterate && linestream >> value_str) {
        // Validate before
        if (value_str != "") {
          switch (value_index) {
            case utime_index:
              utime = std::stol(value_str);
              break;
            case stime_index:
              stime = std::stol(value_str);
              break;
            case cutime_index:
              cutime = std::stol(value_str);
              break;
            case cstime_index:
              cstime = std::stol(value_str);
              must_iterate = false;
              break;
            default:
              // Intended
              break;
          }
        }
        value_index++;
      }
    }
    total_time = utime + stime + cutime + cstime;
  }
  return total_time;  // jiffies
}

// TODO: Read and return the number of active jiffies for the system
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();
  return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) +
         stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_]) +
         stol(jiffies[CPUStates::kSoftIRQ_]) +
         stol(jiffies[CPUStates::kSteal_]);
}

// TODO: Read and return the number of idle jiffies for the system
// https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();
  return (stol(jiffies[CPUStates::kIdle_]) +
          stol(jiffies[CPUStates::kIOwait_]));
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, cpu_dummy, tick_value;
  vector<string> jiffies;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> cpu_dummy;
      while (linestream >> tick_value) {
        jiffies.push_back(tick_value);
      }
      // Though only first line (cpu) matters in this project
    }
  }
  return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int totalproc = 0;
  string totalproc_str;
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> totalproc_str) {
        // Validate before
        if (key == "processes") {
          totalproc = std::stoi(totalproc_str);
          return totalproc;
        }
      }
    }
  }
  return totalproc;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int procrunning = 0;
  string procrunning_str;
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> procrunning_str) {
        // Validate before
        if (key == "procs_running") {
          procrunning = std::stoi(procrunning_str);
        }
      }
    }
  }
  return procrunning;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    return line;
  } else {
    return string();
  }
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string key, ramkb, unitskbdummy, line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> ramkb >> unitskbdummy) {
        if (key == "VmSize:") {
          int ramMbyte;
          try {
            ramMbyte = std::stoi(ramkb) / 1024;
          } catch (std::invalid_argument& e) {
            ramMbyte = 0;
          }
          return to_string(ramMbyte);
        }
      }
    }
  }
  return string();
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string key, uid;
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> uid;
      if (key == "Uid:") {  // you can see Uid: on /proc/[pid]/status
        return uid;
      }
    }
  }
  return string();
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string username, dummy, read_uid, line;
  std::ifstream filestream(kPasswordPath);
  string uidtofind = LinuxParser::Uid(pid);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> username >> dummy >> read_uid;
      if (uidtofind == read_uid) {
        return username;
      }
    }
  }
  return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  string line, value;
  vector<string> linestring;
  long uptime = 0;
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        linestring.push_back(value);
      }
    }
    const unsigned int uptime_index = 21U;
    uptime = std::stol(linestring[uptime_index]) / sysconf(_SC_CLK_TCK);
  }
  return uptime;  // seconds
}

// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
long LinuxParser::StartTimeJiffies(int pid) {
  string line;
  string value_str;
  long starttime = 0;
  const unsigned int starttime_index = 21U;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      unsigned int value_index = 0U;
      bool must_iterate = true;
      while (must_iterate && linestream >> value_str) {
        // Validate before
        if (value_str != "") {
          switch (value_index) {
            case starttime_index:
              starttime = std::stol(value_str);
              must_iterate = false;
              break;
            default:
              // Intended
              break;
          }
        }
        value_index++;
      }
    }
  }
  return starttime;  // jiffies
}

