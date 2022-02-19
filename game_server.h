#ifndef __SERVER__
#define __SERVER__

#include <semaphore.h>
#include "client_struct.h"
#include <netinet/in.h>
#include <vector>
#include <thread>

#define SOCKET_ERROR -1
#define SERVER_PORT 8989
#define CLIENT_LIMIT 4
#define SERVER_BACKLOG 4

#define CLIENT_ROWS 5
#define CLIENT_COLS 10
#define BOARD_ROWS 31
#define BOARD_COLS 64
#define SCOREBOARD_WIDTH 50
#define MARGIN_SCOREBOARD BOARD_COLS + 5

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

class Server {
    enum legend {FREE, WALL, BUSH, COIN, TREASURE, TREASURE_LARGE, TREASURE_DROP, CAMPSITE, BEAST, PLAYER1, PLAYER2, PLAYER3, PLAYER4};
    enum colors {WALL_COLOR = 1, COIN_COLOR, CAMPSITE_COLOR, PLAYER_COLOR, TEXT_COLOR};
    int rows = 0, columns = 0;
    char board[BOARD_ROWS][BOARD_COLS] = {FREE};

    client_struct clients[CLIENT_LIMIT];
    bool connected_clients[CLIENT_LIMIT] = {false};
    bool server_running = true;
    int server_PID = 0;
    int round_number = 0;
    std::vector<Collectible> coins;
    std::vector<MovingObject> beasts;
    std::vector<std::thread> threads_beast;
    Point campfire;

    public:
        Server();
        ~Server();
        void start(Server *server);
        int init();
        void handle_clients(Server *server);

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
    void handle_input(Server *server, int input);
    void add_beast(Server *server);
    void add_collectible(int value);
    void add_collectible(int x, int y, int value);
    void set_client(int client_id, int socket_id);
    void connect_client(Server *server, int client_id, int socket_id);
    void move_player(int client_id, int input);
    int get_first_free_client();
    void new_round();
    void set_campsite();
    void move_beast(int x, int y);
    int player_is_seen(MovingObject &beast);
    void move_towards_player(MovingObject &beast, int player_id);
    void set_client_board(int client_id);
    void send_data(int client_id);
    void paint_all();
    void check_beast_collision(MovingObject &beast, int player_id);
    void check_for_collisions(int client_id);
    void collect_collectible(int client_id);
};

#endif