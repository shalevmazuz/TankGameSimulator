CXX      = g++
CXXFLAGS = -fPIC -std=c++20 -Wall -Werror -Wextra -pedantic -I.. -I../UserCommon
LDFLAGS  = -shared
TARGET   = Algorithm.so
SRC      = TankAlgorithm_A.cpp Player_A.cpp $(wildcard ../UserCommon/*.cpp)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
