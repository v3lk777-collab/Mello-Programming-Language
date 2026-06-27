// Mello Programming Language
// Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.
//
// This source code is private and protected by intellectual property laws.
// Unauthorized use, modification, or distribution for any competitive 
// academic or commercial purpose is strictly prohibited without 
// explicit written permission from the author.

#include "lexer.hpp"
#include "parser.hpp"
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
    std::string copileCommand = ARDUINO_CLI_PATH + " compile --fqbn arduino:avr:uno \"" + getTempSketchDir().string() + "\"";
    int compileStatus = system(copileCommand.c_str());
    
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

    std::string uploadCommand = ARDUINO_CLI_PATH + " upload -p " + detectedPort + " --fqbn arduino:avr:uno \"" + getTempSketchDir().string() + "\"";
    int uploadStatus = system(uploadCommand.c_str());
    
    if (uploadStatus == 0) {
        std::cout << "Upload successful!" << std::endl;
        return true;
    } else {
        std::cerr << "Upload failed! Check connection." << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    std::string fileName;

    if (argc > 1) {
        fileName = argv[1];
    }

    if (fileName.empty()) {
        std::cout << "Enter the Mello file name: ";
        std::cin >> fileName;
    }

    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << fileName << std::endl;
        return 1;
    }

    std::string sourceCode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::vector<std::unique_ptr<ASTNode>> program = parser.parse();
    
    std::filesystem::path sketchDir = getTempSketchDir();
    std::filesystem::create_directories(sketchDir);
    std::filesystem::path inoFilePath = sketchDir / "output.ino";

    std::ofstream outputFile(inoFilePath.string());
    if (!outputFile.is_open()) {
        std::cerr << "Error: Could not open output file." << std::endl;
        return 1;
    }

    bool hasSetup = false;
    bool hasLoop = false;
    bool hasSerial = false;
    bool hasSerialCommand = false;

    std::string includesCode = "";
    std::string globalVariablesCode = "";
    std::string functionsCode = "";
    std::string author = "// Generated by Mello Programming Language\n"
                         "// Copyright (c) 2026 Mohammed Tamer Mohammed Ahmed El-Azab - V3LK\n"
                         "// Full Open-Source project on: https://github.com/v3lk777-collab/Mello";

    for (const auto& node : program) {
        if (auto varNode = dynamic_cast<VarAssignNode*>(node.get())) {
            std::string code = varNode->toCpp();

            if (code.find(varNode->name + " =") == 0) {
                std::cerr << "\nSyntax Error: You cannot reassign the variable '" 
                          << varNode->name << "' outside of a function.\n"
                          << "Please move '" << varNode->name << " = ...' inside 'func start():' or 'func loop():'.\n" 
                          << std::endl;
                
                outputFile.close();
                
                std::filesystem::remove_all(sketchDir);

                return EXIT_FAILURE; 
            } else {
                globalVariablesCode += code;
            }
        } else {
            std::string code = node->toCpp();
            
            if (code.find("void setup()") != std::string::npos) {
                hasSetup = true;
            }

            if ((code.find("Serial.print") != std::string::npos) || (code.find("Serial.println") != std::string::npos) || (code.find("Serial.available") != std::string::npos) || (code.find("Serial.read") != std::string::npos)) {
                hasSerialCommand = true;
            }

            if (code.find("Serial.begin") != std::string::npos) {
                hasSerial = true;
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

            beginArgs.push_back(std::make_unique<LiteralNode>(Token(TokenType::NUMBER, "9600")));
            setupBody.push_back(std::make_unique<FunctionCallNode>("Serial.begin", std::move(beginArgs)));
        }

        FunctionNode autoSetup("setup", std::move(setupBody));
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
        FunctionNode autoLoop("loop", {});
        outputFile << autoLoop.toCpp();
    }

    outputFile.close();

    if (!std::filesystem::exists(ARDUINO_CLI_PATH)) {
        std::cerr << "Error: arduino-cli.exe not found at the specified path!" << std::endl;
        return EXIT_FAILURE;
    }

    if (std::filesystem::exists(CLANG_FORMAT_PATH)) {
        std::string formatCommand = CLANG_FORMAT_PATH + " --style=Google -i \"" + inoFilePath.string() + "\"";
        std::system(formatCommand.c_str());
    } else {
        std::cerr << "Warning: clang-format not found locally, skipping formatting." << std::endl;
    }

    // Delete when u finish from here

    std::string sketchCode;

    std::ifstream sketchCodeFile(inoFilePath.string());

    if (sketchCodeFile.is_open()) {
        while (getline(sketchCodeFile, sketchCode)) {
            std::cout << sketchCode << std::endl;
        }
    } else {
        std::cerr << "Warning: Cann't open the sketch code file." << std::endl;
    }

    sketchCodeFile.close();

    // to here

    installLibraries();

    bool compiledSuccessfully = compileCode();

    if (compiledSuccessfully && argc >= 3 && std::string(argv[2]) == "--upload") {
        uploadCode();
    }

    std::filesystem::remove_all(sketchDir);

    return EXIT_SUCCESS;
}