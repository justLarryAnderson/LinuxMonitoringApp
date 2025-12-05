#include "ProcessMonitor.h"
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <memory>

ProcessMonitor::ProcessMonitor() {}

std::vector<ProcessInfo> ProcessMonitor::getRunningProcesses() {
    std::vector<ProcessInfo> processes;
    
    // Используем ps для получения списка процессов
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ps aux --sort=-%cpu | head -20", "r"), pclose);
    if (!pipe) return processes;
    
    char buffer[256];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != NULL) {
        result += buffer;
    }
    
    return parseProcessOutput(result);
}

std::vector<ProcessInfo> ProcessMonitor::findProcesses(const std::string& pattern) {
    std::vector<ProcessInfo> allProcesses = getRunningProcesses();
    std::vector<ProcessInfo> filteredProcesses;
    
    for (const auto& process : allProcesses) {
        if (process.name.find(pattern) != std::string::npos) {
            filteredProcesses.push_back(process);
        }
    }
    
    return filteredProcesses;
}

std::vector<ProcessInfo> ProcessMonitor::parseProcessOutput(const std::string& output) {
    std::vector<ProcessInfo> processes;
    std::istringstream stream(output);
    std::string line;
    
    // Пропускаем заголовок
    std::getline(stream, line);
    
    while (std::getline(stream, line)) {
        std::istringstream lineStream(line);
        ProcessInfo info;
        
        std::string user, pid, cpu, mem, vsz, rss, tty, stat, start, time, command;
        
        // Парсим вывод ps aux
        lineStream >> user >> pid >> cpu >> mem >> vsz >> rss >> tty >> stat >> start >> time;
        
        // Остальное - команда
        std::getline(lineStream, command);
        
        // Сохраняем все поля
        info.user = user;
        info.pid = pid;
        info.cpu = cpu;
        info.memory = mem;
        info.name = command.empty() ? "unknown" : command.substr(command.find_first_not_of(" "));
        info.status = stat;
        
        processes.push_back(info);
    }
    
    return processes;
}