#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

class Logger {
public:
    virtual ~Logger() = default;
    virtual void log(const std::string& message) = 0;
    virtual void setCursorPosition(int line) = 0;
};

class ConsoleLogger : public Logger {
public:
    void log(const std::string& message) override {
        std::cout << message << std::endl;
    }

    void setCursorPosition(int line) override {
#ifdef _WIN32
        COORD coord;
        coord.X = 0;
        coord.Y = line;
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleCursorPosition(hConsole, coord);
#else
        std::cout << "\033[" << line << "A";
#endif
    }
};

// File Logger class
class FileLogger : public Logger {
private:
    std::string filename;
    std::vector<std::string> fileContent;
    int currentLine;
    std::fstream file;

    void loadFileContent() {
        file.open(filename, std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open log file for reading");
        }

        fileContent.clear();
        std::string line;
        while (std::getline(file, line)) {
            fileContent.push_back(line);
        }
        file.close();
    }

    void saveFileContent() {
        file.open(filename, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open log file for writing");
        }

        for (const auto& line : fileContent) {
            file << line << std::endl;
        }
        file.close();
    }

public:
    FileLogger(const std::string& filename) : filename(filename), currentLine(0) {
        loadFileContent();
        file.open(filename, std::ios::in | std::ios::out);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }

    ~FileLogger() {
        if (file.is_open()) {
            saveFileContent();
            file.close();
        }
    }

    void log(const std::string& message) override {
        if (currentLine < fileContent.size()) {
            fileContent[currentLine] = message;
        } else {
            fileContent.push_back(message);
        }
    }

    void setCursorPosition(int line) override {
        if (line < 0) {
            throw std::out_of_range("Line number cannot be negative");
        }
        currentLine = line;
        if (currentLine >= fileContent.size()) {
            fileContent.resize(currentLine + 1);
        }
    }
};
