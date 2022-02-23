CPP = g++
CPPFLAGS = -g -c -std=c++14 -Wall -pedantic
LINK_FLAGS = -lncurses -lrt -lpthread
OBJS = ./objs

all: directory $(OBJS)/server.o $(OBJS)/client.o

directory:
	mkdir -p $(OBJS)
$(OBJS)/server.o: game_server.cpp game_server.h
	$(CPP) $(CPPFLAGS) $< -o $@ $(LINK_FLAGS)
$(OBJS)/client.o: game_client.cpp game_client.h
	$(CPP) $(CPPFLAGS) $< -o $@ $(LINK_FLAGS)
clean:
	rmdir -rf $(OBJS)
