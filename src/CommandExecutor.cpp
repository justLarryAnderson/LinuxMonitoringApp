#include "CommandExecutor.h"
#include <iostream>
#include <glibmm/spawn.h>

CommandExecutor::CommandExecutor() {}

CommandExecutor::~CommandExecutor() {
    stopAllProcesses();
}

bool CommandExecutor::executeCommand(const std::string& command, std::string& output) {
    try {
        int exit_status = 0;
        Glib::spawn_command_line_sync(command, &output, nullptr, &exit_status);
        return exit_status == 0;
    } catch (const Glib::Error& ex) {
        output = "Error: " + std::string(ex.what());
        return false;
    }
}

bool CommandExecutor::executeCommandAsync(const std::string& command, 
                                        const sigc::slot<void, std::string>& callback) {
    try {
        std::string output;
        bool success = executeCommand(command, output);
        callback(output);
        return success;
    } catch (const Glib::Error& ex) {
        callback("Error: " + std::string(ex.what()));
        return false;
    }
}

void CommandExecutor::stopAllProcesses() {
    // В упрощенной версии просто очищаем список
    m_runningPids.clear();
}

void CommandExecutor::onCommandFinished(Glib::Pid pid, int result, 
                                      const sigc::slot<void, std::string>* callback) {
    // Пустая реализация для упрощения
    if (callback) {
        delete callback;
    }
}