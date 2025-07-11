#include "file_watcher.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

namespace Thorfinn {
namespace FileWatcher {


std::map<std::string, fs::file_time_type> getCurrentDirectoryState(const std::string& directoryPath) {
    std::map<std::string, fs::file_time_type> current_state;
    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                current_state[entry.path().string()] = fs::last_write_time(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error getting directory state for " << directoryPath << ": " << e.what() << std::endl;
    }
    return current_state;
}


void watchDirectory(const std::string& directoryPath, FileSystemChangeCallback callback) {
    // todo: implement subdirectory watching
    std::thread([directoryPath, callback]() {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Directory not found or is not a directory: " << directoryPath << std::endl;
            return;
        }

        std::map<std::string, fs::file_time_type> last_state = getCurrentDirectoryState(directoryPath);

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::map<std::string, fs::file_time_type> current_state = getCurrentDirectoryState(directoryPath);

            for (const auto& [path, current_time] : current_state) {
                auto it = last_state.find(path);
                if (it == last_state.end()) {
                    std::cout << "Detected file created: " << path << std::endl;
                    if (callback) {
                        callback(FileSystemEventType::Created, path);
                    }
                } else if (it->second != current_time) {
                    std::cout << "Detected file modified: " << path << std::endl;
                    if (callback) {
                        callback(FileSystemEventType::Modified, path);
                    }
                }
            }

            for (const auto& [path, last_time] : last_state) {
                if (current_state.find(path) == current_state.end()) {
                    std::cout << "Detected file deleted: " << path << std::endl;
                    if (callback) {
                        callback(FileSystemEventType::Deleted, path);
                    }
                }
            }

            last_state = current_state;
        }
    }).detach(); 
}
}
}