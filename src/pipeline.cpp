#include "pipeline.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <functional>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

Pipeline::Pipeline(const Config& config, const std::string& workingDir) : config_(config), workingDir_(workingDir), sshSession(nullptr), sshChannel(nullptr) {
    if (libssh2_init(0) != 0) {
        std::cerr << "Error initializing libssh2." << std::endl;
    }
    if (!config_.ssh_global_config.host.empty()) {
        std::cout << "Attempting global SSH connection..." << std::endl;
        establishSSHConnection(config_.ssh_global_config);
    }
}

Pipeline::~Pipeline() {
    closeSSHConnection();
    libssh2_exit();
}


bool Pipeline::establishSSHConnection(const std::string& host, int port, const std::string& username, const std::string& password) {
    closeSSHConnection();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return false;
    }


    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(host.c_str());
    if (sin.sin_addr.s_addr == INADDR_NONE) {
        struct hostent* hp = gethostbyname(host.c_str());
        if (hp == nullptr) {
            std::cerr << "Error: Could not resolve hostname: " << host << std::endl;
            close(sock);
            return false;
        }
        memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
    }

    if (connect(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr_in)) != 0) {
        perror("connect");
        close(sock);
        return false;
    }

    sshSession = libssh2_session_init();
    if (!sshSession) {
        std::cerr << "Error initializing SSH session." << std::endl;
        close(sock);
        return false;
    }

    if (libssh2_session_handshake(sshSession, sock) != LIBSSH2_ERROR_NONE) {
        char* err_msg;
        int err_code;
        libssh2_session_last_error(sshSession, &err_msg, &err_code, 0);
        std::cerr << "Error during SSH handshake: " << err_msg << " (" << err_code << ")" << std::endl;
        closeSSHConnection();
        close(sock);
        return false;
    }

    if (libssh2_userauth_password(sshSession, username.c_str(), password.c_str()) != LIBSSH2_ERROR_NONE) {
        char* err_msg;
        int err_code;
        libssh2_session_last_error(sshSession, &err_msg, &err_code, 0);
        std::cerr << "Error during SSH password authentication: " << err_msg << " (" << err_code << ")" << std::endl;
        closeSSHConnection();
        close(sock);
        return false;
    }

    std::cout << "SSH connection established to " << host << ":" << port << " as user " << username << std::endl;
    return true;
}

bool Pipeline::establishSSHConnection(const SSHGlobalConfig& sshConfig) {
    return establishSSHConnection(sshConfig.host, sshConfig.port, sshConfig.username, sshConfig.password);
}

bool Pipeline::establishSSHConnection(const std::map<std::string, std::string>& sshConfig) {
    if (sshConfig.count("host") && sshConfig.count("username") && sshConfig.count("password")) {
        int port = 22;
        if (sshConfig.count("port")) {
            try {
                port = std::stoi(sshConfig.at("port"));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Warning: Invalid SSH port specified: " << sshConfig.at("port") << ". Using default port 22." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Warning: SSH port out of range: " << sshConfig.at("port") << ". Using default port 22." << std::endl;
            }
        }
        return establishSSHConnection(sshConfig.at("host"), port, sshConfig.at("username"), sshConfig.at("password"));
    } else {
        std::cerr << "Warning: Incomplete SSH configuration provided for step." << std::endl;
        return false;
    }
}

void Pipeline::closeSSHConnection() {
    if (sshSession) {
        if (sshChannel) {
            libssh2_channel_close(sshChannel);
            libssh2_channel_free(sshChannel);
            sshChannel = nullptr;
        }
        libssh2_session_disconnect(sshSession, "Closing session");
        libssh2_session_free(sshSession);
        // socket gone?
        sshSession = nullptr;
        std::cout << "SSH connection closed." << std::endl;
    }
}

LIBSSH2_SESSION* Pipeline::getSSHSession() const {
    return sshSession;
}

bool Pipeline::execute() {
    std::cout << "Executing pipeline: " << config_.name << " in " << workingDir_ << std::endl;

    for (const auto& step : config_.steps) {
        std::cout << "\n--- Executing step: " << step.name << " ---" << std::endl;
        if (!executeStep(step)) {
            std::cerr << "Step '" << step.name << "' failed." << std::endl;
            return false;
        }
    }

    std::cout << "\n--- Pipeline execution finished ---" << std::endl;
    return true;
}

