#ifndef PROCESSMONITOR_H
#define PROCESSMONITOR_H

#include <string>
#include <vector>
#include <gtkmm.h>

struct ProcessInfo {
    std::string user;    // Пользователь, запустивший процесс
    std::string pid;     // ID процесса
    std::string name;    // Имя процесса
    std::string cpu;     // Использование CPU в %
    std::string memory;  // Использование памяти в %
    std::string status;  // Статус процесса
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