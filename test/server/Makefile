CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 \
           -I../../include \
           -I../../include/server \
           -I../../include/parser \
           -I../../include/lexer \
           -I../../include/vhosts \
           -pthread

LDFLAGS = -pthread

# Source files for main test
SERVER_SRC = ../../src/server/Reactor.cpp \
			 ../../src/server/Socket.cpp \
             ../../src/server/Server.cpp \
             ../../src/server/HttpServer.cpp \
             ../../src/lexer/tokenize_factory.cpp \
             ../../src/parser/configparser.cpp \
             ../../src/vhosts/virtualhost.cpp \
             ../../src/vhosts/server_types.cpp \
              test.cpp

# Source files for reactor test
REACTOR_SRC = ../../src/server/Reactor.cpp \
              ../../src/server/Socket.cpp \
              ../../src/server/Server.cpp \
              test_reactor.cpp

# Object files
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
REACTOR_OBJ = $(REACTOR_SRC:.cpp=.o)

# Targets
TARGET = test
REACTOR_TARGET = test_reactor

.PHONY: all clean re run test_reactor

all: $(TARGET) $(REACTOR_TARGET)

$(TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(REACTOR_TARGET): $(REACTOR_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJ) $(REACTOR_OBJ) $(TARGET) $(REACTOR_TARGET)

re: clean all

run: $(TARGET)
	./$(TARGET)

test_reactor: $(REACTOR_TARGET)
	./$(REACTOR_TARGET)
