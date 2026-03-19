CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -fsanitize=address \
	-I../../include -I../../include/lexer -I../../include/parser -I../../include/server

SRC = test.cpp \
	../../src/server/virtualhost.cpp \
	../../src/server/server_types.cpp \
	../../src/parser/configparser.cpp \
	../../src/lexer/tokenize_factory.cpp

OBJ = $(SRC:%.cpp=%.o)
TARGET = test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
