CXX = g++
CXXFLAGS = -std=c++20 -Wall -Werror -Wextra -pedantic -rdynamic -pthread -I.. -I../UserCommon
LDFLAGS = -ldl
TARGET = simulator
SRC = AlgorithmRegistrar.cpp \
      GameManagerRegistrar.cpp \
      GameManagerRegistration.cpp \
      PlayerRegistration.cpp \
      Simulator.cpp \
      TankAlgorithmRegistration.cpp \
      main.cpp \
      $(wildcard ../UserCommon/*.cpp)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
