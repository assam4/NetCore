CXX = g++
CXXFLAGS = -std=c++98 -Wall -I../../include/vhosts/  -I../../include/protocol/
SRC = test.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = test_protocol

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ ../../src/protocol/http_request.cpp

clean:
	rm -f $(OBJ) $(TARGET)
