// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#pragma once

#include "lexer.hpp"
#include "parser.hpp"

#include <thread>
#include <vector>
#include <cstdio>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
    #define ARDUINO_CLI_DEFAULT "..\\bin\\win32\\arduino-cli.exe"
    #define CLANG_FORMAT_DEFAULT "..\\bin\\win32\\clang-format.exe"
#elif defined(__APPLE__)
    #define ARDUINO_CLI_DEFAULT "../bin/mac/arduino-cli"
    #define CLANG_FORMAT_DEFAULT "../bin/mac/clang-format"
#else
    #define ARDUINO_CLI_DEFAULT "../bin/linux/arduino-cli"
    #define CLANG_FORMAT_DEFAULT "../bin/linux/clang-format"
#endif

const std::string ARDUINO_CLI_PATH = ARDUINO_CLI_DEFAULT;
const std::string CLANG_FORMAT_PATH = CLANG_FORMAT_DEFAULT;

std::string getComputerCoreNumber() {
    unsigned int coreCount = std::thread::hardware_concurrency();

    if (coreCount == 0) {
        coreCount = 4;
    }

    return std::to_string(coreCount);
}

std::filesystem::path getTempSketchDir() {
    return std::filesystem::temp_directory_path() / "output";
}

bool installLibraries() {
    if (includedLibraries.empty()) {
        return true; 
    }

    std::cout << "Checking and installing required libraries..." << "\n";
    
    for (const auto& lib : includedLibraries) {
        if (!lib.starts_with("avr")) {
            std::string installLibrariesCommand = ARDUINO_CLI_PATH + " lib install \"" + lib + "\"";
        
            int status = system(installLibrariesCommand.c_str());
            
            if (status != 0) {
                std::cerr << "Warning: Failed to install '" << lib << "'. It might be built-in or the name is incorrect." << "\n";
            }
        }
    }
    
    std::cout << "Finished checking libraries." << "\n";
    return true;
}

bool compileCode() {
    std::cout << "Starting code compilation..." << "\n";
    
    std::string compileCommand = ARDUINO_CLI_PATH + " compile --fqbn arduino:avr:uno" + " --build-path \"" + getTempSketchDir().string() + "/build_cache\"" + " --jobs " + getComputerCoreNumber() + " --build-property build.extra_flags=\"-O3 -flto\"" + " \"" + getTempSketchDir().string() + "\"";
    int compileStatus = system(compileCommand.c_str());
    
    if (compileStatus == 0) {
        std::cout << "Compilation successful!" << "\n";
        return true;
    } else {
        std::cerr << "Compilation failed!" << "\n";
        return false;
    }
}

bool uploadCode() {
    std::cout << "Searching for connected Arduino boards..." << "\n";
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
        std::cerr << "No board detected! Please connect your Arduino." << "\n";
        return false;
    }

    std::cout << "Found Arduino on port: " << detectedPort << "\n";
    std::cout << "Uploading code to the board..." << "\n";

    std::string uploadCommand = ARDUINO_CLI_PATH + " upload -p " + detectedPort + " --fqbn arduino:avr:uno" + " --build-path \"" + getTempSketchDir().string() + "/build_cache\"" + " \"" + getTempSketchDir().string() + "\"";
    int uploadStatus = system(uploadCommand.c_str());
    
    if (uploadStatus == 0) {
        std::cout << "Upload successful!" << "\n";
        return true;
    } else {
        std::cerr << "Upload failed! Check connection." << "\n";
        return false;
    }
}

