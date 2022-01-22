#include "game_client.h"
#include <ncurses.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

Client::Client() {

}

Client::~Client() {

}

void Client::connect() {

}

void Client::print_board() {

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

void print_board(){
    /*move(0,0);
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
    refresh();*/
}

void init_window(){
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    //nodelay(stdscr, TRUE); // disable waiting for input
    curs_set(0);
    srand(time(NULL));
}

int main(){

    init_window();

    pthread_t tid;

    pthread_create(&tid, NULL, thread_client, NULL);

    pthread_join(tid, NULL);

    return 0;
}