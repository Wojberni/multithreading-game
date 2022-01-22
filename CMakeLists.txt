cmake_minimum_required(VERSION 3.20)
project(multithreading_game)

set(CMAKE_CXX_STANDARD 14)

add_executable(curse_test curses.cpp)
add_executable(game_server game_server.cpp)
add_executable(game_client game_client.cpp)

target_link_libraries(curse_test -lncurses)
target_link_libraries(game_server -lpthread -lncurses)
target_link_libraries(game_client -lpthread -lncurses)


