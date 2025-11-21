#include "CommandExecutor.h"
#include <iostream>
#include <glibmm/spawn.h>
#include <glibmm/threads.h>

CommandExecutor::CommandExecutor() {}

CommandExecutor::~CommandExecutor() {
    stopAllProcesses();
}

bool CommandExecutor::executeCommand(const std::string& command, std::string& output) {
    try {
        int exit_status = 0;
        std::string full_command = "/bin/bash -c '" + command + "'";
        Glib::spawn_command_line_sync(full_command, &output, nullptr, &exit_status);
        
        return exit_status == 0 || exit_status == 1;
    } catch (const Glib::Error& ex) {
        output = "Error: " + std::string(ex.what());
        return false;
    }
}

bool CommandExecutor::executeCommandAsync(const std::string& command, 
                                        const sigc::slot<void, std::string>& callback) {
    std::thread([this, command, callback]() {
        std::string output;
        bool success = executeCommand(command, output);
        
        Glib::signal_idle().connect_once([callback, output, success]() {
            callback(output);
        });
    }).detach();
    
    return true;
}

void CommandExecutor::stopAllProcesses() {
}

void CommandExecutor::onCommandFinished(Glib::Pid pid, int result, 
                                      const sigc::slot<void, std::string>* callback) {
    if (callback) {
        delete callback;
    }
}