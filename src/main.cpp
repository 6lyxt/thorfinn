#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include "config.h"
#include "pipeline.h"
#include "file_watcher.h"

namespace fs = std::filesystem;

const std::string DEFAULT_CONFIG_CONTENT = R"(
name: MyPipeline
description: A basic thorfinn pipeline
ssh_global_config:
  host: "your_remote_host"
  port: 22
  username: "your_username"
  password: "your_password" # Warning: Insecure for production!
triggers:
  - type: manual
    description: Execute manually via 'thorfinn exec'
on_event:
  - type: file_change
    description: Trigger when any file in /tmp/thorfinn_watch_dir changes
    directory: /tmp/thorfinn_watch_dir # Changed to directory
  - type: interval
    description: Trigger every 5 seconds
    seconds: 5
steps:
  - name: Example Step (File Change or Interval)
    run: echo 'Askeladd on event!'
    on_success:
      - log: "Example step succeeded (on event)."
results: []
)";

void printThorfinnAscii() {
    std::cout << R"(    ▄▄▄▄▀ ▄  █ ████▄ █▄▄▄▄ ▄████  ▄█    ▄      ▄
▀▀▀ █    █   █ █    █ █ ▄▀  █▀    ▀ ██      █      █
    █    ██▀▀█ █    █ █▀▀▌  █▀▀    ██ ██   █ ██   █
   █     █   █ ▀████ █ █   █      ▐█ █ █  █ █ █  █
  ▀       █          █   █   █      ▐ █  █ █ █  █ █
          ▀          ▀     ▀     ▀   █   ██ █   ██
                                        █      █
                                        ▀      ▀ )" << std::endl;
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

void handleEvent(const Config& config, const std::string& workingDir) {
    Pipeline pipeline(config, workingDir);
    pipeline.execute();
}

void eventLoop(const Config& config, const std::string& workingDir) {
    std::cout << "Thorfinn is listening for events..." << std::endl;
    for (const auto& event_trigger : config.on_event) {
        if (event_trigger.type == "file_change") {
            try {
                std::string directoryToWatch = event_trigger.config.at("path");
                if (!fs::exists(directoryToWatch)) {
                    std::cerr << "Error: Directory not watchable" << std::endl;
                }

                Thorfinn::FileWatcher::watchDirectory(directoryToWatch, [&](Thorfinn::FileWatcher::FileSystemEventType eventType, const std::string& changedPath) {
                    std::string eventTypeStr;
                    switch (eventType) {
                        case Thorfinn::FileWatcher::FileSystemEventType::Modified: eventTypeStr = "Modified"; break;
                        case Thorfinn::FileWatcher::FileSystemEventType::Created:  eventTypeStr = "Created";  break;
                        case Thorfinn::FileWatcher::FileSystemEventType::Deleted:  eventTypeStr = "Deleted";  break;
                        default: eventTypeStr = "Unknown"; break;
                    }
                    std::cout << "File system event detected: " << eventTypeStr << " - " << changedPath << std::endl;
                    handleEvent(config, workingDir);
                });
                std::cout << "Watching directory: " << directoryToWatch << " for changes..." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Error: 'path' key not found in file_change event configuration." << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error setting up watcher for " << event_trigger.config.at("path") << ": " << e.what() << std::endl;
            }
        } else if (event_trigger.type == "interval") {
            try {
                int seconds = std::stoi(event_trigger.config.at("seconds"));
                std::thread([seconds, config, workingDir]() {
                    while (true) {
                        std::this_thread::sleep_for(std::chrono::seconds(seconds));
                        std::cout << "Interval event triggered." << std::endl;
                        handleEvent(config, workingDir);
                    }
                }).detach();
                std::cout << "Interval trigger set for every " << seconds << " seconds..." << std::endl;
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid 'seconds' value in interval event." << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Error: 'seconds' value out of range in interval event." << std::endl;
            }
        } else if (event_trigger.type == "webhook") {
            std::string endpoint = event_trigger.config.at("endpoint");
            std::string method = event_trigger.config.at("method");
            std::cerr << "Warning: Webhook event handling is not yet implemented." << std::endl;
        } else {
            std::cerr << "Warning: Unknown event type: " << event_trigger.type << std::endl;
        }
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
    } else if (argc >= 2 && std::string(argv[1]) == "listen") {
        std::string directory = (argc > 2) ? argv[2] : fs::current_path().string();
        Config config = Config::loadFromFile(fs::path(directory) / "thorfinn.yaml");
        if (!config.on_event.empty()) {
            eventLoop(config, directory);
        } else {
            std::cout << "No 'on_event' triggers defined in thorfinn.yaml. Nothing to listen for." << std::endl;
        }
    } else {
        std::cout << "Usage: thorfinn <command> [directory]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  make                   Prepares a pipeline for the current directory." << std::endl;
        std::cout << "  exec [directory]       Executes a pipeline in the specified directory (default: current)." << std::endl;
        std::cout << "  listen [directory]     Listens for events to trigger the pipeline (default: current)." << std::endl;
    }

    return 0;
}