#!/bin/bash

SOURCE_FILES="src/main.cpp src/config.cpp src/pipeline.cpp src/file_watcher.cpp"
OUTPUT_EXECUTABLE="thorfinn"

YAML_CPP_INCLUDE_DIR="/opt/homebrew/Cellar/yaml-cpp/0.8.0/include"
YAML_CPP_LIBRARY_DIR="/opt/homebrew/Cellar/yaml-cpp/0.8.0/lib"
YAML_CPP_LIB="yaml-cpp"

LIBSSH2_INCLUDE_DIR="/opt/homebrew/Cellar/libssh2/1.11.1/include"
LIBSSH2_LIBRARY_DIR="/opt/homebrew/Cellar/libssh2/1.11.1/lib"
LIBSSH2_LIB="ssh2"

g++ -std=c++17 $SOURCE_FILES -o $OUTPUT_EXECUTABLE \
  -I"$YAML_CPP_INCLUDE_DIR" -L"$YAML_CPP_LIBRARY_DIR" -l"$YAML_CPP_LIB" \
  -I"$LIBSSH2_INCLUDE_DIR" -L"$LIBSSH2_LIBRARY_DIR" -l"$LIBSSH2_LIB" \

if [ $? -eq 0 ]; then
  echo "Compilation successful. Executable created: $OUTPUT_EXECUTABLE"
else
  echo "Compilation failed. Please check the error messages."
fi