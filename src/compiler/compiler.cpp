/*
 * Mello Programming Language

 * Copyright (C) 2026 Mohammed Tamer Mohammed Ahmed El-Azab. All Rights Reserved.

 * This source code is private and protected by intellectual property laws.
 * Unauthorized use, modification, or distribution for any competitive 
 * academic or commercial purpose is strictly prohibited without 
 * explicit written permission from the author.
*/

#include "compiler.hpp"

#include "lexer.hpp"
#include "parser.hpp"
#include "module_loader.hpp"
#include "semantic_analyzer.hpp"

#include <thread>
#include <vector>
#include <random>
#include <format>
#include <cstdint>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <unordered_set>

#ifdef _WIN32
    #define ARDUINO_CLI_DEFAULT  "..\\bin\\win32\\arduino-cli.exe"
    #define CLANG_FORMAT_DEFAULT "..\\bin\\win32\\clang-format.exe"
#elif defined(__APPLE__)
    #define ARDUINO_CLI_DEFAULT  "../bin/mac/arduino-cli"
    #define CLANG_FORMAT_DEFAULT "../bin/mac/clang-format"
#else
    #define ARDUINO_CLI_DEFAULT  "../bin/linux/arduino-cli"
    #define CLANG_FORMAT_DEFAULT "../bin/linux/clang-format"
#endif

const std::string ARDUINO_CLI_PATH = ARDUINO_CLI_DEFAULT;
const std::string CLANG_FORMAT_PATH = CLANG_FORMAT_DEFAULT;

const std::unordered_set<std::string> ARDUINO_SUPPORTED_BOARDS = {
    "uno", "nano"
};

std::string Compiler::getComputerCoreNumber() {
    unsigned int coreCount = std::thread::hardware_concurrency();

    if (coreCount == 0) {
        coreCount = 4;
    }

    return std::to_string(coreCount);
}

std::filesystem::path Compiler::getTempSketchDir() {
    /*
    std::random_device rd;
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<uint64_t> distribution(0, 0xFFFFFFFFFFFFFFFF);

    uint64_t high_bits = distribution(generator);
    uint64_t low_bits = distribution(generator);

    low_bits = (low_bits & 0x3FFFFFFFFFFFFFFF) | 0x8000000000000000;

    high_bits = (high_bits & 0xFFFFFFFFFFFF0FFF) | 0x0000000000004000;

    auto uuid = std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
        (high_bits >> 32),
        ((high_bits >> 16) & 0xFFFF),
        (high_bits & 0xFFFF),
        (low_bits >> 48),
        (low_bits & 0xFFFFFFFFFFFF)
    );
    */

    return std::filesystem::temp_directory_path() / /* uuid */ "output";
}

