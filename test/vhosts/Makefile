CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -fsanitize=address \
	-I../../include -I../../include/lexer -I../../include/parser -I../../include/vhosts -I../../include/protocol

SRC = test.cpp \
	../../src/vhosts/virtualhost.cpp \
	../../src/vhosts/server_types.cpp \
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
