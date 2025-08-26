#ifndef JOURNALLOGGER_H
#define JOURNALLOGGER_H

#include <string>

enum class ImportanceLevel {
    Low,
    Medium,
    High
};

class JournalLogger {
public:
    JournalLogger(const std::string& filename, ImportanceLevel level = ImportanceLevel::Medium);

    JournalLogger(const JournalLogger&) = delete;
    JournalLogger& operator=(const JournalLogger&) = delete;

    JournalLogger(JournalLogger&& other) noexcept;
    JournalLogger& operator=(JournalLogger&& other) noexcept;

    ~JournalLogger();

    ImportanceLevel GetLevel();

    void SetLevel(ImportanceLevel level);

    void SaveMessage(const std::string& message, ImportanceLevel level);

    void SaveMessage(const std::string& message);

private:
    class Impl;
    Impl* pImpl;
};

#endif // JOURNALLOGGER_H
