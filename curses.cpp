#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <time.h>
#include "point.h"
#include <pthread.h>

#define BOARD_ROWS 31
#define BOARD_COLS 64
#define SCOREBOARD_WIDTH 50
#define MARGIN_SCOREBOARD BOARD_COLS + 5

enum legend {FREE, WALL, BUSH, COIN, TREASURE, TREASURE_LARGE, TREASURE_DROP, CAMPSITE};
enum colors {WALL_COLOR = 1, COIN_COLOR, CAMPSITE_COLOR, PLAYER_COLOR, TEXT_COLOR};
int rows = 0, columns = 0;
char board[BOARD_ROWS][BOARD_COLS] = {FREE};

std::vector<Collectible> coins;
std::vector<MovingObject> beasts;

MovingObject player = MovingObject(1, 1); // todo poprzerabiac tak aby bylo na 4 graczy (klasa gracz?)
//MovingObject players[4];

void init_window();
int check_size_terminal();
void init_colors();
void set_board();
void paint_board();
void paint_scoreboard();
int handle_event(MovingObject &position, int input);
void paint_player(MovingObject &position);
void add_collectible(int value);
void add_collectible(int x, int y, int value);
void add_beast();
void delete_beasts();
void move_beasts();
void paint_beasts();
int beast_sees_player(MovingObject &beast);

