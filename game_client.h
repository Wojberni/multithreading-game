#ifndef __CLIENT__
#define __CLIENT__

#define SERVER_PORT 8989
#define SOCKET_ERROR -1
#define CLIENT_ROWS 5
#define CLIENT_COLS 10

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

class Client{
    enum legend {FREE, WALL, BUSH, COIN, TREASURE, TREASURE_LARGE, TREASURE_DROP, CAMPSITE};
    enum colors {WALL_COLOR = 1, COIN_COLOR, CAMPSITE_COLOR, PLAYER_COLOR, TEXT_COLOR};
    char board[CLIENT_ROWS][CLIENT_COLS] = {FREE};
    void print_board();

    public:
        Client();
        ~Client();
        void connect();

};

#endif