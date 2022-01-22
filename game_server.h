#ifndef __SERVER__
#define __SERVER__

#include <semaphore.h>
#include "client.h"
#include <netinet/in.h>
#include <vector>

#define SOCKET_ERROR -1
#define SERVER_PORT 8989
#define CLIENT_LIMIT 4
#define SERVER_BACKLOG 4

#define BOARD_ROWS 31
#define BOARD_COLS 64
#define SCOREBOARD_WIDTH 50
#define MARGIN_SCOREBOARD BOARD_COLS + 5

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

class Server {
    enum legend {FREE, WALL, BUSH, COIN, TREASURE, TREASURE_LARGE, TREASURE_DROP, CAMPSITE}; // todo class enum?
    enum colors {WALL_COLOR = 1, COIN_COLOR, CAMPSITE_COLOR, PLAYER_COLOR, TEXT_COLOR};
    int rows = 0, columns = 0;
    char board[BOARD_ROWS][BOARD_COLS] = {FREE};

    Client clients[CLIENT_LIMIT];
    bool connected_clients[CLIENT_LIMIT] = {false};
    int server_PID = 0;
    int round_number = 0;
    std::vector<Collectible> coins;
    std::vector<MovingObject> beasts;
    Point campfire;

    public:
        Server();
        ~Server();
        void start();

private:
    void init_window();
    void init_colors();
    int check_size_terminal();
    void set_board();
    void paint_board();
    void paint_scoreboard();
    int count_clients_connected();
    void paint_players();
    void paint_beasts();
    void handle_input(int input);
    void add_beast();
    void add_collectible(int value);
    void add_collectible(int x, int y, int value);
    void move_beasts();
    //int handle_event(int input);
};

#endif