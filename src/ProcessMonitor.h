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
    std::vector<ProcessInfo> findProcessByPID(const std::string& pid);  // Новый метод для поиска по PID

private:
    std::vector<ProcessInfo> parseProcessOutput(const std::string& output);
    std::vector<ProcessInfo> getAllProcesses();  // Внутренний метод для получения всех процессов
};

#endif
