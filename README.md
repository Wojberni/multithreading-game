
# Multithreading game

This game is a classic treasure hunt game. The difference is we have a server and clients, so we can play with our friends. The goal is to bring as many coins as you can to the campsite. Good luck on your treasure hunting!
## Screenshots

![Client](https://github.com/Wojberni/multithreading-game/blob/feature/cgame/client.png?raw=true)

Client's view

![Server](https://github.com/Wojberni/multithreading-game/blob/feature/cgame/server.png?raw=true)

Server's view
## Tech Stack

**Client:** C++ and C semaphores

**Server:** C++ and C semaphores


## FAQ

#### How do you run this game?

Look at Run program section. First run server then run the clients.

#### How many players can play the game?

The game can play up to 4 players. Next players joining won't be able to play the game and their terminal is likely to crash.

#### What are the controls for this game?

Server can input beasts by pressing b/B button on keyboard. He can also add coins by pressing c/t/T. The players move with the arrow keys.

#### How do you end the game as a client or a server?

Just press q/Q button to end the client or the server.

#### What are the important rules of this game?

First you must be aware of many threats that are hiding behind corners. Collision with either player or beast will result in death. After dying, you lose all the coins you was carrying. So be careful when wandering with a bag full of coins, better leave them in campsite rather than lose them all.

#### What are the differences between types of treasure? 

Different types of treasure give players different amount of coins. So you should always look, wheather it is one coin or a chest of coins. There is a legend for newbies, so you won't get lost.

#### Why players see only a small portion of map?

To make a game more difficult.


## Installation

You must have gcc build-essential and ncurses library installed. You must run it on Linux, because I used POSIX semaphores, which are specific for Unix.

```bash
  sudo apt-get update
  sudo apt-get install build-essential libncurses5-dev libncursesw5-dev
```

## Run program

Now you have to use Makefile to compile and run the client and the server. 

```bash
  make all
  cd objs
  ./server 
  ./client
```

And after quiting the server just delete the directory with the compiled programs.

```bash
  make clean
```


## 🚀 About Me
I'm a Computer Science student and I am on my last year of TUOL in Łódź. This is one of the projects, that I was really working hard to look good and I am happy how it came out.

If you liked my project or you found a bug in it, leave a comment if you like. I am only a human, I make mistakes.

