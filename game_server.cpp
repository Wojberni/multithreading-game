#include "game_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

int main(){
    Server *server = new Server();
    if(server->init() == EXIT_FAILURE)
        return EXIT_FAILURE;

    std::thread client_handler(&Server::handle_clients, server, server);
    std::thread input_server(&Server::start, server, server);

    client_handler.detach();
    input_server.join();

    delete server;
    return 0;
}

Server::Server() {

}

void Server::handle_clients(Server *server){
    SA_IN server_address;
    socklen_t address_size;

    int server_socket;
    SA_IN client_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == SOCKET_ERROR){
        printw("Failed to create stream!\n");
        return;
    }

    // terminating program won't trigger bind failed
    int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == SOCKET_ERROR)
        printw("Setsockopt(SO_REUSEADDR) failed!\n");

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    int result = bind(server_socket, (SA*)&server_address, sizeof(server_address));
    if(result == SOCKET_ERROR){
        printw("Bind failed!\n");
        return;
    }
    result = listen(server_socket, SERVER_BACKLOG);
    if(result == SOCKET_ERROR){
        printw("Listening failed!\n");
        return;
    }

    std::thread thread_pool[4];
    int client_socket[4] = {0};
    this->server_PID = server_socket;

    while(server_running){
        int i = get_first_free_client();
        if(i == -1){
            sleep(1);
            continue;
        }
        else{
            address_size = sizeof(SA_IN);
            client_socket[i] = accept(server_socket, (SA*)&client_address, (socklen_t*)&address_size);
            if(client_socket[i] == SOCKET_ERROR){
                printw("Error connecting client_info %d to server!", i);
                continue;
            }
            connected_clients[i] = true;
            thread_pool[i] = std::thread(&Server::connect_client, server, server, i, client_socket[i]);
            thread_pool[i].detach();
        }

    }
}

void Server::connect_client(Server *server, int client_id, int socket_id){
    set_client(client_id, socket_id);
    std::thread sender(&Server::send_data, server, client_id);
    sender.detach();
    //sem_post(&client_count);
    while(connected_clients[client_id] && server_running){
        int choice;
        int res = recv(socket_id, &choice, sizeof(choice), 0);
        if(res == SOCKET_ERROR){
            printw("Error reading socket data!");
            continue;
        }
        if(choice == 'q' || choice == 'Q'){
            connected_clients[client_id] = false;
            break;
        }
        move_player(client_id, choice);
        collect_collectible(client_id);
        check_for_collisions(client_id);
        add_coins(client_id);
    }
    close(socket_id);
}

void Server::add_coins(int client_id){
    if(clients[client_id].player.left == campfire && clients[client_id].coins_carried){
        clients[client_id].coins_brought += clients[client_id].coins_carried;
        clients[client_id].coins_carried = 0;
    }
}

void Server::collect_collectible(int client_id) {
    int client_x1 = clients[client_id].player.left.x;
    int client_x2 = clients[client_id].player.right.x;
    int client_y = clients[client_id].player.left.y;
    for(int i = 0; i < coins.size(); i++){
        if(coins[i].point == Point(client_x1, client_y) || coins[i].point == Point(client_x2, client_y)){
            clients[client_id].coins_carried += coins[i].value;
            board[coins[i].point.y][coins[i].point.x] = FREE;
            coins.erase(coins.begin() + i);
        }
    }
}

void Server::check_for_collisions(int client_id) {
    for(int i = 0; i < CLIENT_LIMIT; i++){
        if(i != client_id && connected_clients[i]){
            if(clients[i].player.left.y == clients[client_id].player.left.y){
                if(clients[i].player == clients[client_id].player ||
                   clients[i].player.left.x == clients[client_id].player.right.x ||
                   clients[i].player.right.x == clients[client_id].player.left.x){
                    clients[i].coins_carried += clients[client_id].coins_carried;
                    clients[client_id].coins_carried = 0;
                    set_player_after_collision(i);
                    set_player_after_collision(client_id);
                }
            }
        }
    }
}

