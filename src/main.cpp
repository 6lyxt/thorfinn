#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include "config.h"
#include "pipeline.h"

namespace fs = std::filesystem;

const std::string DEFAULT_CONFIG_CONTENT = R"(
name: MyPipeline
description: A basic thorfinn pipeline
triggers:
  - type: manual
    description: Execute manually via 'thorfinn exec'
steps:
  - name: Example Step
    run: echo 'Askeladd?'
results: []
)";

void printThorfinnAscii() {
    std::cout << R"(   ▄▄▄▄▀ ▄  █ ████▄ █▄▄▄▄ ▄████  ▄█    ▄      ▄   
▀▀▀ █   █   █ █   █ █  ▄▀ █▀   ▀ ██     █      █  
    █   ██▀▀█ █   █ █▀▀▌  █▀▀    ██ ██   █ ██   █ 
   █    █   █ ▀████ █  █  █      ▐█ █ █  █ █ █  █ 
  ▀        █          █    █      ▐ █  █ █ █  █ █ 
          ▀          ▀      ▀       █   ██ █   ██ 
                                                  )" << std::endl;
}

void createDefaultConfig(const std::string& directory) {
    fs::path configPath = fs::path(directory) / "thorfinn.yaml";
    if (!fs::exists(configPath)) {
        std::ofstream configFile(configPath);
        if (configFile.is_open()) {
            configFile << DEFAULT_CONFIG_CONTENT;
            configFile.close();
            std::cout << "Created default thorfinn.yaml in: " << directory << std::endl;
        } else {
            std::cerr << "Error creating thorfinn.yaml in: " << directory << std::endl;
        }
    } else {
        std::cout << "thorfinn.yaml already exists in: " << directory << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2 && std::string(argv[1]) == "make") {
        printThorfinnAscii();
        createDefaultConfig(fs::current_path().string());
    } else if (argc >= 2 && std::string(argv[1]) == "exec") {
        std::string directory = (argc > 2) ? argv[2] : fs::current_path().string();
        Config config = Config::loadFromFile(fs::path(directory) / "thorfinn.yaml");
        if (!config.steps.empty() || !config.triggers.empty() || !config.results.empty() || !config.name.empty() || !config.description.empty()) {
            Pipeline pipeline(config, directory);
            pipeline.execute();
        } else {
            std::cerr << "Error: Could not load pipeline configuration from " << fs::path(directory) / "thorfinn.yaml" << std::endl;
        }

    // todo: implement pipeline on event / automatic    
    } else {
        std::cout << "Usage: thorfinn <command> [directory]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  make                 Prepares a pipeline for the current directory." << std::endl;
        std::cout << "  exec [directory]     Executes a pipeline in the specified directory (default: current)." << std::endl;
    }

    return 0;
}