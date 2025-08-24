#include "JournalLogger.h"
#include <iostream>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <memory>

class MessageQueue {
public:
    void push(const std::string& message, ImportanceLevel level) {
        std::unique_lock<std::mutex> lock(mutex);

        queue.push({message, level});

        lock.unlock();

        condition.notify_one();
    }

    std::pair<std::string, ImportanceLevel> pop() {
        std::unique_lock<std::mutex> lock(mutex);

        condition.wait(lock, [this] {
            return !queue.empty() || stopped;
        });

        if (stopped && queue.empty()) {
            return{"", ImportanceLevel::Medium};
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


private:
    std::queue<std::pair<std::string, ImportanceLevel>> queue;
    std::mutex mutex;
    std::condition_variable condition;
    std::atomic<bool> stopped{false};
};

int main(int argc, char* argv[]) {
    return 0;
}
