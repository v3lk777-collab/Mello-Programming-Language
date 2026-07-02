// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once
#include <thread>
#include <vector>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
    const std::string ARDUINO_CLI_PATH = "..\\bin\\win32\\arduino-cli.exe";
    const std::string CLANG_FORMAT_PATH = "..\\bin\\win32\\clang-format.exe";
#elif defined(__APPLE__)
    const std::string ARDUINO_CLI_PATH = "../bin/mac/arduino-cli";
    const std::string CLANG_FORMAT_PATH = "../bin/mac/clang-format";
#else
    const std::string ARDUINO_CLI_PATH = "../bin/linux/arduino-cli";
    const std::string CLANG_FORMAT_PATH = "../bin/linux/clang-format";
#endif

std::string getComputerCoreNumber() {
    unsigned int coreCount = std::thread::hardware_concurrency();
    if (coreCount == 0)
        coreCount = 4; 

    return std::to_string(coreCount);
}

std::filesystem::path getTempSketchDir() {
    return std::filesystem::temp_directory_path() / "output";
}

bool installLibraries() {
    if (includedLibraries.empty()) {
        return true; 
    }

    std::cout << "Checking and installing required libraries..." << std::endl;
    
    for (const auto& lib : includedLibraries) {        
        std::string installLibrariesCommand = ARDUINO_CLI_PATH + " lib install \"" + lib + "\"";
        
        int status = system(installLibrariesCommand.c_str());
        
        if (status != 0) {
            std::cerr << "Warning: Failed to install '" << lib << "'. It might be built-in or the name is incorrect." << std::endl;
        }
    }
    
    std::cout << "Finished checking libraries." << std::endl;
    return true;
}

bool compileCode() {
    std::cout << "Starting code compilation..." << std::endl;
    
    std::string compileCommand = ARDUINO_CLI_PATH + " compile --fqbn arduino:avr:uno" + " --build-path \"" + getTempSketchDir().string() + "/build_cache\"" + " --jobs " + getComputerCoreNumber() + " --build-property build.extra_flags=\"-O3 -flto\"" + " \"" + getTempSketchDir().string() + "\"";
    int compileStatus = system(compileCommand.c_str());
    
    if (compileStatus == 0) {
        std::cout << "Compilation successful!" << std::endl;
        return true;
    } else {
        std::cerr << "Compilation failed!" << std::endl;
        return false;
    }
}

bool uploadCode() {
    std::cout << "Searching for connected Arduino boards..." << std::endl;
    std::filesystem::path portsFilePath = std::filesystem::temp_directory_path() / "ports.txt";
    std::string listCommand = ARDUINO_CLI_PATH + " board list --format json > \"" + portsFilePath.string() + "\"";
    system(listCommand.c_str());

    std::ifstream portsFile(portsFilePath.string());
    std::string line;
    std::string detectedPort = "";

    if (portsFile.is_open()) {
        while (std::getline(portsFile, line)) {
            size_t addressPos = line.find("\"address\":");
            if (addressPos != std::string::npos) {
                size_t start = line.find("\"", addressPos + 10);
                if (start != std::string::npos) {
                    size_t end = line.find("\"", start + 1);
                    if (end != std::string::npos) {
                        detectedPort = line.substr(start + 1, end - start - 1);
                        break; 
                    }
                }
            }
        }
        portsFile.close();
        std::filesystem::remove(portsFilePath); 
    }

    if (detectedPort.empty()) {
        std::cerr << "No board detected! Please connect your Arduino." << std::endl;
        return false;
    }

    std::cout << "Found Arduino on port: " << detectedPort << std::endl;
    std::cout << "Uploading code to the board..." << std::endl;

    std::string uploadCommand = ARDUINO_CLI_PATH + " upload -p " + detectedPort + " --fqbn arduino:avr:uno" + " --build-path \"" + getTempSketchDir().string() + "/build_cache\"" + " \"" + getTempSketchDir().string() + "\"";
    int uploadStatus = system(uploadCommand.c_str());
    
    if (uploadStatus == 0) {
        std::cout << "Upload successful!" << std::endl;
        return true;
    } else {
        std::cerr << "Upload failed! Check connection." << std::endl;
        return false;
    }
}