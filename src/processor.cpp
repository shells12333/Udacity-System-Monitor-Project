#include <string>
#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() { 

    std::string line, cpu;
    std::ifstream file(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    static float oldTotal {0};
    static float oldIdle {0};

    if (!file.is_open()) {
        return 0.0;
    }

    std::getline(file, line);
    std::istringstream iss(line);

    
    if (!(iss >> cpu >> user_ >> nice_ >> system_ >> idle_ >> iowait_ >> irq_ >> softirq_ >> steal_ >> guest_ >> guest_nice_)) {
        // kept segfaulting, added format safeguard
        return 0.0;
    }

    total = user_ + nice_ + system_ + idle_ + iowait_ + irq_ + softirq_ + steal_ + guest_ + guest_nice_;
    float idle = idle_ + iowait_;
    
    float total_ = total - oldTotal;
    float idle_ = idle - oldIdle;

    // track old stats
    oldTotal = total;
    oldIdle = idle;

    // prevent divide by zero
    if (total_ == 0) {
        return 0.0;
    }

    return (total_ - idle_) / total_;
}
