//
// Created by carson on 5/20/20.
//

#include "stdio.h"
#include "stdlib.h"
#include "server.h"
#include "char_buff.h"
#include "game.h"
#include "repl.h"
#include "pthread.h"
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h>    //inet_addr
#include<unistd.h>    //write

static game_server *SERVER;

void init_server() {
    if (SERVER == NULL) {
        SERVER = calloc(1, sizeof(struct game_server));
    } else {
        printf("Server already started");
    }
}

int handle_client_connect(int player) {
    // STEP 9 - This is the big one: you will need to re-implement the REPL code from
    // the repl.c file, but with a twist: you need to make sure that a player only
    // fires when the game is initialized and it is there turn.  They can broadcast
    // a message whenever, but they can't just shoot unless it is their turn.
    //
    // The commands will obviously not take a player argument, as with the system level
    // REPL, but they will be similar: load, fire, etc.
    //
    // You must broadcast informative messages after each shot (HIT! or MISS!) and let
    // the player print out their current board state at any time.
    //
    // This function will end up looking a lot like repl_execute_command, except you will
    // be working against network sockets rather than standard out, and you will need
    // to coordinate turns via the game::status field.

    char_buff * command;

    do {
        // This is the classic Read, Evaluate, Print Loop, hence REPL
        command = repl_read_command("battleBit (? for help) > ");
        char* command = cb_tokenize(buffer, " \n");
        if (command) {
            char* arg1 = cb_next_token(buffer);
            char* arg2 = cb_next_token(buffer);
            char* arg3 = cb_next_token(buffer);
            if (strcmp(command, "exit") == 0) {
                printf("goodbye!");
                exit(EXIT_SUCCESS);
            } else if(strcmp(command, "?") == 0) {
                printf("? - show help\n");
                printf("load [0-1] <string> - load a ship layout file for the given player\n");
                printf("show [0-1] - shows the board for the given player\n");
                printf("fire [0-1] [0-7] [0-7] - fire at the given position\n");
                printf("say <string> - Send the string to all players as part of a chat\n");
                printf("reset - reset the game\n");
                printf("server - start the server\n");
                printf("exit - quit the server\n");
            } else if(strcmp(command, "server") == 0) {
                server_start();
            } else if(strcmp(command, "show") == 0) {
                int intCommand;
                //intCommand = atoi(command);
                // work with repl_print_board
                repl_print_board(game_get_current(), atoi(arg1), buffer);

            } else if(strcmp(command, "reset") == 0) {

                game_init();

            } else if (strcmp(command, "load") == 0) {

                // work with game_load_board
                game_load_board(game_get_current(), atoi(arg1), arg2);

            } else if (strcmp(command, "fire") == 0) {

                // work with game_fire
                game_fire(game_get_current(), atoi(arg1), atoi(arg2), atoi(arg3));

            } else if (strcmp(command, "nasm") == 0) {
                //nasm_hello_world();
            } else if (strcmp(command, "shortcut") == 0) {
                // update player 1 to only have a single ship in position 0, 0
                game_get_current()->players[1].ships = 1ull;
            } else {
                printf("Unknown Command: %s\n", command);
            }
        }
        cb_free(command);
    } while (command);
}

void server_broadcast(char_buff *msg) {
    // send message to all players


}

int run_server() {
    // STEP 8 - implement the server code to put this on the network.
    // Here you will need to initalize a server socket and wait for incoming connections.
    //
    // When a connection occurs, store the corresponding new client socket in the SERVER.player_sockets array
    // as the corresponding player position.
    //
    // You will then create a thread running handle_client_connect, passing the player number out
    // so they can interact with the server asynchronously

    // creates socket
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_fd == -1) {
        printf("Can't create socket\n");
    }

    //resuse port
    int yes = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in server;

    //socket information
    server.sin_family = AF_INET;

    // bind socket
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9876);

    int request = 0;
    if ( bind(server_socket_fd, (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("BIND FAILED\n");
    } else {
        puts("BIND WORKED\n");
        listen(server_socket_fd, 3);
    }

    puts("WAITING FOR CONNECTION\n");

    // client socket descrption
    struct sockaddr_in client;
    socklen_t size_from_connect;
    int client_socket_fd;
    //int request_count = 0;
    int count = 0;

    while((client_socket_fd = accept(server_socket_fd,
                                     (struct sockaddr *) &client, &size_from_connect)) > 0) {

        // how to tell which player? Do I need an if statement?
        //int opponent = (player + 1) % 2;

        SERVER->player_sockets[count] = client_socket_fd;
        //SERVER->player_sockets[1] = client_socket_fd;


        pthread_create(&SERVER->player_threads[count], NULL, handle_client_connect, &count);

        count +=1;

        /*char message[100] = {0};
        sprintf(message, "blaw blaw blaw - req %d\n\n", request_count++);
        send(client_socket_fd, message, strlen(message), 0);
        close(client_socket_fd); */
    }

    //port 9876
    // new p thread to handle client connect, pass in player
}

int server_start() {
    // STEP 7 - using a pthread, run the run_server() function asynchronously, so you can still
    // interact with the game via the command line REPL

    init_server();
    //pthread_t game_thread;
    //pthread_create(&tid1, NULL, game_init, 2);
    // or its backwards..
    pthread_create(&SERVER->server_thread, NULL, run_server, NULL);

    // lock and unlock before and after critical code
    //pthread_mutex_t lock;
    //pthread_mutex_init(&lock, NULL);
    //pthread_mutex_lock(&lock);
    //pthread_mutex_unlock(&lock);
    //pthread_mutex_destroy(&lock);
}
