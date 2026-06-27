#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace PaperPad {

class Logger {
public:
    static Logger& getInstance();

    void init(const std::string& filename);
    void log(const std::string& message);
    void logError(const std::string& message);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream m_file;
    std::mutex m_mutex;
    bool m_initialized = false;
};

#define LOG_INFO(msg) PaperPad::Logger::getInstance().log(msg)
#define LOG_ERROR(msg) PaperPad::Logger::getInstance().logError(msg)

} // namespace PaperPad
