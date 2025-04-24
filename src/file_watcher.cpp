#include "file_watcher.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

namespace Thorfinn {
namespace FileWatcher {

void watchFile(const std::string& filePath, FileChangeCallback callback) {
    std::thread([filePath, callback]() {
        if (!fs::exists(filePath)) {
            std::cerr << "Error: File not found: " << filePath << std::endl;
            return;
        }

        auto lastWriteTime = fs::last_write_time(filePath);

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            try {
                if (fs::exists(filePath)) {
                    auto currentWriteTime = fs::last_write_time(filePath);
                    if (currentWriteTime != lastWriteTime) {
                        std::cout << "File changed: " << filePath << std::endl;
                        lastWriteTime = currentWriteTime;
                        if (callback) {
                            callback(filePath);
                        }
                    }
                } else {
                    std::cerr << "Warning: File no longer exists: " << filePath << std::endl;
                    break;
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Filesystem error while watching " << filePath << ": " << e.what() << std::endl;
                break;
            }
        }
    }).detach();
}

}
}