#ifndef THORFINN_PIPELINE_H
#define THORFINN_PIPELINE_H

#include "config.h"
#include <libssh2.h>
#include <functional>
#include <optional>
#include <string>

class Pipeline {
public:
    Pipeline(const Config& config, const std::string& workingDir);
    ~Pipeline();
    bool execute();
    bool establishSSHConnection(const SSHGlobalConfig& sshConfig);
    void closeSSHConnection();
    LIBSSH2_SESSION* getSSHSession() const;

private:
    const Config& config_;
    std::string workingDir_;
    LIBSSH2_SESSION* sshSession;
    LIBSSH2_CHANNEL* sshChannel;
    std::optional<std::vector<const Step*>> resolveStepOrder() const;
    bool executeStep(const Step& step);
    void handleStepActions(const std::vector<std::map<std::string, std::string>>& actions, const std::string& stepName, const std::string& output);
    bool establishSSHConnection(const std::map<std::string, std::string>& sshConfig);
};

#endif