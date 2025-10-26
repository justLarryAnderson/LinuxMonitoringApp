#ifndef COMMANDEXECUTOR_H
#define COMMANDEXECUTOR_H

#include <string>
#include <vector>
#include <glibmm.h>

class CommandExecutor {
public:
    CommandExecutor();
    ~CommandExecutor();
    
    bool executeCommand(const std::string& command, std::string& output);
    bool executeCommandAsync(const std::string& command, 
                           const sigc::slot<void, std::string>& callback);
    void stopAllProcesses();
    
private:
    std::vector<Glib::Pid> m_runningPids;
    static void onCommandFinished(Glib::Pid pid, int result, 
                                 const sigc::slot<void, std::string>* callback);
};

#endif