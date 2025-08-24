CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -fPIC
LDFLAGS := -shared
TARGET_LIB := libJournalLogger.so
TARGET_APP := JournalLogger

.PHONY: all clean run

all: $(TARGET_LIB) $(TARGET_APP)

$(TARGET_LIB): JournalLogger.cpp JournalLogger.h
        $(CXX) $(CXXFLAGS) -c JournalLogger.cpp -o JournalLogger.o
        $(CXX) $(LDFLAGS) -o $@ JournalLogger.o

$(TARGET_APP): main.cpp $(TARGET_LIB)
        $(CXX) $(CXXFLAGS) -L. -o $@ main.cpp -lJournalLogger -pthread

clean:
        rm -f $(TARGET_LIB) $(TARGET_APP) *.o

run: all
        LD_LIBRARY_PATH=. ./$(TARGET_APP) test.log INFO
