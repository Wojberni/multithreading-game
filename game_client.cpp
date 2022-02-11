#include "game_client.h"
#include <ncurses.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

Client::Client() {

}

int Client::init(){
    init_window();
    if(!has_colors() || check_size_terminal()){
        printw("Invalid size of terminal (min.%dx%d) or no colors support!\nCheck you terminal preferences!\n"
                , MARGIN_SCOREBOARD + SCOREBOARD_WIDTH, BOARD_ROWS);
        return EXIT_FAILURE;
    }
    init_colors();
    return EXIT_SUCCESS;
}

Client::~Client() {

}

void Client::init_window() {
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    //nodelay(stdscr, TRUE); // disable waiting for input
    curs_set(0);
}

void Client::init_colors() {
    start_color();
    use_default_colors();
    init_pair(WALL_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(COIN_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CAMPSITE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(PLAYER_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);
}

bool Client::check_size_terminal() {
    getmaxyx(stdscr, rows, columns);
    if (rows < BOARD_ROWS || columns < BOARD_COLS + SCOREBOARD_WIDTH)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;

}

int Client::connection() {
    network_socket = socket(AF_INET,SOCK_STREAM, 0);

    SA_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    int connection_status = connect(network_socket,(struct sockaddr*)&server_address, sizeof(server_address));

    if (connection_status == SOCKET_ERROR) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void Client::print_board() {
    move(0,0);
    for (int i = 0; i < CLIENT_ROWS; i++) {
        for (int j = 0; j < CLIENT_COLS; j++) {
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

void Client::print_scoreboard(){
    mvprintw(0, MARGIN_SCOREBOARD, "Server PID: %d", 1370);
    mvprintw(1, MARGIN_SCOREBOARD, "Campsite X/Y: unknown");
    mvprintw(2, MARGIN_SCOREBOARD, "Round number: %d", 3324);
    mvprintw(4, MARGIN_SCOREBOARD, "Player:");
    mvprintw(5, MARGIN_SCOREBOARD+2, "Number");
    mvprintw(6, MARGIN_SCOREBOARD+2, "Type");
    mvprintw(7, MARGIN_SCOREBOARD+2, "Curr X/Y");
    mvprintw(8, MARGIN_SCOREBOARD+2, "Deaths");
    mvprintw(10, MARGIN_SCOREBOARD+2, "Coins carried");
    mvprintw(11, MARGIN_SCOREBOARD+2, "Coins brought");

    mvprintw(5, MARGIN_SCOREBOARD+15, "%d", client.player_id);
    mvprintw(6, MARGIN_SCOREBOARD+15, "HUMAN");
    mvprintw(7, MARGIN_SCOREBOARD+15, "%d/%d", client.player_position.x, client.player_position.y);
    mvprintw(8, MARGIN_SCOREBOARD+15, "%d", client.deaths);
    mvprintw(10, MARGIN_SCOREBOARD+15, "%d", client.coins_carried);
    mvprintw(11, MARGIN_SCOREBOARD+15, "%d", client.coins_brought);

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

void Client::play_game() {
    print_board();
    print_scoreboard();
    while(true){
        int input = getch();
        flushinp();
        send(network_socket, &input, sizeof(input), 0);
        if(input == 'q' || input == 'Q')
            break;
        recv(network_socket, &client, sizeof(client), 0);
        print_board();
        print_scoreboard();
        sleep(1);
    }
    close(network_socket);
}

int main(){

    Client *client = new Client();
    if(client->init() == EXIT_FAILURE){
        printw("Error initializing client!\n");
        return EXIT_FAILURE;
    }
    if(client->connection() == EXIT_FAILURE){
        printw("Error connecting to the server!\n");
        return EXIT_FAILURE;
    }
    client->play_game();
    delete client;
    return 0;
}


void *thread_client(void *param){

    int network_socket = socket(AF_INET,SOCK_STREAM, 0);

    SA_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);

    int connection_status = connect(network_socket,(struct sockaddr*)&server_address, sizeof(server_address));

    if (connection_status == SOCKET_ERROR) {
        puts("Error\n");
        return NULL;
    }

    //printf("Connection established %d!\n", network_socket);

    char test[5][10];
    while(true){
        //printf("Enter a character!: ");
        int input = getch();
        flushinp();
        send(network_socket, &input, sizeof(input), 0);
        if(input == 'q' || input == 'Q')
            break;
        recv(network_socket, &test, sizeof(test), 0);
        mvprintw(3,0, "%d %d %d", test[0][0], test[1][0], test[2][0]);
    }

    close(network_socket);
    return NULL;
}