#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <string>
#include <vector>
#include <gtkmm.h>

struct ProcessInfo {
    std::string pid;
    std::string name;
    std::string cpu;
    std::string memory;
    std::string status;
};

class ProcessMonitor {
public:
    ProcessMonitor();
    std::vector<ProcessInfo> getRunningProcesses();
    std::vector<ProcessInfo> findProcesses(const std::string& pattern);
    
private:
    std::vector<ProcessInfo> parseProcessOutput(const std::string& output);
};

#endif