bool runMelloCompiler(int argc, char* argv[]) {
    std::string filePath;

    if (argc > 1) {
        filePath = argv[1];
    }

    if (filePath.empty()) {
        std::cout << "Enter the Mello file path: ";
        std::cin >> filePath;
    }

    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePath << "\n";
        return false;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();
    
    std::vector<std::unique_ptr<ASTNode>> program;

    try {
        Parser parser(tokens, sourceCode);
        program = parser.parse();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";
        return false;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return false;
    }

    std::filesystem::path sketchDir = getTempSketchDir();
    std::filesystem::create_directories(sketchDir);
    std::filesystem::path inoFilePath = sketchDir / "output.ino";

    std::ofstream outputFile(inoFilePath.string());

    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file." << "\n";
        return false;
    }

    bool hasSetup = false;
    bool hasLoop = false;
    bool hasSerial = false;
    bool hasSerialCommand = false;

    std::string includesCode = "";
    std::string globalVariablesCode = "";
    std::string functionsCode = "";
    std::string author = "// Generated by Mello Programming Language\n"
                         "// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.\n"
                         "//\n"
                         "// This source code is private and protected by intellectual property laws\n"
                         "// Unauthorized use, modification, or distribution for any competitive\n"
                         "// academic or commercial purpose is strictly prohibited without\n"
                         "// explicit written permission from the author";

    for (const auto& node : program) {
        if (auto varNode = dynamic_cast<VarAssignNode*>(node.get())) {
            std::string code = varNode->toCpp();

            if (code.find(varNode->name + " =") == 0) {
                std::cerr << "\nSyntax Error: You cannot reassign the variable '" 
                          << varNode->name << "' outside of a function.\n"
                          << "Please move '" << varNode->name << " = ...' inside 'func start():' or 'func loop():'.\n" 
                          << "\n";
                
                outputFile.close();
                
                std::filesystem::remove_all(sketchDir);

                return false; 
            } else {
                globalVariablesCode += code;
            }
        } else {
            std::string code = node->toCpp();
            
            if (code.find("void setup()") != std::string::npos) {
                hasSetup = true;
            }

            if (code.find("Serial.begin") != std::string::npos) {
                hasSerial = true;
            }

            if ((code.find("Serial.") != std::string::npos) && code.find("Serial.begin") == std::string::npos) {
                hasSerialCommand = true;
            }

            if (code.find("void loop()") != std::string::npos) {
                hasLoop = true;
            }

            functionsCode += code + "\n";
        }
    }

    if (!includedLibraries.empty()) {
        for (const auto& lib : includedLibraries) {
            includesCode += "#include <" + lib + ".h>\n";
        }
    }

    outputFile << author << "\n";
    outputFile << includesCode << "\n";
    outputFile << globalVariablesCode << "\n";

    if (!hasSetup) {
        std::vector<std::unique_ptr<ASTNode>> setupBody;

        if (!hasSerial && hasSerialCommand) {
            std::vector<std::unique_ptr<ExpressionNode>> beginArgs;

            beginArgs.push_back(std::make_unique<LiteralNode>(Token(TokenType::NUMBER, "9600", 0)));
            setupBody.push_back(std::make_unique<FunctionCallNode>("Serial.begin", std::move(beginArgs), 0, ""));
        }

        FunctionNode autoSetup("setup", std::move(setupBody), 0, "");
        outputFile << autoSetup.toCpp() << "\n";
    } else if (!hasSerial && hasSerialCommand) {
        size_t pos = functionsCode.find("void setup() {");

        if (pos != std::string::npos) {
            size_t insertPos = functionsCode.find('\n', pos) + 1;
            functionsCode.insert(insertPos, "Serial.begin(9600);\n");
        }
    }

    outputFile << functionsCode << "\n";

    if (!hasLoop) {
        FunctionNode autoLoop("loop", {}, 0, "");
        outputFile << autoLoop.toCpp();
    }

    outputFile.close();

    if (!std::filesystem::exists(ARDUINO_CLI_PATH)) {
        std::cerr << "Error: arduino-cli.exe not found at the specified path" << "\n";
        return false;
    }

    if (std::filesystem::exists(CLANG_FORMAT_PATH)) {
        std::string formatCommand = CLANG_FORMAT_PATH + " --style=Google -i \"" + inoFilePath.string() + "\"";
        std::system(formatCommand.c_str());
    } else {
        std::cerr << "Warning: clang-format not found locally, skipping formatting" << "\n";
    }

    // Delete when u finish from here
    // But wait, why are u reaing my code
    // yk, if u steal it, i will kill u
    // so btw, dont steal it

    std::string sketchCode;

    std::ifstream sketchCodeFile(inoFilePath.string());

    std::cout << "-------------------------------------------------------------------------------------\n";
    if (sketchCodeFile.is_open()) {
        while (getline(sketchCodeFile, sketchCode)) {
            std::cout << "| " << std::left << std::setw(82) << sketchCode << "|\n";
        }
    } else {
        std::cerr << "Warning: Cann't open the sketch code file" << "\n";
    }

    std::cout << "-------------------------------------------------------------------------------------\n\n";

    sketchCodeFile.close();

    // to here

    installLibraries();

    bool compiledSuccessfully = compileCode();

    bool isUpload = false;
    bool isCompileCode = true;
    bool isSaveSketchCodeDir = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--upload") {
            isUpload = true;
        } else if (arg == "--save-code") {
            isSaveSketchCodeDir = true;
        } else if (arg == "--no-compile") {
            isCompileCode = false;
        } else {
            std::cerr << "Warning: There is not arg called '" << arg << "'\n";
        }
    }

    if (compiledSuccessfully && isUpload) {
        uploadCode();
    }

    if (!isSaveSketchCodeDir) {
        std::filesystem::remove_all(sketchDir);
    } else {
        std::cout << "Transpiled C++ code saved inside: " << sketchDir << "\n";
    }

    return true;
}