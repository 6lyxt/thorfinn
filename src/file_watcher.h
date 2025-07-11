#ifndef THORFINN_FILE_WATCHER_H
#define THORFINN_FILE_WATCHER_H

#include <string>
#include <functional>
#include <map>
#include <filesystem>

namespace Thorfinn {
namespace FileWatcher {

enum class FileSystemEventType {
    Modified,
    Created,
    Deleted
};

using FileSystemChangeCallback = std::function<void(FileSystemEventType, const std::string&)>;

void watchDirectory(const std::string& directoryPath, FileSystemChangeCallback callback);
void watchFile(const std::string& filePath, FileSystemChangeCallback callback); // todo: implement file_change type

}}

#endif