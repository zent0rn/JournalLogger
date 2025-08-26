#include "JournalLogger.h"
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <memory>

class JournalLogger::Impl {
public:
    Impl(const std::string& filename, ImportanceLevel level)
        : currentLevel(level) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Не удалось открыть файл журнала: " + filename);
        }
    }

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    ~Impl() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }

    ImportanceLevel GetLevel() {
        std::lock_guard<std::mutex> lock(writeMutex);
        return currentLevel;
    }

    void SetLevel(ImportanceLevel level) {
        std::lock_guard<std::mutex> lock(writeMutex);
        currentLevel = level;
    }

    void SaveMessage(const std::string& message, ImportanceLevel level) {
        if (static_cast<int>(level) < static_cast<int>(currentLevel)) {
            return;
        }

        std::lock_guard<std::mutex> lock(writeMutex);

        logFile << CurrentTime() << " [" << LevelToString(level) << "] " << message << std::endl;

        if (logFile.fail()) {
            throw std::runtime_error("Ошибка при записи в файл журнала");
        }

    }

private:
    std::ofstream logFile;
    ImportanceLevel currentLevel;
    std::mutex writeMutex;

    std::string LevelToString(ImportanceLevel level) const {
        switch (level) {
        case ImportanceLevel::Low:   return "LOW";
        case ImportanceLevel::Medium:    return "MEDIUM";
        case ImportanceLevel::High:   return "HIGH";
        default:                return "UNKNOWN";
        }
    }

    std::string CurrentTime() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

};

JournalLogger::JournalLogger(const std::string& filename, ImportanceLevel level)
    : pImpl(std::make_unique<Impl>(filename, level)) {}

JournalLogger::JournalLogger(JournalLogger&& other) noexcept = default;
JournalLogger& JournalLogger::operator=(JournalLogger&& other) noexcept = default;

JournalLogger::~JournalLogger() = default;

ImportanceLevel JournalLogger::GetLevel() {
    return pImpl->GetLevel();
}

void JournalLogger::SetLevel(ImportanceLevel level) {
    pImpl->SetLevel(level);
}

void JournalLogger::SaveMessage(const std::string& message, ImportanceLevel level) {
    pImpl->SaveMessage(message, level);
}

void JournalLogger::SaveMessage(const std::string& message) {
    pImpl->SaveMessage(message, ImportanceLevel::Medium);
}
