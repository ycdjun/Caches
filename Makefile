CC=gcc
CXX=g++
RM=rm -f

SRCS=cache-sim.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: cache-sim

cache-sim: $(OBJS)
	$(CXX) -o cache-sim $(OBJS)

cache-sim.o: cache-sim.cpp

clean:
	$(RM) $(OBJS)

