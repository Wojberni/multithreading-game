#ifndef __CLIENT__
#define __CLIENT__

#include "client_struct.h"
#include <arpa/inet.h>

#define SERVER_PORT 8989
#define SOCKET_ERROR -1
#define CLIENT_ROWS 5
#define CLIENT_COLS 10

#define BOARD_ROWS 31
#define BOARD_COLS 64
#define SCOREBOARD_WIDTH 50
#define MARGIN_SCOREBOARD BOARD_COLS + 5

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

class Client{
    enum legend {FREE, WALL, BUSH, COIN, TREASURE, TREASURE_LARGE, TREASURE_DROP, CAMPSITE, BEAST, PLAYER1, PLAYER2, PLAYER3, PLAYER4};
    enum colors {WALL_COLOR = 1, COIN_COLOR, CAMPSITE_COLOR, PLAYER_COLOR, TEXT_COLOR};
    int rows = 0, columns = 0;
    int network_socket = 0;
    client_struct client_info;
    bool is_connected = true;

    public:
        Client();
        ~Client();
        int connection();
        void play_game(Client *client);
        int init();

    private:
        void init_colors();
        bool check_size_terminal();
        void init_window();
        void print_scoreboard();
        void print_board();
        void receiver();
        void clear_board();
};

#endif