#include "game_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ncurses.h>

Server::Server() { // todo error throwing
    init_window();
    if(!has_colors() || check_size_terminal()){
        printf("Invalid size of terminal (min.%dx%d) or no colors support!\nCheck you terminal preferences!"
                , MARGIN_SCOREBOARD + SCOREBOARD_WIDTH, BOARD_ROWS);
    }
    init_colors();
    set_board();

}

void Server::start() {

    paint_board();
    paint_scoreboard();
    paint_players();

    while(true){
        int input = getch();
        flushinp();

        if(input == 'q' || input == 'Q')
            break;

        handle_input(input);
        move_beasts();

        paint_board();
        paint_scoreboard();
        paint_players();
        paint_beasts();
        //sleep(1);
    }

}

Server::~Server() {
    beasts.clear();
    coins.clear();
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
                    (j >= 51 && j <= 61))
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
                if(j == 6 || j == 9 || j == 15 || j == 24 || j == 30 || j == 33 ||
                   j == 45 || j == 48 || j == 54 || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 6){
                if( (j >= 3 && j <= 6) || j == 9 || (j >= 12 && j <= 15) || (j >= 18 && j <= 36) ||
                    (j >= 39 && j <= 45) || (j >= 48 && j <= 57) || j == 60 )
                    board[i][j] = WALL;
            }
            else if(i == 7){
                if( j == 3 || j == 9 || j == 12 || j == 18 || j == 36 || j == 39 || j == 45
                    || j == 54)
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
                    (j >= 24 && j <= 27) || j == 30 || (j >= 33 && j <= 48) || j == 51 ||
                    (j >= 51 && j <= 62))
                    board[i][j] = WALL;
            }
            else if(i == 11){
                if( j == 3 || j == 9 || j == 12 || j == 27 || j == 30 || j == 45 ||
                    j == 48 || j == 60)
                    board[i][j] = WALL;
            }
            else if(i == 12){
                if( j == 3 || (j >= 6 && j <= 9) || (j >= 12 && j <= 24) || j == 27 ||
                    (j >= 30 && j <= 42) || j == 45 || (j >= 48 && j <= 51) || (j >= 54 && j <= 57)
                    || j == 60 )
                    board[i][j] = WALL;
            }
            else if(i == 13){
                if(j == 3 || j == 6 || j == 27 || j == 42 || j == 45 || j == 51 || j == 54
                   || j == 57)
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
                if(j == 3 || j == 12 || j == 18 || j == 39 || j == 42 || j == 48 || j == 51
                   || j == 57)
                    board[i][j] = WALL;
            }
            else if(i == 16){
                if((j >= 3 && j <= 9) || j == 12 || j == 15 || j == 18 || (j >= 21 && j <= 24) ||
                   (j >= 27 && j <= 33) || (j >= 36 && j <= 42) || j == 45 || j == 48 ||
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
                   (j >= 33 && j <= 45) || j == 48 || (j >= 51 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 21){
                if(j == 3 || j == 27 || j == 30 || j == 36 ||  j == 48  || j == 60)
                    board[i][j] = WALL;
                if((j >= 12 && j <= 15) )
                    board[i][j] = BUSH;
            }
            else if(i == 22){
                if(j == 3 || (j >= 6 && j <= 33) || (j >= 36 && j <= 42) || (j >= 45 && j <= 54) ||
                   (j >= 57 && j <= 60))
                    board[i][j] = WALL;
            }
            else if(i == 23){
                if(j == 3 || j == 12 || j == 18 || j == 21 || j == 33 || j == 36 || j == 54 || j == 57)
                    board[i][j] = WALL;
            }
            else if(i == 24){
                if((j >= 1 && j <= 9) || j == 12 || j == 15 || j == 18 || j == 21 || (j >= 24 && j <= 27)
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
                if(j == 18 || j == 42 || j == 57)
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
    mvprintw(0, MARGIN_SCOREBOARD, "Server PID: %d", 1370);
    mvprintw(1, MARGIN_SCOREBOARD, "Campsite X/Y: %d/%d", 23, 11);
    mvprintw(2, MARGIN_SCOREBOARD, "Round number: %d", 3324);
    mvprintw(4, MARGIN_SCOREBOARD, "Parameter:");
    mvprintw(5, MARGIN_SCOREBOARD+2, "PID");
    mvprintw(6, MARGIN_SCOREBOARD+2, "Type");
    mvprintw(7, MARGIN_SCOREBOARD+2, "Curr X/Y");
    mvprintw(8, MARGIN_SCOREBOARD+2, "Deaths");
    mvprintw(9, MARGIN_SCOREBOARD+2, "Coins");
    mvprintw(10, MARGIN_SCOREBOARD+4, "carried");
    mvprintw(11, MARGIN_SCOREBOARD+4, "brought");

    for(int i = 0; i < CLIENT_LIMIT; i++){
        mvprintw(4, MARGIN_SCOREBOARD+15+i*10, "Player %d", clients[i].player_id);
        mvprintw(5, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].client_port);
        mvprintw(6, MARGIN_SCOREBOARD+15+i*10, "HUMAN");
        mvprintw(7, MARGIN_SCOREBOARD+15+i*10, "%d/%d", clients[i].player_position.x, clients[i].player_position.y);
        mvprintw(8, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].deaths);
        mvprintw(10, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].coins_carried);
        mvprintw(11, MARGIN_SCOREBOARD+15+i*10, "%d", clients[i].coins_brought);
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

void Server::paint_players(){

}

void Server::paint_beasts(){
    for(auto &x : beasts){
        attron(COLOR_PAIR(TEXT_COLOR));
        mvaddstr(x.left.y, x.left.x, "**");
        attroff(COLOR_PAIR(TEXT_COLOR));
    }
    refresh();
}

void Server::handle_input(int input) {
    if(input == 'b' || input == 'B')
        add_beast();
    if(input == 'c')
        add_collectible(1);
    if(input == 't')
        add_collectible(50);
    if(input == 'T')
        add_collectible(100);
}

/*int Server::handle_event(int input){
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
            return 1;
    }
    left = position.left + point;
    right = position.right + point;
    switch (input) { // todo dodanie kolizji z bestiami i graczami, slow z busha i zbieranie coinow i oddawanie (campsite)
        case KEY_UP:
        case KEY_DOWN:
            if(board[left.y][left.x] != WALL && board[right.y][right.x] != WALL){
                position.left = left;
                position.right = right;
                return 0;
            }
            return 1;
        case KEY_LEFT:
            if(board[left.y][left.x] != WALL){
                position.left = left;
                position.right = right;
                return 0;
            }
            return 1;
        case KEY_RIGHT:
            if(board[right.y][right.x] != WALL){
                position.left = left;
                position.right = right;
                return 0;
            }
            return 1;
        default:
            return 1;
    }
    return 1;
}*/

void Server::add_beast() {
    int x = rand() % (BOARD_COLS - 2) + 1; // todo rand omijajacy playerow
    int y = rand() % (BOARD_ROWS - 2) + 1;
    if(board[y][x] == FREE && board[y][x+1] == FREE){
        beasts.emplace_back(MovingObject(x, y));
    }
    else
        add_beast();
}

void Server::add_collectible(int value) {
    int x = rand() % (BOARD_COLS - 2) + 1; // todo dodac rand omijajacych playerow i dla bestii
    int y = rand() % (BOARD_ROWS - 2) + 1; // todo dropped treasure niewpisywany?
    if(board[y][x] == FREE){
        add_collectible(x, y, value);
    }
    else
        add_collectible(value);
}

void Server::add_collectible(int x, int y, int value){
    if(board[y][x] == FREE){
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

void Server::move_beasts() {

}

int main(){
    Server server = Server();
    server.start();
}


// testing
/*

void *starting(void *param) {
    int *temp = (int*) param;
    char testing[5][10] = {1};
    while(true){
        int choice;
        int res = recv(*temp, &choice, sizeof(choice), 0);
        printf("Client %d entered character: %d\n", *temp, choice);
        if(res == SOCKET_ERROR){
            printf("Error reading socket data!");
            exit(1);
        }
        if(choice == 'q')
            break;
        send(*temp, testing, sizeof(testing), 0);
    }
    close(*temp);
    pthread_exit(NULL);
}

int main(){
    SA_IN server_addr;
    socklen_t addr_size;

    int i = 0;
    int server_socket;
    SA_IN client_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == SOCKET_ERROR){
        printf("Failed to create stream!");
        exit(1);
    }

    // terminating program won't trigger bind failed
    int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == SOCKET_ERROR)
        printf("Setsockopt(SO_REUSEADDR) failed!\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    int result = bind(server_socket, (SA*)&server_addr, sizeof(server_addr));
    if(result == SOCKET_ERROR){
        printf("Bind failed!");
        exit(1);
    }
    result = listen(server_socket, SERVER_BACKLOG);
    if(result == SOCKET_ERROR){
        printf("Listening failed!");
        exit(1);
    }

    pthread_t tid[10];
    int client_socket[10] = {0};
    printf("Waiting for connections...\n");

    while(true){
        addr_size = sizeof(SA_IN);
        client_socket[i] = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size);
        if(client_socket[i] == SOCKET_ERROR){
            printf("Error connecting client to server!");
            exit(1);
        }
        printf("Connected %d!\n", client_socket[i]);

        pthread_create(&tid[i], NULL, starting, &client_socket[i]);
        i++;
    }
}*/
