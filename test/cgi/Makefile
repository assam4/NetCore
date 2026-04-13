CXX = g++
CXXFLAGS = -Wall -Wextra -Werror --std=c++98  -I../../include -I../../include/server -I../../include/protocol -I../../include/vhosts -I../../include/utils
SRC = ../../src/server/CGI.cpp ../../src/protocol/http_response.cpp ../../src/protocol/http_types.cpp ../../src/protocol/http_cookie.cpp test.cpp
TARGET = test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean
