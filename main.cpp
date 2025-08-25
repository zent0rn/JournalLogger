#include "JournalLogger.h"
#include <iostream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <optional>

class MessageQueue {
public:
    void push(const std::string& message, ImportanceLevel level) {
        std::unique_lock<std::mutex> lock(mutex);

        queue.push({message, level});

        lock.unlock();

        condition.notify_one();
    }

    std::optional<std::pair<std::string, ImportanceLevel>> pop() {
        std::unique_lock<std::mutex> lock(mutex);

        condition.wait(lock, [this] {
            return !queue.empty() || stopped;
        });

        if (stopped && queue.empty()) {
            return std::nullopt;
        }

        auto item = queue.front();
        queue.pop();
        return item;
    }


    void stop() {
        std::unique_lock<std::mutex> lock(mutex);
        stopped = true;

        condition.notify_all();
    }

    bool is_stopped() const {
        return stopped.load();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.empty();
    }

private:
    std::queue<std::pair<std::string, ImportanceLevel>> queue;
    mutable std::mutex mutex;
    std::condition_variable condition;
    std::atomic<bool> stopped{false};
};

int LogWriter(MessageQueue& queue, JournalLogger& logger) {
    int processedCount = 0;

    while (!queue.is_stopped() || !queue.empty()) {
        auto item = queue.pop();

        if (item.has_value()) {
            logger.SaveMessage(item->first, item->second);
            processedCount++;
        }
        else if (queue.is_stopped()) {
            break;
        }
    }

    logger.SaveMessage("Log writer thread stopped, processed " +
                   std::to_string(processedCount) + " messages",
               ImportanceLevel::High);

    return processedCount;
}

ImportanceLevel ParseImportanceLevel(const std::string& levelStr) {
    if (levelStr == "LOW") return ImportanceLevel::Low;
    if (levelStr == "MEDIUM") return ImportanceLevel::Medium;
    if (levelStr == "HIGH") return ImportanceLevel::High;
    return ImportanceLevel::Medium;
}

std::string ImportanceLevelToString(ImportanceLevel level) {
    switch (level) {
    case ImportanceLevel::Low: return "LOW";
    case ImportanceLevel::Medium: return "MEDIUM";
    case ImportanceLevel::High: return "HIGH";
    default: return "MEDIUM";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0]
                  << " <файл_журнала> <уровень_важности>" << std::endl;
        std::cerr << "Уровни важности: LOW, MEDIUM, HIGH" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    ImportanceLevel defaultLevel = ParseImportanceLevel(argv[2]);

    try {
        JournalLogger logger(filename, defaultLevel);
        MessageQueue queue;

        std::thread writerThread(LogWriter, std::ref(queue), std::ref(logger));

        std::cout << "Логгер запущен. Файл: " << filename << std::endl;
        std::cout << "Уровень важности сообщения по умолчанию: "
                  << ImportanceLevelToString(defaultLevel) << std::endl;
        std::cout << "Формат ввода: [УРОВЕНЬ]:СООБЩЕНИЕ" << std::endl;
        std::cout << "Уровни важности: LOW, MEDIUM, HIGH" << std::endl;
        std::cout << "Пример: MEDIUM:Запуск системы" << std::endl;
        std::cout << "Если уровень не указан, "
                     "используется уровень по умолчанию" << std::endl;
        std::cout << "Сообщения с уровнем важности ниже, "
                     "чем по умолчанию не будут сохранены" << std::endl;
        std::cout << "Для выхода введите 'exit'" << std::endl;
        std::cout << "Для изменения уровня важности сообщения "
                     "по умолчанию введите 'level УРОВЕНЬ'" << std::endl;
        std::cout << std::string(50, '-') << std::endl;

        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "exit") {
                break;
            }

            if (input.find("level ") == 0) {
                std::string levelStr = input.substr(6);
                ImportanceLevel newLevel = ParseImportanceLevel(levelStr);
                logger.SetLevel(newLevel);
                std::cout << "Уровень важности сообщений "
                             "по умолчанию изменен на: " << levelStr << std::endl;
                continue;
            }

            size_t colonPos = input.find(':');
            if (colonPos != std::string::npos) {
                std::string levelStr = input.substr(0, colonPos);
                std::string message = input.substr(colonPos + 1);

                ImportanceLevel level = ParseImportanceLevel(levelStr);
                ImportanceLevel currentLevel = logger.GetLevel();

                if (static_cast<int>(level) < static_cast<int>(currentLevel)) {
                    std::cout << "Сообщение не будет сохранено, так "
                                 "как имеет более низкий уровень важности" << std::endl;
                } else {
                    queue.push(message, level);
                    std::cout << "Сообщение успешно сохранено" << std::endl;
                }
            } else {
                ImportanceLevel currentLevel = logger.GetLevel();
                queue.push(input, currentLevel);
                std::cout << "Сообщение успешно сохранено с "
                             "текущим уровнем важности по умолчанию ("
                                 + ImportanceLevelToString(currentLevel) + ")"<< std::endl;
            }
        }

        queue.stop();
        writerThread.join();

        std::cout << "Логгер остановлен." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