int main()
{
    init_window();

    if(!has_colors() || check_size_terminal()){
        printf("Invalid size of terminal (min.%dx%d) or no colors support!\nCheck you terminal preferences!"
               , MARGIN_SCOREBOARD + SCOREBOARD_WIDTH, BOARD_ROWS);
        sleep(10);
        return EXIT_FAILURE;
    }

    init_colors();
    set_board();

    paint_board();
    paint_scoreboard();
    paint_player(player);

    while(true){
        int input = getch();
        flushinp();

        if(input == 'q' || input == 'Q')
            break;

        move_beasts();
        handle_event(player, input);

        paint_board();
        paint_scoreboard();
        paint_player(player);
        paint_beasts();
        //sleep(1);
    }

    delete_beasts();
    coins.clear();
    endwin();
    return EXIT_SUCCESS;
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

void init_colors(){
    start_color();
    use_default_colors();
    init_pair(WALL_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(COIN_COLOR, COLOR_BLACK, COLOR_YELLOW);
    init_pair(CAMPSITE_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(PLAYER_COLOR, COLOR_WHITE, COLOR_BLUE);
    init_pair(TEXT_COLOR, COLOR_BLACK, COLOR_WHITE);
}

int check_size_terminal(){
    getmaxyx(stdscr, rows, columns);
    if(rows < BOARD_ROWS || columns < BOARD_COLS + SCOREBOARD_WIDTH)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void set_board(){
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

void paint_board(){
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

void paint_scoreboard(){
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

    for(int i = 0; i < 4; i++){
        mvprintw(4, MARGIN_SCOREBOARD+15+i*10, "Player %d", i+1);
        mvprintw(5, MARGIN_SCOREBOARD+15+i*10, "%d", 2000);
        mvprintw(6, MARGIN_SCOREBOARD+15+i*10, "HUMAN");
        mvprintw(7, MARGIN_SCOREBOARD+15+i*10, "%d/%d", 12, 13);
        mvprintw(8, MARGIN_SCOREBOARD+15+i*10, "%d", 5);
        mvprintw(10, MARGIN_SCOREBOARD+15+i*10, "%d", 5);
        mvprintw(11, MARGIN_SCOREBOARD+15+i*10, "%d", 5);
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

int handle_event(MovingObject &position, int input){
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
        case 'c':
            add_collectible(1);
            return 0;
        case 't':
            add_collectible(50);
            return 0;
        case 'T':
            add_collectible(100);
            return 0;
        case 'b':
        case 'B':
            add_beast();
            return 0;
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
}

void paint_player(MovingObject &position){
    attron(COLOR_PAIR(PLAYER_COLOR));
    mvaddstr(position.left.y, position.left.x, "11");
    attroff(COLOR_PAIR(PLAYER_COLOR));
    refresh();
}

void add_collectible(int value){
    int x = rand() % (BOARD_COLS - 2) + 1; // todo dodac rand omijajacych playerow i dla bestii
    int y = rand() % (BOARD_ROWS - 2) + 1; // todo dropped treasure niewpisywany?
    if(board[y][x] == FREE){
        add_collectible(x, y, value);
    }
    else
        add_collectible(value);
}

void add_collectible(int x, int y, int value){
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

void add_beast(){
    int x = rand() % (BOARD_COLS - 2) + 1; // todo rand omijajacy playerow
    int y = rand() % (BOARD_ROWS - 2) + 1;
    if(board[y][x] == FREE && board[y][x+1] == FREE){
        beasts.emplace_back(MovingObject(x, y));
    }
    else
        add_beast();
}

void delete_beasts(){
    beasts.clear();
}

void paint_beasts(){
    for(auto &x : beasts){
        attron(COLOR_PAIR(TEXT_COLOR));
        mvaddstr(x.left.y, x.left.x, "**");
        attroff(COLOR_PAIR(TEXT_COLOR));
    }
    refresh();
}

void move_beasts(){
    for(auto &x : beasts){
        int move_dir = beast_sees_player(x);
        if(move_dir != -1)
            handle_event(x, move_dir);
        else{
            int valid_move = 1;
            while(valid_move){
                move_dir = rand() % 4 + KEY_DOWN; // lowest value ascii is key down
                valid_move = handle_event(x, move_dir);
            }
        }
    }
}

int beast_sees_player(MovingObject &beast){ // todo refactor???
    if(beast.left.y == player.left.y){
        if(beast.left.x < player.left.x){
            for(int i = beast.left.x; i < player.left.x; i++){
                if(board[beast.left.x][i] == WALL)
                    return -1;
            }
            return KEY_RIGHT;
        }
        else if(beast.left.x > player.left.x){
            for(int i = player.left.x; i < beast.left.x; i++){
                if(board[beast.left.x][i] == WALL)
                    return -1;
            }
            return KEY_LEFT;
        }
        return -1;
    }
    else if(beast.left.x == player.right.x){
        if(beast.left.y < player.left.y){
            for(int i = beast.left.y; i < player.left.y; i++){
                if(board[i][beast.left.x] == WALL)
                    return -1;
            }
            return KEY_DOWN;
        }
        else if(beast.left.y > player.left.y){
            for(int i = player.left.y; i < beast.left.y; i++){
                if(board[i][beast.left.x] == WALL)
                    return -1;
            }
            return KEY_UP;
        }
        return -1;
    }
    else if(beast.right.x == player.left.x){
        if(beast.left.y < player.left.y){
            for(int i = beast.left.y; i < player.left.y; i++){
                if(board[i][beast.right.x] == WALL)
                    return -1;
            }
            return KEY_DOWN;
        }
        else if(beast.left.y > player.left.y){
            for(int i = player.left.y; i < beast.left.y; i++){
                if(board[i][beast.right.x] == WALL)
                    return -1;
            }
            return KEY_UP;
        }
        return -1;
    }
    else if(beast.left.x == player.left.x && beast.right.x == player.right.x){
        if(beast.left.y < player.left.y){
            for(int i = beast.left.y; i < player.left.y; i++){
                if(board[i][beast.left.x] == WALL)
                    return -1;
            }
            return KEY_DOWN;
        }
        else if(beast.left.y > player.left.y){
            for(int i = player.left.y; i < beast.left.y; i++){
                if(board[i][beast.left.x] == WALL)
                    return -1;
            }
            return KEY_UP;
        }
        return -1;
    }
    return -1;
}
