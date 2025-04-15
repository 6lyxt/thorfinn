#!/bin/bash


SOURCE_FILES="src/main.cpp src/config.cpp src/pipeline.cpp"
OUTPUT_EXECUTABLE="thorfinn"
INCLUDE_DIR="/opt/homebrew/Cellar/yaml-cpp/0.8.0/include"
LIBRARY_DIR="/opt/homebrew/Cellar/yaml-cpp/0.8.0/lib"
YAML_CPP_LIB="yaml-cpp"

g++ -std=c++17 $SOURCE_FILES -o $OUTPUT_EXECUTABLE -I"$INCLUDE_DIR" -L"$LIBRARY_DIR" -l"$YAML_CPP_LIB"

if [ $? -eq 0 ]; then
  echo "Compilation successful. Executable created: $OUTPUT_EXECUTABLE"
else
  echo "Compilation failed. Please check the error messages."
fi