#ifndef JOURNALLOGGER_H
#define JOURNALLOGGER_H

#include <string>

enum class ImportanceLevel {
    High,
    Medium,
    Low
};

class JournalLogger {
public:
    JournalLogger(const std::string& filename, ImportanceLevel level = ImportanceLevel::Medium);

    ~JournalLogger();

    void SetLevel(ImportanceLevel level);

    void SaveMessage(const std::string& message, ImportanceLevel level);

    void SaveMessage(const std::string& message);

private:
    class Impl;
    Impl* pImpl;
};

#endif // JOURNALLOGGER_H