void Server::set_player_after_collision(int client_id){
    clients[client_id].deaths += 1;
    add_collectible(clients[client_id].player.left.x, clients[client_id].player.left.y, clients[client_id].coins_carried);
    clients[client_id].coins_carried = 0;
    clients[client_id].player.left = clients[client_id].spawn_position;
    clients[client_id].player.right = clients[client_id].spawn_position + Point(1, 0);
    clients[client_id].slowed = false;
}

void Server::send_data(int client_id){
    while(connected_clients[client_id] && server_running){
        set_client_board(client_id);
        clients[client_id].round_nr = round_number;
        send(clients[client_id].client_port, &clients[client_id], sizeof(struct client_struct), 0);
        sleep(1);
    }
}

void Server::set_client(int client_id, int socket_id){
    clients[client_id].server_pid = server_PID;
    clients[client_id].round_nr = round_number;
    clients[client_id].player_id = client_id;
    clients[client_id].client_port = socket_id;
    while(true){
        int x = rand() % (BOARD_COLS - 2) + 1; // todo rand omijajacy playerow
        int y = rand() % (BOARD_ROWS - 2) + 1;
        if(board[y][x] == FREE && board[y][x+1] == FREE){
            clients[client_id].spawn_position.y = y;
            clients[client_id].spawn_position.x = x;
            break;
        }
    }
    clients[client_id].player.left = clients[client_id].spawn_position;
    clients[client_id].player.right = clients[client_id].spawn_position + Point(1, 0);
    clients[client_id].coins_carried = 0;
    clients[client_id].coins_brought = 0;
    clients[client_id].deaths = 0;
    clients[client_id].slowed = false;
    set_client_board(client_id);
}

void Server::set_client_board(int client_id){
    int first_x = clients[client_id].player.left.x - CLIENT_COLS / 2 + 1;
    int first_y = clients[client_id].player.left.y - CLIENT_ROWS / 2;
    for(int i = 0; i < CLIENT_ROWS; i++){
        if(first_y + i < 0 || first_y + i >= BOARD_ROWS){
            for(int j = 0; j < CLIENT_COLS; j++){
                clients[client_id].player_board[i][j] = WALL;
            }
            continue;
        }
        for(int j = 0; j < CLIENT_COLS; j++){
            if( first_x + j < 0 || first_x + j >= BOARD_COLS)
                clients[client_id].player_board[i][j] = WALL;
            else
                clients[client_id].player_board[i][j] = board[first_y + i][first_x + j];
        }
    }
    for(int i = 0; i < CLIENT_LIMIT; i++){
        if(connected_clients[i]){
            if(clients[i].player.left.y >= first_y && clients[i].player.left.y < first_y + CLIENT_ROWS){
                if(clients[i].player.left.x >= first_x && clients[i].player.right.x < first_x + CLIENT_COLS){
                    clients[client_id].player_board[clients[i].player.left.y-first_y][clients[i].player.left.x-first_x] = PLAYER1 + i;
                    clients[client_id].player_board[clients[i].player.left.y-first_y][clients[i].player.right.x-first_x] = PLAYER1 + i;
                }
            }
        }
    }
    for(auto &x : beasts){
        if(x.left.y >= first_y && x.left.y < first_y + CLIENT_ROWS){
            if(x.left.x >= first_x && x.right.x < first_x + CLIENT_COLS){
                clients[client_id].player_board[x.left.y-first_y][x.left.x-first_x] = BEAST;
                clients[client_id].player_board[x.left.y-first_y][x.right.x-first_x] = BEAST;
            }
        }
    }
}

