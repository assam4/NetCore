CXX = g++
CXXFLAGS = -Wall -Wextra -Werror  -I../../include -I../../include/lexer
SRC = ../../src/lexer/tokenize_factory.cpp  test.cpp
TARGET = test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
