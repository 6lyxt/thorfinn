#ifndef THORFINN_FILE_WATCHER_H
#define THORFINN_FILE_WATCHER_H

#include <string>
#include <functional>

namespace Thorfinn {
namespace FileWatcher {

using FileChangeCallback = std::function<void(const std::string&)>;

void watchFile(const std::string& filePath, FileChangeCallback callback);

}}

#endif