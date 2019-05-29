CXXFLAGS = -g -std=c++17
LDFLAGS = -lsqlite3

cme: cme.cpp
	g++ $(CXXFLAGS) cme.cpp -o cme $(LDFLAGS)
