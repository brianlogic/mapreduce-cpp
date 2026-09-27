# make                 build everything
# make SAN=thread      build with ThreadSanitizer (the C++ version of go -race)
# make SAN=address     build with AddressSanitizer (memory bugs)
# make clean

CXX ?= c++
CXXFLAGS = -std=c++17 -O1 -g -Wall -pthread -I. $(if $(SAN),-fsanitize=$(SAN))
LDLIBS = -ldl

MR_SRCS = mr/coordinator.cpp mr/worker.cpp mr/rpc_lib.cpp
MR_HDRS = $(wildcard mr/*.h)
BINS = main/mrcoordinator main/mrworker main/mrsequential
APPS = $(patsubst %.cpp,%.so,$(wildcard mrapps/*.cpp))

all: $(BINS) $(APPS)

main/%: main/%.cpp $(MR_SRCS) $(MR_HDRS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(MR_SRCS) $(LDLIBS)

mrapps/%.so: mrapps/%.cpp mr/mr.h
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $@ $<

clean:
	rm -rf $(BINS) $(APPS) main/*.dSYM mrapps/*.dSYM

.PHONY: all clean