void Server::move_player(int client_id, int input){
    Point point, left, right;
    switch (input) {
        case KEY_UP:
            point.y = -1;
            break;
        case KEY_DOWN:
            point.y = 1;
            break;
        case KEY_LEFT:
            point.x = -1;
            break;
        case KEY_RIGHT:
            point.x = 1;
            break;
        default:
           break;
    }
    left = clients[client_id].player.left + point;
    right = clients[client_id].player.right + point;
    switch (input) {
        case KEY_UP:
        case KEY_DOWN:
            if(board[left.y][left.x] != WALL && board[right.y][right.x] != WALL){
                if(clients[client_id].slowed){
                    clients[client_id].slowed = false;
                    break;
                }
                if(board[left.y][left.x] == BUSH || board[right.y][right.x] == BUSH){
                    clients[client_id].slowed = true;
                }
                clients[client_id].player.left = left;
                clients[client_id].player.right = right;
            }
            break;
        case KEY_LEFT:
            if(board[left.y][left.x] != WALL){
                if(clients[client_id].slowed){
                    clients[client_id].slowed = false;
                    break;
                }
                if(board[left.y][left.x] == BUSH){
                    clients[client_id].slowed = true;
                }
                clients[client_id].player.left = left;
                clients[client_id].player.right = right;
            }
            break;
        case KEY_RIGHT:
            if(board[right.y][right.x] != WALL){
                if(clients[client_id].slowed){
                    clients[client_id].slowed = false;
                    break;
                }
                if(board[right.y][right.x] == BUSH){
                    clients[client_id].slowed = true;
                }
                clients[client_id].player.left = left;
                clients[client_id].player.right = right;
            }
            break;
        default:
            break;
    }
}

void Server::paint_all(){
    while(server_running){
        paint_board();
        paint_scoreboard();
        paint_players();
        paint_beasts();
        new_round();
        sleep(1);
    }
}

void Server::start(Server *server) {
    std::thread painter(&Server::paint_all, server);
    painter.detach();
    while(server_running){
        int input = getch();
        flushinp();

        if(input == 'q' || input == 'Q'){
            server_running = false;
            break;
        }
        handle_input(server, input);
    }

}

void Server::new_round(){
    round_number += 1;
}

Server::~Server() {
    sem_destroy(&client_sem);
    sem_destroy(&beast_sem);
    sem_destroy(&client_count);
    sem_destroy(&beast_count);
    pthread_mutex_destroy(&data);
}

int Server::init() {
    init_window();
    if(!has_colors() || check_size_terminal()){
        printf("Invalid size of terminal (min.%dx%d) or no colors support!\nCheck you terminal preferences!"
                , MARGIN_SCOREBOARD + SCOREBOARD_WIDTH, BOARD_ROWS);
        return EXIT_FAILURE;
    }
    init_colors();
    set_board();
    set_campsite();
    set_semaphores();
    server_running = true;
    return EXIT_SUCCESS;
}

void Server::set_semaphores(){
    sem_init(&client_sem, 0 , 0);
    sem_init(&beast_sem, 0 , 0);
    sem_init(&client_count, 0 , 0);
    sem_init(&beast_count, 0 , 0);
    pthread_mutex_init(&data, NULL);
}

void Server::set_campsite(){
    campfire.y = 17;
    campfire.x = 34;
}

void Server::init_window(){
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    //nodelay(stdscr, TRUE); // disable waiting for input
    curs_set(0);
    srand(time(NULL));
}

