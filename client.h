#ifndef __CLIENT__
#define __CLIENT__

#include "point.h"

struct Client{
    int player_id;
    int client_port;
    int coins_carried = 0;
    int coins_brought = 0;
    int deaths = 0;
    Point player_position;
    Point spawn_position;
    char player_board[5][10] = {0};
};

#endif