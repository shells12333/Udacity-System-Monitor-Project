#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "linux_parser.h"
#include <stdexcept>
#include <unistd.h>
#include <iomanip>
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
    string os, version, kernel;
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
std::unordered_map<std::string, float> LinuxParser::getMemKeys() {
    std::unordered_map<std::string, float> table;
    std::ifstream file("/proc/meminfo");
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        float val;
        std::string datatype;

        if (iss >> key >> val >> datatype) {
            if (datatype == "kB") { 
                table[key] = val;
            }
        }
    }
    return table;
}

float LinuxParser::MemoryUtilization() {
    std::unordered_map<std::string, float> table = getMemKeys();
    float total = table.at("MemTotal:");
    float free = table.at("MemFree:");
    float used = total - free;
    float utilization = used / total;
    return utilization;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
    string line;
    long uptime {0};
    double uptimeDouble {0};
    std::ifstream file(kUptimeFilename);

    if(file.is_open()) {
        std::getline(file, line);
        std::istringstream iss(line);
        if(iss >> uptimeDouble){
            uptime = static_cast<long>(uptimeDouble);
        }
    }
    return uptime;
 }

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
    long uptime = UpTime();
    return uptime * sysconf(_SC_CLK_TCK);
 }

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
    std::string temp, pid_, temp2;
    std::ifstream file(kProcDirectory + "/" + std::to_string(pid) + "/" + kStatFilename);
    if (file.is_open()) {
        string line;
        long unsigned utime, stime, cutime, cstime;
        
        std::getline(file, line);
        std::istringstream iss(line);
        iss >> pid_ >> temp >> temp2 >> utime >> stime >> cutime >> cstime;
        return utime + stime;
    }
  
    return 0;
 }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
    std::ifstream filestream(kProcDirectory + kStatFilename);
    long active = 0;
    if (filestream.is_open()) {
        string line;
        std::getline(filestream, line);
        std::istringstream linestream(line);
        string cpu;
        long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        linestream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
        long total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
        long idle_jiffies = idle + iowait;
        active = total - idle_jiffies;
    }
    return active;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
    std::ifstream filestream(kProcDirectory + kStatFilename);
    long idle = 0;
    if (filestream.is_open()) {
        string line;
        std::getline(filestream, line);
        std::istringstream linestream(line);
        string cpu;
        long user, nice, system, idle_jiffies, iowait, irq, softirq, steal, guest, guest_nice;
        linestream >> cpu >> user >> nice >> system >> idle_jiffies >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
        idle = idle_jiffies + iowait;
    }
    return idle;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
    vector<string> cpu_util;
    std::ifstream file(kProcDirectory + kStatFilename);
    if (file.is_open()) {
        string line;
        while (std::getline(file, line)) {
            if (line.find("cpu ") == 0) {
                std::istringstream iss(line);
                string cpu;
                iss >> cpu; 
                string token;
                while (iss >> token) {
                    cpu_util.push_back(token);
                }
                break;
            }
        }
    }
    return cpu_util;
}

// Parse the number of running processes or total processes
int LinuxParser::ParseStat(std::string key){
    string line;
    int running {0};
    string _key;
    if (key != "procs_running" && key != "processes"){
        throw std::invalid_argument("Invalid key, use procs_running/processes");
    }

    std::ifstream file(kProcDirectory + kStatFilename);
    while (std::getline(file, line)){
        std::istringstream iss(line);
        iss >> _key;
        if(_key == key){
            iss >> running;
            break;
        }
    }
    return running;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
    return ParseStat("processes");
 }

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
    return ParseStat("procs_running");
  }

// TODO: Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  std::string cmd;
  std::ifstream file(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if(file.is_open()){
    std::getline(file, cmd);
    std::replace(cmd.begin(), cmd.end(), '\0', ' ');
  }
  return cmd;
}


// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
    string line, key, vmrssStr;
    double vmrss {0.0};
    std::ifstream file(kProcDirectory + std::to_string(pid) + kStatusFilename);
    if(file.is_open()){
    while(std::getline(file, line)){
        std::istringstream iss(line);
        if (iss >> key >> vmrss){
        if(key == "VmRSS:"){
            vmrss /= 1024.0; // kB to MB
            std::ostringstream strs;
            strs << std::fixed << std::setprecision(2) << vmrss;
            vmrssStr = strs.str();
            return vmrssStr;
                }
            }
        }
    }
    return "0.0";

}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
    string line, key, userid;
    std::ifstream file(kProcDirectory + std::to_string(pid) + kStatusFilename);
    if(file.is_open()){
    while(std::getline(file, line)){
        std::istringstream iss(line);
        if (iss >> key >> userid){
        if(key == "Uid:"){
            break;
                }
            }
        }
        return userid;
    }
    return "";
}
// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) { 
    string uid = Uid(pid);
    if(uid.empty()){
        return "";
    }

    string line, name, passCheck, uid_;
    std::ifstream file(kPasswordPath);

    if(file.is_open()){
        while(std::getline(file, line)){
            std::replace(line.begin(), line.end(), ':', ' ');
            std::istringstream iss(line);
            if(iss >> name >> passCheck >> uid_){
                if(uid_ == uid){
                    return name;
                }
            }
        }
    }
    return "";
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
    string line, temp;
    long uptimeJ, pUptime;
    const long ticksPerSecond = sysconf(_SC_CLK_TCK);
    std::ifstream file(kProcDirectory + std::to_string(pid) + kStatFilename);
    if(!file.is_open()){
        return 0;
    }
    std::ifstream upfile(kProcDirectory + kUptimeFilename);
    if(!upfile.is_open()){
        return 0;
    }
    
    double upSeconds;
    upfile >> upSeconds;
        std::getline(file, line);
        std::istringstream iss(line);
        for(int i = 0; i <= 21; i++){
            iss >> temp;
        }
        iss >> uptimeJ;
        pUptime = uptimeJ / ticksPerSecond;
        pUptime = static_cast<long>(upSeconds - pUptime);
        return pUptime;
}