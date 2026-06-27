#include "utils/Logger.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace PaperPad {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

void Logger::init(const std::string& filename) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) {
        m_file.open(filename, std::ios::out | std::ios::trunc);
        m_initialized = true;
        if (m_file.is_open()) {
            m_file << "[INFO] " << "Logger initialized." << std::endl;
        }
    }
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized && m_file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm bt;
        localtime_s(&bt, &in_time_t);
        
        m_file << "[" << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << "] [INFO] " << message << std::endl;
    }
}

void Logger::logError(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized && m_file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm bt;
        localtime_s(&bt, &in_time_t);
        
        m_file << "[" << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << "] [ERROR] " << message << std::endl;
    }
}

} // namespace PaperPad
