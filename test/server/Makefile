CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 \
           -I../../include \
           -I../../include/server \
           -I../../include/parser \
           -I../../include/lexer \
           -I../../include/virtual_host \
           -pthread

LDFLAGS = -pthread

# Source files
SERVER_SRC = ../../src/server/Reactor.cpp \
			 ../../src/server/Socket.cpp \
             ../../src/server/Server.cpp \
             ../../src/server/HttpServer.cpp \
             ../../src/lexer/tokenize_factory.cpp \
             ../../src/parser/configparser.cpp \
             ../../src/virtual_host/virtualhost.cpp \
             ../../src/virtual_host/server_types.cpp \
              test.cpp

# Object files
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)

# Target
TARGET = webserv

all: $(TARGET)

$(TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJ) $(TARGET)

re: clean all

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean re run
