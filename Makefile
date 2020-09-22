CXXFLAGS = $(shell pkg-config --cflags) -g -std=gnu++11
#-std=c++17
LDFLAGS = $(shell pkg-config --libs sqlite3)

cme: cme.cpp
	g++ $(CXXFLAGS) $< $(LDFLAGS) -o $@
