CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -fPIC
LDFLAGS := -shared
TARGET_LIB := libJournalLogger.so

.PHONY: all clean

all: $(TARGET_LIB)

$(TARGET_LIB): JournalLogger.cpp JournalLogger.h
	$(CXX) $(CXXFLAGS) -c JournalLogger.cpp -o JournalLogger.o
	$(CXX) $(LDFLAGS) -o $@ JournalLogger.o

clean:
	rm -f $(TARGET_LIB) *.o
