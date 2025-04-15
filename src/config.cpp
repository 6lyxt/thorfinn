#include "config.h"
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

Config Config::loadFromFile(const std::string& filepath) {
    Config config;
    try {
        YAML::Node root = YAML::LoadFile(filepath);

        if (root["name"]) config.name = root["name"].as<std::string>();
        if (root["description"]) config.description = root["description"].as<std::string>();

        if (root["triggers"] && root["triggers"].IsSequence()) {
            for (const auto& trigger : root["triggers"]) {
                config.triggers.push_back(trigger.as<std::map<std::string, std::string>>());
            }
        }

        if (root["steps"] && root["steps"].IsSequence()) {
            for (const auto& step_node : root["steps"]) {
                Step step;
                if (step_node["name"]) step.name = step_node["name"].as<std::string>();
                if (step_node["run"]) step.run = step_node["run"].as<std::string>();
                if (step_node["dependencies"] && step_node["dependencies"].IsSequence()) {
                    for (const auto& dep : step_node["dependencies"]) {
                        step.dependencies.push_back(dep.as<std::string>());
                    }
                }
                if (step_node["on_success"] && step_node["on_success"].IsSequence()) {
                    for (const auto& action : step_node["on_success"]) {
                        step.on_success.push_back(action.as<std::map<std::string, std::string>>());
                    }
                }
                if (step_node["on_failure"] && step_node["on_failure"].IsSequence()) {
                    for (const auto& action : step_node["on_failure"]) {
                        step.on_failure.push_back(action.as<std::map<std::string, std::string>>());
                    }
                }
                config.steps.push_back(step);
            }
        }

        if (root["results"] && root["results"].IsSequence()) {
            for (const auto& result : root["results"]) {
                config.results.push_back(result.as<std::map<std::string, std::string>>());
            }
        }

    } catch (const YAML::Exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
    }
    return config;
}

bool Config::saveToFile(const std::string& filepath) const {
    try {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "name" << YAML::Value << name;
        out << YAML::Key << "description" << YAML::Value << description;

        out << YAML::Key << "triggers" << YAML::Value << YAML::BeginSeq;
        for (const auto& trigger : triggers) {
            out << trigger;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "steps" << YAML::Value << YAML::BeginSeq;
        for (const auto& step : steps) {
            out << YAML::BeginMap;
            out << YAML::Key << "name" << YAML::Value << step.name;
            out << YAML::Key << "run" << YAML::Value << step.run;
            if (!step.dependencies.empty()) {
                out << YAML::Key << "dependencies" << YAML::Value << YAML::Flow << step.dependencies;
            }
            if (!step.on_success.empty()) {
                out << YAML::Key << "on_success" << YAML::Value << YAML::BeginSeq;
                for (const auto& action : step.on_success) {
                    out << action;
                }
                out << YAML::EndSeq;
            }
            if (!step.on_failure.empty()) {
                out << YAML::Key << "on_failure" << YAML::Value << YAML::BeginSeq;
                for (const auto& action : step.on_failure) {
                    out << action;
                }
                out << YAML::EndSeq;
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::Key << "results" << YAML::Value << YAML::BeginSeq;
        for (const auto& result : results) {
            out << result;
        }
        out << YAML::EndSeq;

        out << YAML::EndMap;

        std::ofstream fout(filepath);
        if (fout.is_open()) {
            fout << out.c_str();
            return true;
        } else {
            std::cerr << "Error opening file for writing: " << filepath << std::endl;
            return false;
        }
    } catch (const YAML::Exception& e) {
        std::cerr << "Error saving config file: " << e.what() << std::endl;
        return false;
    }
}