bool Compiler::installLibraries() {
    if (includedLibraries.empty()) {
        return true; 
    }

    std::string command = ARDUINO_CLI_PATH + " lib list > \"" + (sketchDir / "libs.txt").string() + "\"";
    std::system(command.c_str());

    std::unordered_set<std::string> installedLibraries;

    std::ifstream file(sketchDir / "libs.txt");
    std::string line;

    while (std::getline(file, line)) {
        installedLibraries.insert(line);
    }

    std::cout << "Checking and installing required libraries...\n" << std::flush;
    
    for (const auto& lib : includedLibraries) {
        if (installedLibraries.contains(lib)) {
            continue;
        }

        if (!lib.starts_with("avr") && lib != "EEPROM") {
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

bool Compiler::compileCode() {
    std::cout << "Starting code compilation...\n" << std::flush;

    std::string compileCommand = ARDUINO_CLI_PATH + " compile --fqbn arduino:avr:" + boardType  + " --build-property \"compiler.cpp.extra_flags=-std=gnu++14\" --build-path \"" + sketchDir.string() + "/build_cache\"" + " --jobs " + getComputerCoreNumber() + " --build-property build.extra_flags=\"-O3 -flto\"" + " \"" + sketchDir.string() + "\"";
    int compileStatus = system(compileCommand.c_str());

    if (compileStatus == 0) {
        std::cout << "Compilation successful!" << "\n";

        return true;
    } else {
        std::cerr << "Compilation failed!" << "\n";

        return false;
    }
}

std::string Compiler::findArduinoBoard() {
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
        exit(EXIT_FAILURE);
    }

    return detectedPort;
}

bool Compiler::uploadCode() {
    std::cout << "Searching for connected Arduino boards...\n" << std::flush;

    std::string detectedPort = findArduinoBoard();
    
    std::cout << "Found Arduino on port: " << detectedPort << "\n";
    std::cout << "Uploading code to the board..." << "\n";

    std::string uploadCommand = ARDUINO_CLI_PATH + " upload -p " + detectedPort + " --fqbn arduino:avr:" + boardType + " --build-path \"" + sketchDir.string() + "/build_cache\"" + " \"" + sketchDir.string() + "\"";
    int uploadStatus = system(uploadCommand.c_str());

    if (uploadStatus == 0) {
        std::cout << "Upload successful!" << "\n";
        return true;
    } else {
        std::cerr << "Upload failed! Check connection." << "\n";
        return false;
    }
}

void Compiler::printSketchFileCode(std::filesystem::path inoFilePath) {
    std::string sketchCode;

    std::ifstream sketchCodeFile(inoFilePath.string());

    constexpr char BORDER[] = "-------------------------------------------------------------------------------------";

    std::cout << BORDER << "\n";

    if (sketchCodeFile.is_open()) {
        while (getline(sketchCodeFile, sketchCode)) {
            std::cout << "| " << std::left << std::setw(82) << sketchCode << "|\n";
        }
    } else {
        std::cerr << "Warning: Cann't open the sketch code file\n";
    }

    std::cout << BORDER << "\n";

    sketchCodeFile.close();
}

bool Compiler::runMelloCompiler(int argc, char* argv[]) {
    std::string filePath;

    if (argc > 1) {
        filePath = argv[1];
    }

    if (filePath.empty()) {
        std::cout << "Enter the Mello file path: ";
        std::cin >> filePath;
    }

    melloSourceFileName = std::filesystem::path(filePath).filename().string();

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

        for (const auto& stdLib : includedStdLibs) {
            ModuleLoader moduleLoader(stdLib);

            auto moduleNodes = moduleLoader.load();

            for (auto& node : moduleNodes) {
                program.insert(program.begin(), std::move(node));
            }
        }

        SemanticAnalyzer analyzer;

        for (const auto& node : program) {
            analyzer.analyzeNode(node.get());
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << "\n";

        return false;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";

        return false;
    }

    sketchDir = getTempSketchDir();
    std::filesystem::create_directories(sketchDir);
    std::filesystem::path inoFilePath = sketchDir / (sketchDir.filename().string() + ".ino");

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
                std::cerr << "\nSyntax Error: You cannot reassign the variable '" << varNode->name << "' outside of a function.\n" << "Please move '" << varNode->name << " = ...' inside 'fn start():' or 'fn loop():'.\n" << "\n";
                
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

            beginArgs.push_back(std::make_unique<LiteralNode>(Token(TokenType::INTEGER, "9600", 0, 0, "")));
            setupBody.push_back(std::make_unique<FunctionCallNode>("Serial.begin", std::move(beginArgs), 0, 0, ""));
        }

        FunctionNode autoSetup("setup", std::move(setupBody), 0, 0, "");

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
        FunctionNode autoLoop("loop", {}, 0, 0, "");

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

    // But wait, why are u reaing my code
    // yk, if u steal it, i will kill u
    // so btw, dont steal it

    bool isUpload = false;
    bool isCompileCode = true;
    bool printOutputCode = true;
    bool isShouldDeleteCache = false;
    bool isSaveSketchCodeDir = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];

        if (ARDUINO_SUPPORTED_BOARDS.count(arg)) {
            boardType = arg;
        } else if (arg == "--upload") {
            isUpload = true;
        } else if (arg == "--no-output") {
            printOutputCode = false;
        } else if (arg == "--save-code") {
            isSaveSketchCodeDir = true;
        } else if (arg == "--no-compile") {
            isCompileCode = false;
        } else if (arg == "--delete-cache") {
            isShouldDeleteCache = true;
        } else {
            std::cerr << "Warning: There is not arg called '" << arg << "'\n";
        }
    }

    if (printOutputCode) {
        printSketchFileCode(inoFilePath);
    }

    installLibraries();

    bool compiledSuccessfully = isCompileCode ? compileCode() : false;

    if (compiledSuccessfully && isUpload) {
        uploadCode();
    }

    if (!isSaveSketchCodeDir) {
        for (const auto& entry : std::filesystem::directory_iterator(sketchDir)) {
            if (entry.path().filename() == "build_cache" && !isShouldDeleteCache) {
                continue;
            }

            std::filesystem::remove_all(entry.path());
        }
    } else {
        std::cout << "Transpiled Arduino C++ code saved inside: " << sketchDir << "\n";
    }

    if (isShouldDeleteCache) {
        std::filesystem::remove_all(sketchDir);
    }

    return true;
}

bool Compiler::run(int argc, char* argv[]) {
    return runMelloCompiler(argc, argv);
}