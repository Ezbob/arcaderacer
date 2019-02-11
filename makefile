CXXFLAGS=-Wall -Wextra -municode -std=c++17
LDFLAGS=-lm

SRCS=racer.cpp
OBJS=$(patsubst %.cpp, %.o, $(SRCS))
HDRS=$(wildcard %.h)
TARGET=racer.exe

all: $(TARGET)

$(TARGET): $(OBJS) $(HDRS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	$(RM) $(TARGET) $(OBJS)