bool Pipeline::executeStep(const Step& step) {

    if (step.run.empty()) {
        std::cerr << "Warning: 'run' command not defined for step '" << step.name << "'." << std::endl;
        return true;
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("fork failed");
    } else if (pid == 0) {
        std::stringstream ss(step.run);
        std::string segment;
        std::vector<char*> args;
        while (std::getline(ss, segment, ' ')) {
            args.push_back(strdup(segment.c_str())); // might need to free these
        }
        args.push_back(nullptr);

        if (!workingDir_.empty() && chdir(workingDir_.c_str()) != 0) {
            perror("chdir");
            exit(EXIT_FAILURE);
        }

        execvp(args[0], args.data());
        perror("execvp");
        exit(EXIT_FAILURE);
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0);
        bool success = false;

        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus == 0) {
                std::cout << "Step '" << step.name << "' completed successfully." << std::endl;
                handleStepActions(step.on_success, step.name, "");
                success = true;
            } else {
                std::cerr << "Step '" << step.name << "' failed with exit code: " << exitStatus << std::endl;
                handleStepActions(step.on_failure, step.name, "");
            }
        } else if (WIFSIGNALED(status)) {
            std::cerr << "Step '" << step.name << "' terminated by signal: " << WTERMSIG(status) << std::endl;
        } else {
            std::cerr << "Step '" << step.name << "' had an unexpected termination." << std::endl;
        }

        return success;
    }
}

void Pipeline::handleStepActions(const std::vector<std::map<std::string, std::string>>& actions, const std::string& stepName, const std::string& output) {
    for (const auto& action : actions) {
        if (action.count("log")) {
            std::cout << "  [" << stepName << "] Log: " << action.at("log") << std::endl;
        } else if (action.count("bash")) {
            std::string bashCommand = action.at("bash");
            std::cout << "  [" << stepName << "] Executing bash action: " << bashCommand << std::endl;
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
            } else if (pid == 0) {
                execl("/bin/bash", "bash", "-c", bashCommand.c_str(), nullptr);
                perror("execl");
                exit(EXIT_FAILURE);
            } else {
                int status;
                waitpid(pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    std::cerr << "  [" << stepName << "] Bash action failed with exit code: " << WEXITSTATUS(status) << std::endl;
                }
            }
        } else if (action.count("file_output")) {
            std::cout << "  [" << stepName << "] File output: " << action.at("file_output") << std::endl;
        } else if (action.count("notify")) {
            std::cout << "  [" << stepName << "] Notification: " << action.at("notify") << std::endl;
        } else if (action.count("ssh_command")) {
            if (sshSession) {
                std::string sshCommand = action.at("ssh_command");
                std::cout << "  [" << stepName << "] Executing SSH command: " << sshCommand << std::endl;
                LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(sshSession);
                if (channel) {
                    if (libssh2_channel_exec(channel, sshCommand.c_str()) == 0) {
                        // thanks mr. stackoverflow & libssh docs
                        char buffer[0x4000];
                        ssize_t nbytes;
                        while ((nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer))) > 0) {
                            fwrite(buffer, 1, nbytes, stdout);
                        }
                        int exitcode = 0;
                        if (libssh2_channel_close(channel) == 0) {
                            exitcode = libssh2_channel_get_exit_status(channel);
                            if (exitcode != 0) {
                                std::cerr << "  [" << stepName << "] SSH command exited with code: " << exitcode << std::endl;
                            }
                        }
                        libssh2_channel_free(channel);
                    } else {
                        char* err_msg;
                        int err_code;
                        libssh2_session_last_error(sshSession, &err_msg, &err_code, 0);
                        std::cerr << "  [" << stepName << "] Error executing SSH command: " << err_msg << " (" << err_code << ")" << std::endl;
                    }
                } else {
                    std::cerr << "  [" << stepName << "] Error opening SSH channel." << std::endl;
                }
            } else {
                std::cerr << "  [" << stepName << "] SSH session not established. Cannot execute ssh_command." << std::endl;
            }
        } else if (action.count("deploy_files")) {
            if (sshSession) {
                std::string remote_path = action.at("deploy_files");
                std::cout << "  [" << stepName << "] Preparing to deploy files to: " << remote_path << std::endl;
                // todo: implement libssh2_scp_send
                std::cerr << "  [" << stepName << "] Warning: 'deploy_files' action is not yet implemented." << std::endl;
            } else {
                std::cerr << "  [" << stepName << "] SSH session not established. Cannot deploy files." << std::endl;
            }
        }
         else if (action.count("establish_ssh")) {
            establishSSHConnection(action);
        }
        else {
            std::cout << "  [" << stepName << "] Unknown action." << std::endl;
        }
    }
}