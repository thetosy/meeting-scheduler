

CXXFLAGS = -ggdb -Wall -std=c++11



client: client.o
	g++ $(CXXFLAGS) -o client client.o

client.o:  client.cpp
	g++ $(CXXFLAGS) -c client.cpp

serverA: serverA.o timeslotsFuncs.o
	g++ $(CXXFLAGS) -o serverA serverA.o timeslotsFuncs.o

serverA.o:  serverA.cpp timeslotsFuncs.h
	g++ $(CXXFLAGS) -c serverA.cpp

serverB: serverB.o timeslotsFuncs.o
	g++ $(CXXFLAGS) -o serverB serverB.o timeslotsFuncs.o

serverB.o:  serverB.cpp timeslotsFuncs.h
	g++ $(CXXFLAGS) -c serverB.cpp

serverM: serverM.o timeslotsFuncs.o
	g++ $(CXXFLAGS) -o serverM serverM.o timeslotsFuncs.o

serverM.o:  serverM.cpp timeslotsFuncs.h
	g++ $(CXXFLAGS) -c serverM.cpp

timeslotsFuncs.o: timeslotsFuncs.cpp timeslotsFuncs.h
	g++ $(CXXFLAGS) -c timeslotsFuncs.cpp

all: serverM serverA serverB client

clean:
	rm -f *.o serverA serverB serverM client
