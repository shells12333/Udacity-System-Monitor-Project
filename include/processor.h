#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <linux_parser.h>

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp
    float total, idle, nonIdle, total_, idle_, oldTotal, oldIdle;
  // TODO: Declare any necessary private members
 private:
 std::string cpu_;
  float user_, nice_, system_, iowait_, irq_, softirq_, steal_, guest_, guest_nice_;
};

#endif