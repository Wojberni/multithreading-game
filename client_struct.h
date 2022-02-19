#ifndef __CLIENT_STRUCT__
#define __CLIENT_STRUCT__

#include "point.h"

struct client_struct{
    int server_pid = 0;
    int round_nr = 0;
    int player_id = 0;
    int client_port = 0;
    int coins_carried = 0;
    int coins_brought = 0;
    int deaths = 0;
    bool slowed = false;
    MovingObject player;
    Point spawn_position;
    char player_board[5][10] = {0};
};

#endif