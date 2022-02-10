#ifndef __CLIENT_STRUCT__
#define __CLIENT_STRUCT__

#include "point.h"

struct client_struct{
    int player_id = 0;
    int client_port = 0;
    int coins_carried = 0;
    int coins_brought = 0;
    int deaths = 0;
    Point player_position;
    Point spawn_position;
    char player_board[5][10] = {0};
};

#endif