int Server::check_size_terminal(){
    getmaxyx(stdscr, rows, columns);
    if (rows < BOARD_ROWS || columns < BOARD_COLS + SCOREBOARD_WIDTH)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void Server::init_colors() {
    start_color();
    use_default_colors();
    init_pair(WALL_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(COIN_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CAMPSITE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(PLAYER_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);
}

void Server::set_board() {
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            if (j == 0 || j == BOARD_COLS - 1 || i == 0 || i == BOARD_ROWS - 1) // borders
                board[i][j] = WALL;
            if(i == 1){                                                 //  next rows below
                if(j == 9 || j == 18)
                    board[i][j] = WALL;
                if((j >= 30 && j <= 35))
                    board[i][j] = BUSH;
            }
            else if(i == 2){
                if( (j >= 3 && j <= 6) || j == 9 || (j >= 12 && j <= 18) || (j >= 21 && j <= 48) ||
                    (j >= 51 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 3){
                if(j == 6 || j == 12 || j == 21 || j == 27 || j == 33 || j == 48 || j == 54)
                    board[i][j] = WALL;
            }
            else if(i == 4){
                if( (j >= 1 && j <= 3) || j == 6 || (j >= 9 && j <= 12) || (j >= 15 && j <= 21) ||
                    j == 24 || j == 27 || j == 30 || j == 33 || (j >= 36 && j <= 45) ||
                    (j >= 48 && j <= 51) || j == 54 || (j >= 57 && j <= 62))
                    board[i][j] = WALL;
            }
            else if(i == 5){
                if(j == 6 || j == 9 || j == 15 || j == 24 || j == 30 ||
                   j == 45 || j == 48 || j == 54 || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 6){
                if( (j >= 3 && j <= 6) || j == 9 || (j >= 12 && j <= 15) || (j >= 18 && j <= 36) ||
                    (j >= 39 && j <= 45) || j == 48 ||(j >= 51 && j <= 57) || j == 60 )
                    board[i][j] = WALL;
            }
            else if(i == 7){
                if( j == 3 || j == 18 || j == 36 || j == 39 || j == 54)
                    board[i][j] = WALL;
            }
            else if(i == 8){
                if( (j >= 3 && j <= 9) || (j >= 12 && j <= 15) || j == 18 || (j >= 21 && j <= 30)
                    || j == 33 || j == 36 || j == 39 || j == 42 || (j >= 45 && j <= 48) || j == 51
                    || (j >= 54 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 9){
                if(j == 9 || j == 12 || j == 18 || j == 24 || j == 30 || j == 33 || j == 36 ||
                   j == 42 || j == 48 || j == 51)
                    board[i][j] = WALL;

            }
            else if(i == 10){
                if( (j >= 3 && j <= 6) || j == 9 || j == 12 || (j >= 15 && j <= 21) ||
                    (j >= 24 && j <= 27) || j == 30 || (j >= 33 && j <= 45) || j == 48 || j == 51 ||
                    (j >= 51 && j <= 62))
                    board[i][j] = WALL;
            }
            else if(i == 11){
                if( j == 3 || j == 9 || j == 12 || j == 27 || j == 30 || j == 48 || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 12){
                if( j == 3 || (j >= 6 && j <= 9) || (j >= 12 && j <= 24) || j == 27 ||
                    (j >= 30 && j <= 42) || j == 45 || (j >= 48 && j <= 51) || (j >= 54 && j <= 57)
                    || j == 60 )
                    board[i][j] = WALL;
            }
            else if(i == 13){
                if(j == 3 || j == 6 || j == 42 || j == 45 || j == 51 || j == 54 || j == 57)
                    board[i][j] = WALL;
                if((j >= 14 && j <= 19) )
                    board[i][j] = BUSH;
            }
            else if(i == 14){
                if(j == 3 || j == 6 || (j >= 9 && j <= 30) || (j >= 33 && j <= 39) || j == 42
                   || (j >= 45 && j <= 48) || j == 51 || j == 54 || (j >= 57 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 15){
                if(j == 3 || j == 12 || j == 18 || j == 42 || j == 48 || j == 51
                   || j == 57)
                    board[i][j] = WALL;
            }
            else if(i == 16){
                if((j >= 3 && j <= 9) || j == 12 || j == 15 || j == 18 || (j >= 21 && j <= 24) ||
                   (j >= 27 && j <= 33) || (j >= 36 && j <= 39) || j == 42 || j == 45 || j == 48 ||
                   (j >= 51 && j <= 54) || j == 57 || (j >= 60 && j <= 62))
                    board[i][j] = WALL;
            }
            else if(i == 17){
                if(j == 3 || j == 12 || j == 15 || j == 18 || j == 24 || j == 33 || j == 36
                   || j == 45 || j == 48 || j == 51 || j == 54 || j == 57)
                    board[i][j] = WALL;
                if(j == 34 || j == 35)
                    board[i][j] = CAMPSITE;
            }
            else if(i == 18){
                if(j == 3 || (j >= 6 && j <= 12) || j == 15 || (j >= 18 && j <= 21) || j == 24
                   || (j >= 27 && j <= 36) || (j >= 39 && j <= 45) || j == 48 || j == 51 || j == 54 ||
                   j == 57 || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 19){
                if(j == 3 || j == 6 || j == 15 || j == 21 || j == 24 || j == 30 || j == 36 || j == 48 ||
                   j == 51 || j == 57 || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 20){
                if(j == 3 || j == 6 || (j >= 9 && j <= 21) || (j >= 24 && j <= 27) || j == 30 ||
                   (j >= 33 && j <= 45) || j == 48 || (j >= 51 && j <= 57) || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 21){
                if(j == 3 || j == 27 || j == 30 || j == 60)
                    board[i][j] = WALL;
                if((j >= 12 && j <= 15) )
                    board[i][j] = BUSH;
            }
            else if(i == 22){
                if(j == 3 || (j >= 6 && j <= 27) || (j >= 30 && j <= 33) || (j >= 36 && j <= 42) || (j >= 45 && j <= 54) ||
                   (j >= 57 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 23){
                if(j == 3 || j == 12 || j == 21 || j == 33 || j == 36 || j == 54 || j == 57)
                    board[i][j] = WALL;
            }
            else if(i == 24){
                if((j >= 3 && j <= 9) || j == 12 || j == 15 || j == 18 || j == 21 || (j >= 24 && j <= 27)
                   || (j >= 30 && j <= 33) || (j >= 36 && j <= 39) || (j >= 42 && j <= 45) ||
                   (j >= 48 && j <= 54) || j == 57 || (j >= 60 && j <= 62))
                    board[i][j] = WALL;
            }
            else if(i == 25){
                if(j == 6 || j == 12 || j == 15 || j == 18 || j == 21 || j == 24 ||
                   j == 39 || j == 42 || j == 48 || j == 54 || j == 57)
                    board[i][j] = WALL;
            }
            else if(i == 26){
                if((j >= 3 && j <= 6) || (j >= 9 && j <= 12) || j == 15 || j == 18 || j == 21
                   || (j >= 24 && j <= 39) || j == 42 || (j >= 45 && j <= 48) || j == 51 || j == 54
                   || (j >= 57 && j <= 62))
                    board[i][j] = WALL;
            }
            else if(i == 27){
                if(j == 12 || j == 15 || j == 18 || j == 21 || j == 39 || j == 42 ||
                   j == 51 || j == 54 )
                    board[i][j] = WALL;
                if((j >= 25 && j <= 28))
                    board[i][j] = BUSH;
            }
            else if(i == 28){
                if((j >= 1 && j <= 9) || (j >= 12 && j <= 15) || j == 18 || (j >= 21 && j <= 36)
                   || j == 39 || j == 42 || (j >= 45 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 29){
                if(j == 18 || j == 42)
                    board[i][j] = WALL;
            }
        }
    }
}

void Server::paint_board(){
    move(0,0);
    for (int i = 0; i < BOARD_ROWS; i++) {
        for (int j = 0; j < BOARD_COLS; j++) {
            switch(board[i][j]){
                case FREE:
                    attron(COLOR_PAIR(TEXT_COLOR));
                    addch(' ');
                    attroff(COLOR_PAIR(TEXT_COLOR));
                    break;
                case WALL:
                    attron(COLOR_PAIR(WALL_COLOR));
                    addch(' ');
                    attroff(COLOR_PAIR(WALL_COLOR));
                    break;
                case BUSH:
                    attron(COLOR_PAIR(TEXT_COLOR));
                    addch('#');
                    attroff(COLOR_PAIR(TEXT_COLOR));
                    break;
                case COIN:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('c');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case TREASURE:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('t');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case TREASURE_LARGE:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('T');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case TREASURE_DROP:
                    attron(COLOR_PAIR(COIN_COLOR));
                    addch('D');
                    attroff(COLOR_PAIR(COIN_COLOR));
                    break;
                case CAMPSITE:
                    attron(COLOR_PAIR(CAMPSITE_COLOR));
                    addch('A');
                    attroff(COLOR_PAIR(CAMPSITE_COLOR));
                    break;
            }
        }
        addch('\n');
    }
    refresh();
}

void Server::paint_scoreboard(){
    mvprintw(0, MARGIN_SCOREBOARD, "Server PID: %d", server_PID);
    mvprintw(1, MARGIN_SCOREBOARD, "Campsite X/Y: %d/%d", campfire.x, campfire.y);
    mvprintw(2, MARGIN_SCOREBOARD, "Round number: %d", round_number);
    mvprintw(4, MARGIN_SCOREBOARD, "Parameter:");
    mvprintw(5, MARGIN_SCOREBOARD+2, "PID");
    mvprintw(6, MARGIN_SCOREBOARD+2, "Type");
    mvprintw(7, MARGIN_SCOREBOARD+2, "Curr X/Y");
    mvprintw(8, MARGIN_SCOREBOARD+2, "Deaths");
    mvprintw(9, MARGIN_SCOREBOARD+2, "Coins");
    mvprintw(10, MARGIN_SCOREBOARD+4, "carried");
    mvprintw(11, MARGIN_SCOREBOARD+4, "brought");

    for(int i = 0; i < CLIENT_LIMIT; i++){
        if(connected_clients[i]){
            mvprintw(4, MARGIN_SCOREBOARD+15+i*10, "Player %d", i+1);
            mvprintw(5, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].client_port);
            mvprintw(6, MARGIN_SCOREBOARD+15+i*10, "HUMAN");
            mvprintw(7, MARGIN_SCOREBOARD+15+i*10, "%d/%d  ", clients[i].player.left.x, clients[i].player.left.y);
            mvprintw(8, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].deaths);
            mvprintw(10, MARGIN_SCOREBOARD+15+i*10, "%d  ", clients[i].coins_carried);
            mvprintw(11, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].coins_brought);
        }
        else{
            mvprintw(4, MARGIN_SCOREBOARD+15+i*10, "Player %d", i+1);
            mvprintw(5, MARGIN_SCOREBOARD+15+i*10, "-");
            mvprintw(6, MARGIN_SCOREBOARD+15+i*10, "-");
            mvprintw(7, MARGIN_SCOREBOARD+15+i*10, "--/--", clients[i].player.left.x, clients[i].player.left.y);
            mvprintw(8, MARGIN_SCOREBOARD+15+i*10, "-", clients[i].deaths);
            mvprintw(10, MARGIN_SCOREBOARD+15+i*10, "-", clients[i].coins_carried);
            mvprintw(11, MARGIN_SCOREBOARD+15+i*10, "-", clients[i].coins_brought);
        }
    }

    mvprintw(13, MARGIN_SCOREBOARD, "Legend:");

    attron(COLOR_PAIR(PLAYER_COLOR));
    mvprintw(14, MARGIN_SCOREBOARD, "%d%d%d%d", 1, 2, 3, 4);
    attroff(COLOR_PAIR(PLAYER_COLOR));
    mvprintw(14, MARGIN_SCOREBOARD+6, "- players");

    attron(COLOR_PAIR(WALL_COLOR));
    mvprintw(15, MARGIN_SCOREBOARD, " ");
    attroff(COLOR_PAIR(WALL_COLOR));
    mvprintw(15, MARGIN_SCOREBOARD+6, "- wall");

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(16, MARGIN_SCOREBOARD, "#");
    attroff(COLOR_PAIR(TEXT_COLOR));
    mvprintw(16, MARGIN_SCOREBOARD+6, "- bushes (slow down)");

    attron(COLOR_PAIR(TEXT_COLOR));
    mvprintw(17, MARGIN_SCOREBOARD, "*");
    attroff(COLOR_PAIR(TEXT_COLOR));
    mvprintw(17, MARGIN_SCOREBOARD+6, "- beast");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(18, MARGIN_SCOREBOARD, "c");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(18, MARGIN_SCOREBOARD+6, "- one coin");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(19, MARGIN_SCOREBOARD, "t");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(19, MARGIN_SCOREBOARD+6, "- treasure (10 coins)");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(20, MARGIN_SCOREBOARD, "T");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(20, MARGIN_SCOREBOARD+6, "- large treasure (50 coins)");

    attron(COLOR_PAIR(COIN_COLOR));
    mvprintw(21, MARGIN_SCOREBOARD, "D");
    attroff(COLOR_PAIR(COIN_COLOR));
    mvprintw(21, MARGIN_SCOREBOARD+6, "- dropped treasure");

    attron(COLOR_PAIR(CAMPSITE_COLOR));
    mvprintw(22, MARGIN_SCOREBOARD, "A");
    attroff(COLOR_PAIR(CAMPSITE_COLOR));
    mvprintw(22, MARGIN_SCOREBOARD+6, "- campsite");

    refresh();
}

int Server::count_clients_connected() {
    int client_nr = 0;
    for(bool connected_client : connected_clients){
        if(connected_client)
            client_nr++;
    }
    return client_nr;
}

int Server::get_first_free_client() {
    if(count_clients_connected() == CLIENT_LIMIT)
        return -1;
    for(int i = 0; i < CLIENT_LIMIT; i++)
        if(!connected_clients[i])
            return i;
    return -1;
}

void Server::paint_players(){
    for(int i = 0; i < CLIENT_LIMIT; i++){
        if(connected_clients[i]){
            attron(COLOR_PAIR(PLAYER_COLOR));
            mvprintw(clients[i].player.left.y, clients[i].player.left.x, "%d%d", i + 1, i + 1);
            attroff(COLOR_PAIR(PLAYER_COLOR));
        }
    }
    refresh();
}

void Server::paint_beasts(){
    for(auto &x : beasts){
        attron(COLOR_PAIR(TEXT_COLOR));
        mvaddstr(x.left.y, x.left.x, "**");
        attroff(COLOR_PAIR(TEXT_COLOR));
    }
    refresh();
}

void Server::handle_input(Server *server, int input) {
    if(input == 'b' || input == 'B')
        add_beast(server);
    if(input == 'c')
        add_collectible(1);
    if(input == 't')
        add_collectible(10);
    if(input == 'T')
        add_collectible(50);
}

void Server::add_beast(Server *server) {
    int x = rand() % (BOARD_COLS - 2) + 1; // todo rand omijajacy playerow
    int y = rand() % (BOARD_ROWS - 2) + 1;
    if(board[y][x] == FREE && board[y][x+1] == FREE){
        threads_beast.emplace_back(std::thread(&Server::move_beast, server, x, y));
        threads_beast.back().detach();
    }
    else
        add_beast(server);
}

void Server::move_beast(int x, int y){
    beasts.emplace_back(MovingObject(x, y));
    MovingObject &beast = beasts.back();
    //sem_post(&beast_count);
    while(server_running){
        int player_id = player_is_seen(beast);
        if(player_id != -1){
            move_towards_player(beast, player_id);
            //sem_wait(&beast_sem);
            check_beast_collision(beast, player_id);
        }
        sleep(1);
    }
}

void Server::check_beast_collision(MovingObject &beast, int player_id){
    if(beast == clients[player_id].player || (beast.left.y == clients[player_id].player.left.y
    && (beast.left.x == clients[player_id].player.right.x || beast.right.x == clients[player_id].player.left.x)))
    {
        set_player_after_collision(player_id);
    }
}

int Server::player_is_seen(MovingObject &beast){ // returns which player is the closest one
    int id = -1;
    int min = BOARD_COLS;
    for(int i = 0; i < CLIENT_LIMIT; i++){
        if(connected_clients[i]){
            if(beast.left.y == clients[i].player.left.y){
                int x = 0, higher_x = 0;
                if(beast.left.x > clients[i].player.left.x){
                    higher_x = beast.left.x;
                    x = clients[i].player.right.x;
                }
                else if(beast.left.x < clients[i].player.left.x){
                    higher_x = clients[i].player.left.x;
                    x = beast.right.x;
                }
                int temp_x = x;
                for(; temp_x < higher_x; temp_x++){
                    if(board[beast.left.y][temp_x] == WALL)
                        break;
                }
                if(temp_x == higher_x){
                    if(higher_x - x < min){
                        min = higher_x - x;
                        id = i;
                    }
                }
            }
            else if(beast.left.x == clients[i].player.left.x){
                int wall_count = 0;
                int y = 0, higher_y = 0;
                if(beast.left.y > clients[i].player.left.y){
                    y = clients[i].player.left.y;
                    higher_y = beast.left.y;
                }
                else if(beast.left.y < clients[i].player.left.y){
                    higher_y = clients[i].player.left.y;
                    y = beast.left.y;
                }
                for(int temp_y = y; temp_y < higher_y; temp_y++){
                    if(board[temp_y][beast.left.x] == WALL){
                        wall_count += 1;
                        break;
                    }
                }
                for(int temp_y = y; temp_y < higher_y; temp_y++){
                    if(board[temp_y][beast.right.x] == WALL){
                        wall_count += 1;
                        break;
                    }
                }
                if(wall_count != 2){
                    if(higher_y - y < min){
                        min = higher_y - y;
                        id = i;
                    }
                }
            }
            else if(beast.right.x == clients[i].player.left.x || beast.left.x == clients[i].player.right.x){
                int x;
                if(beast.right.x == clients[i].player.left.x)
                    x = beast.right.x;
                else
                    x = beast.left.x;

                int wall_count = 0, y = 0, higher_y = 0;
                if(beast.left.y > clients[i].player.left.y){
                    y = clients[i].player.left.y;
                    higher_y = beast.left.y;
                }
                else if(beast.left.y < clients[i].player.left.y){
                    higher_y = clients[i].player.left.y;
                    y = beast.left.y;
                }
                for(int temp_y = y; temp_y < higher_y; temp_y++){
                    if(board[temp_y][x] == WALL){
                        wall_count += 1;
                        break;
                    }
                }
                if(wall_count != 1){
                    if(higher_y - y < min){
                        min = higher_y - y;
                        id = i;
                    }
                }
            }
        }
    }
    return id;
}

void Server::move_towards_player(MovingObject &beast, int player_id){
    if(clients[player_id].player.left.y == beast.left.y){
        if(beast.left.x < clients[player_id].player.left.x)
            beast += Point(1, 0);
        else
            beast += Point(-1, 0);
    }
    else if(clients[player_id].player.left.x == beast.left.x){
        if(beast.left.y < clients[player_id].player.left.y)
            beast += Point(0, 1);
        else
            beast += Point(0, -1);
    }
    else if(clients[player_id].player.left.x == beast.right.x){
        if(beast.left.y < clients[player_id].player.left.y){
            if(board[beast.left.y + 1][beast.left.x] != WALL && board[beast.left.y + 1][beast.right.x] != WALL)
                beast += Point(0, 1);
            else
                beast += Point(1, 0);
        }
        else{
            if(board[beast.left.y - 1][beast.left.x] != WALL && board[beast.left.y - 1][beast.right.x] != WALL)
                beast += Point(0, -1);
            else
                beast += Point(1, 0);
        }

    }
    else if(clients[player_id].player.right.x == beast.left.x){
        if(beast.left.y < clients[player_id].player.left.y){
            if(board[beast.left.y + 1][beast.left.x] != WALL && board[beast.left.y + 1][beast.right.x] != WALL)
                beast += Point(0, 1);
            else
                beast += Point(-1, 0);
        }
        else{
            if(board[beast.left.y - 1][beast.left.x] != WALL && board[beast.left.y - 1][beast.right.x] != WALL)
                beast += Point(0, -1);
            else
                beast += Point(-1, 0);
        }
    }
}

void Server::add_collectible(int value) {
    if(!value)
        return;
    int x = rand() % (BOARD_COLS - 2) + 1; // todo dodac rand omijajacych playerow i dla bestii
    int y = rand() % (BOARD_ROWS - 2) + 1;
    if(board[y][x] == FREE){
        add_collectible(x, y, value);
    }
    else
        add_collectible(value);
}

void Server::add_collectible(int x, int y, int value){
    if(board[y][x] == FREE && value){
        coins.emplace_back(Collectible(x, y, value));
        if(value == 1)
            board[y][x] = COIN;
        else if(value == 10)
            board[y][x] = TREASURE;
        else if(value == 50)
            board[y][x] = TREASURE_LARGE;
        else
            board[y][x] = TREASURE_DROP;
    }
}
