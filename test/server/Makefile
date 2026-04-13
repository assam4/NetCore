CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 \
           -I../../include \
           -I../../include/server \
           -I../../include/parser \
           -I../../include/lexer \
           -I../../include/vhosts \
           -I../../include/signal \
           -I../../include/protocol \
           -I../../include/utils \
           -pthread

LDFLAGS = -pthread

TARGET = test
REACTOR_TARGET = test_reactor

# ===================== SOURCES =====================

SERVER_SRC = ../../src/server/Reactor.cpp \
             ../../src/server/Socket.cpp \
             ../../src/server/Server.cpp \
             ../../src/server/HttpServer.cpp \
             ../../src/lexer/tokenize_factory.cpp \
             ../../src/parser/configparser.cpp \
             ../../src/vhosts/virtualhost.cpp \
             ../../src/vhosts/server_types.cpp \
             ../../src/signal/SignalHandler.cpp \
             ../../src/protocol/http_request.cpp \
             ../../src/protocol/http_response.cpp \
             ../../src/protocol/http_types.cpp \
             ../../src/protocol/http_transaction.cpp \
             test.cpp

REACTOR_SRC = ../../src/server/Reactor.cpp \
              ../../src/server/Socket.cpp \
              ../../src/server/Server.cpp \
              ../../src/server/HttpServer.cpp \
              ../../src/lexer/tokenize_factory.cpp \
              ../../src/parser/configparser.cpp \
              ../../src/vhosts/virtualhost.cpp \
              ../../src/vhosts/server_types.cpp \
              ../../src/signal/SignalHandler.cpp \
              ../../src/protocol/http_request.cpp \
              ../../src/protocol/http_response.cpp \
              ../../src/protocol/http_types.cpp \
              ../../src/protocol/http_transaction.cpp \

# ===================== OBJECTS (SEPARATED!) =====================

SERVER_OBJ = $(SERVER_SRC:.cpp=.server.o)
REACTOR_OBJ = $(REACTOR_SRC:.cpp=.reactor.o)

# ===================== RULES =====================

all: $(TARGET)

$(TARGET): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(REACTOR_TARGET): $(REACTOR_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# ===================== COMPILE RULE =====================

%.server.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.reactor.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ===================== CLEAN =====================

clean:
	rm -f *.server.o *.reactor.o $(TARGET) $(REACTOR_TARGET)

fclean: clean

re: fclean all

run: $(TARGET)
	./$(TARGET)

run_reactor: $(REACTOR_TARGET)
	./$(REACTOR_TARGET)

.PHONY: all clean fclean re run run_reactor
