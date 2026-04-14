CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 \
		   -I./include \
		   -I./include/server \
		   -I./include/parser \
		   -I./include/lexer \
		   -I./include/vhosts \
		   -I./include/signal \
		   -I./include/protocol \
		   -I./include/utils \
		   -pthread

TARGET = webserv

SRCS =  src/main.cpp \
		src/server/HttpServer.cpp \
		src/server/Reactor.cpp \
		src/server/Server.cpp \
		src/server/Socket.cpp \
		src/server/CGI.cpp \
		src/lexer/tokenize_factory.cpp \
		src/parser/configparser.cpp \
		src/vhosts/virtualhost.cpp \
		src/vhosts/server_types.cpp \
		src/signal/SignalHandler.cpp \
		src/protocol/http_request.cpp \
		src/protocol/http_cookie.cpp \
		src/protocol/session_store.cpp \
		src/protocol/http_response.cpp \
		src/protocol/http_types.cpp \
		src/protocol/http_transaction.cpp

OBJS = $(SRCS:.cpp=.o)

RESET   = \033[0m
BOLD    = \033[1m
GREEN   = \033[32m
CYAN    = \033[36m
YELLOW  = \033[33m
MAGENTA = \033[35m
RED     = \033[31m
DIM     = \033[2m


all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "$(BOLD)$(CYAN)  linking   →  $(MAGENTA)$(TARGET)$(RESET)"
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "$(BOLD)$(GREEN)  ✓ build complete → $(TARGET)$(RESET)"

%.o: %.cpp
	@echo "$(DIM)$(YELLOW)  compiling  "
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(RED)  cleaning objects…$(RESET)"
	@rm -f $(OBJS)
	@echo "$(DIM)  done$(RESET)"

fclean: clean
	@echo "$(RED)  removing $(TARGET)…$(RESET)"
	@rm -f $(TARGET)
	@echo "$(DIM)  done$(RESET)"

re: fclean all

.PHONY: all clean fclean re
