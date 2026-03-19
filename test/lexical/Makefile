CXX = g++
CXXFLAGS = -Wall -Wextra -Werror --std=c++98  -I../../include -I../../include/lexer
SRC = ../../src/lexer/tokenize_factory.cpp  test.cpp
TARGET = test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
