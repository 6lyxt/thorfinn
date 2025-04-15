#include "pipeline.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

Pipeline::Pipeline(const Config& config, const std::string& workingDir) : config_(config), workingDir_(workingDir) {}

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

        if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            if (exitStatus == 0) {
                std::cout << "Step '" << step.name << "' completed successfully." << std::endl;
                if (!step.on_success.empty()) {
                    handleStepActions(step.on_success, step.name, "");
                }
                return true;
            } else {
                std::cerr << "Step '" << step.name << "' failed with exit code: " << exitStatus << std::endl;
                // todo: capture err
                if (!step.on_failure.empty()) {
                    handleStepActions(step.on_failure, step.name, "");
                }
                return false;
            }
        } else if (WIFSIGNALED(status)) {
            std::cerr << "Step '" << step.name << "' terminated by signal: " << WTERMSIG(status) << std::endl;
            return false;
        } else {
            std::cerr << "Step '" << step.name << "' had an unexpected termination." << std::endl;
            return false;
        }
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
        } else {
            std::cout << "  [" << stepName << "] Unknown action." << std::endl;
        }
    }
}