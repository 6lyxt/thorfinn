#ifndef THORFINN_CONFIG_H
#define THORFINN_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <yaml-cpp/yaml.h>

struct Step {
    std::string name;
    std::string run;
    std::vector<std::string> dependencies;
    std::vector<std::map<std::string, std::string>> on_success;
    std::vector<std::map<std::string, std::string>> on_failure;
    std::map<std::string, std::string> ssh_config;
};

struct SSHGlobalConfig {
    std::string host;
    int port = 22;
    std::string username;
    std::string password; // not prod ready
};

struct Config {
    std::string name;
    std::string description;
    std::vector<std::map<std::string, std::string>> triggers;
    std::vector<Step> steps;
    std::vector<std::map<std::string, std::string>> results;
    SSHGlobalConfig ssh_global_config;

    static Config loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& filepath) const;
};

#endif