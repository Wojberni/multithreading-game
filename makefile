CPP = g++
CPPFLAGS = -g -std=c++14 -Wall -pedantic
LINK_FLAGS = -lncurses -lrt -lpthread
OBJS = ./objs

all: directory $(OBJS)/server $(OBJS)/client permission

directory:
	mkdir -p $(OBJS)

$(OBJS)/server: game_server.cpp game_server.h
	$(CPP) $(CPPFLAGS) $< -o $@ $(LINK_FLAGS)

$(OBJS)/client: game_client.cpp game_client.h
	$(CPP) $(CPPFLAGS) $< -o $@ $(LINK_FLAGS)

permission:
	chmod 777 -R $(OBJS)

clean:
	rm -rf $(OBJS)
