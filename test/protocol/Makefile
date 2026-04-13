CXX = g++
CXXFLAGS = -std=c++98 -Wall -I../../include/vhosts/  -I../../include/protocol/ -I../../include/server/ -I../../include/utils/
SRC = test.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = test_protocol

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ \
	  ../../src/protocol/http_cookie.cpp \
	  ../../src/protocol/http_response.cpp \
	  ../../src/protocol/http_types.cpp \
	  ../../src/protocol/http_request.cpp \
	  ../../src/server/Server.cpp \
	  ../../src/server/Socket.cpp

clean:
	rm -f $(OBJ) $(TARGET)
