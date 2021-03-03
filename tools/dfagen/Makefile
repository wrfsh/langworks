AR ?= ar
CXX ?= g++
CXXFLAGS = -Wall -std=c++14 -fPIC -I.

HDRS := $(wildcard include/*/*.h)
SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc,%.o,$(SRCS))

TARGET := libdfagen.a

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(AR) rcs $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean
