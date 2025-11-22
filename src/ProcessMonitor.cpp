#include "ProcessMonitor.h"
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <memory>

ProcessMonitor::ProcessMonitor() {}

std::vector<ProcessInfo> ProcessMonitor::getRunningProcesses() {
    std::vector<ProcessInfo> processes;

    // Используем ps для получения списка процессов (топ 20 по CPU)
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ps aux --sort=-%cpu | head -20", "r"), pclose);
    if (!pipe) return processes;

    char buffer[256];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != NULL) {
        result += buffer;
    }

    return parseProcessOutput(result);
}

std::vector<ProcessInfo> ProcessMonitor::getAllProcesses() {
    std::vector<ProcessInfo> processes;

    // Получаем все процессы (без ограничения head)
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ps aux", "r"), pclose);
    if (!pipe) return processes;

    char buffer[256];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != NULL) {
        result += buffer;
    }

    return parseProcessOutput(result);
}

std::vector<ProcessInfo> ProcessMonitor::findProcesses(const std::string& pattern) {
    std::vector<ProcessInfo> allProcesses = getAllProcesses();  // Используем все процессы для поиска
    std::vector<ProcessInfo> filteredProcesses;

    for (const auto& process : allProcesses) {
        // Ищем по имени ИЛИ по PID
        if (process.name.find(pattern) != std::string::npos ||
            process.pid.find(pattern) != std::string::npos) {
            filteredProcesses.push_back(process);
        }
    }

    return filteredProcesses;
}

std::vector<ProcessInfo> ProcessMonitor::findProcessByPID(const std::string& pid) {
    std::vector<ProcessInfo> allProcesses = getAllProcesses();
    std::vector<ProcessInfo> result;

    for (const auto& process : allProcesses) {
        if (process.pid == pid) {  // Точное совпадение по PID
            result.push_back(process);
            break;  // PID уникален, можно прервать поиск после нахождения
        }
    }

    return result;
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

        lineStream >> user >> pid >> cpu >> mem >> vsz >> rss >> tty >> stat >> start >> time;

        // Остальное - команда
        std::getline(lineStream, command);

        info.pid = pid;
        info.cpu = cpu;
        info.memory = mem;
        info.name = command.empty() ? "unknown" : command.substr(command.find_first_not_of(" "));
        info.status = stat;

        processes.push_back(info);
    }

    return processes;
}
