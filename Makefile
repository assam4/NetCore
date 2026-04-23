CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -fsanitize=address -g3 -std=c++98 \
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
BUILD_DIR = build

SRCS = src/main.cpp \
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
       src/protocol/http_upload.cpp \
       src/protocol/http_response.cpp \
       src/protocol/http_types.cpp \
       src/protocol/http_transaction.cpp

OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

RESET   := \033[0m
BOLD    := \033[1m
GREEN   := \033[32m
CYAN    := \033[36m
YELLOW  := \033[33m
MAGENTA := \033[35m
RED     := \033[31m
DIM     := \033[2m

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)/src/server \
	          $(BUILD_DIR)/src/lexer \
	          $(BUILD_DIR)/src/parser \
	          $(BUILD_DIR)/src/vhosts \
	          $(BUILD_DIR)/src/signal \
	          $(BUILD_DIR)/src/protocol

$(BUILD_DIR)/$(TARGET): $(OBJS) | $(BUILD_DIR)
	@echo "$(BOLD)$(CYAN)Linking → $(MAGENTA)$(BUILD_DIR)/$(TARGET)$(RESET)"
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "$(BOLD)$(GREEN)✓ Build complete → $(BUILD_DIR)/$(TARGET)$(RESET)"

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	@echo "$(DIM)$(YELLOW)Compiling $<$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning objects…$(RESET)"
	@rm -rf $(BUILD_DIR)
	@echo "$(DIM)Done$(RESET)"

fclean: clean
	@echo "$(RED)Removing executable…$(RESET)"
	@rm -f $(TARGET)
	@echo "$(DIM)Done$(RESET)"

re: fclean all

.PHONY: all clean fclean re
