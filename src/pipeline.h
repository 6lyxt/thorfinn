#ifndef THORFINN_PIPELINE_H
#define THORFINN_PIPELINE_H

#include "config.h"
#include <string>

class Pipeline {
public:
    Pipeline(const Config& config, const std::string& workingDir);
    bool execute();

private:
    const Config& config_;
    std::string workingDir_;
    bool executeStep(const Step& step);
    void handleStepActions(const std::vector<std::map<std::string, std::string>>& actions, const std::string& stepName, const std::string& output);
};

#endif