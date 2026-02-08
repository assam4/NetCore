CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -fsanitize=address -I../../include -I../../include/lexer -I../../include/parser
SRC = test.cpp ../../src/parser/configparser.cpp ../../src/lexer/tokenize_factory.cpp
OBJ = $(SRC:%.cpp=%.o)
TARGET = test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Explicit rules for object files
./test.o: ./test.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

../../src/parser/configparser.o: ../../src/parser/configparser.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

../../src/lexer/tokenize_factory.o: ../../src/lexer/tokenize_factory.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
