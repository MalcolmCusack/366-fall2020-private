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
static pthread_mutex_t *LOCK;

void init_server() {
    if (SERVER == NULL) {
        SERVER = calloc(1, sizeof(struct game_server));
        LOCK = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(LOCK, NULL);
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

    char raw_buffer[2000];
    char_buff *input_buffer = cb_create(2000);
    char_buff *output_buffer = cb_create(2000);

    int read_size;
    char playerChar = player + '0';
    char opponentChar = ((player + 1) % 2) + '0';

    int fd = SERVER->player_sockets[player];
    struct game * gameon = game_get_current();
    int playerTurn;


    cb_append(output_buffer, "Welcome to the battleBit server Player ");
    cb_append(output_buffer, &playerChar);
    cb_append(output_buffer, "\nbattleBit (? for help) > ");
    cb_write(fd, output_buffer);

    while ((read_size = recv(fd, raw_buffer, 2000, 0)) > 0) {
        cb_reset(output_buffer);
        cb_reset(input_buffer);
        if(gameon->status == PLAYER_0_TURN) {
            playerTurn = 0;
        } else {
            playerTurn = 1;
        }

        if (read_size > 0) {
            raw_buffer[read_size] = '\0'; //null terminate read
            // append to input buffer
            cb_append(input_buffer, raw_buffer);

            char *command = cb_tokenize(input_buffer, " \r\n");

            if (command) {
                char* arg1 = cb_next_token(input_buffer);
                char* arg2 = cb_next_token(input_buffer);

                if (strcmp(command, "?") == 0) {
                    // create output
                    cb_append(output_buffer, "? - show help\n");
                    cb_append(output_buffer,"load <string> - load a ship layout file for the given player\n");
                    cb_append(output_buffer,"show - shows the board for the given player\n");
                    cb_append(output_buffer,"fire [0-7] [0-7] - fire at the given position\n");
                    cb_append(output_buffer,"say <string> - Send the string to all players as part of a chat\n");
                    cb_append(output_buffer,"exit - quit the server\n");
                    //cb_append(output_buffer, command);

                } else if (strcmp(command, "exit") == 0) {
                    cb_append(output_buffer, "Goodbye!\n");

                    close(fd);

                } else if (strcmp(command, "show") == 0) {
                    repl_print_board(gameon, player, output_buffer);
                } else if (strcmp(command, "load") == 0) {
                    game_load_board(gameon, player, arg1);

                    if (gameon->status == PLAYER_0_TURN) {
                        cb_append(output_buffer, "All Player Boards Loaded\n");
                        cb_append(output_buffer, "Player 0 Turn");

                    } else {
                        cb_append(output_buffer, "Waiting On Player ");
                        cb_append(output_buffer, &opponentChar);
                    }
                } else if (strcmp(command, "fire") == 0) {

                    if ((gameon->status == NULL)) {
                        cb_append(output_buffer, "Game Has Not Begun!");
                    } else if (player != playerTurn ) {
                        cb_append(output_buffer, "\nPlayer ");
                        cb_append(output_buffer, &opponentChar);
                        cb_append(output_buffer, " Turn - Not Your Turn Dumbass");
                    } else {
                        unsigned long long hits = gameon->players[player].hits;
                        pthread_mutex_lock(LOCK);
                        game_fire(gameon, player, atoi(arg1), atoi(arg2));
                        pthread_mutex_unlock(LOCK);

                        if (gameon->players[player].hits != hits ) {
                            cb_append(output_buffer, "Player ");
                            cb_append(output_buffer, &playerChar);
                            cb_append(output_buffer, " fires at ");
                            cb_append(output_buffer, arg1);
                            cb_append(output_buffer, " ");
                            cb_append(output_buffer, arg2);
                            cb_append(output_buffer, " - HIT");
                            cb_write(SERVER->player_sockets[0], output_buffer);
                            cb_write(SERVER->player_sockets[1], output_buffer);
                            cb_write(SERVER->server_thread, output_buffer);
                            cb_reset(output_buffer);

                        } else {
                            cb_append(output_buffer, "Player ");
                            cb_append(output_buffer, &playerChar);
                            cb_append(output_buffer, " fires at ");
                            cb_append(output_buffer, arg1);
                            cb_append(output_buffer, " ");
                            cb_append(output_buffer, arg2);
                            cb_append(output_buffer, " - MISS");
                            cb_write(SERVER->player_sockets[0], output_buffer);
                            cb_write(SERVER->player_sockets[1], output_buffer);
                            cb_write(SERVER->server_thread, output_buffer);
                            cb_reset(output_buffer);
                        }
                        if (gameon->status == PLAYER_0_WINS) {
                            cb_append(output_buffer, " - Player 0 WINS!");
                            cb_write(SERVER->player_sockets[0], output_buffer);
                            cb_write(SERVER->player_sockets[1], output_buffer);
                            cb_write(SERVER->server_thread, output_buffer);
                            cb_reset(output_buffer);
                            game_init();
                        } else if (gameon->status == PLAYER_1_WINS) {
                            cb_append(output_buffer, " - Player 1 WINS!\n");
                            cb_write(SERVER->player_sockets[0], output_buffer);
                            cb_write(SERVER->player_sockets[1], output_buffer);
                            //cb_write(SERVER->server_thread, output_buffer);
                            cb_reset(output_buffer);
                            game_init();
                        } else {
                            cb_append(output_buffer, "\nPlayer ");
                            cb_append(output_buffer, &opponentChar);
                            cb_append(output_buffer, " Turn");
                        }
                    }
                } else if (strcmp(command, "say") == 0) {
                    server_broadcast(arg1, player);
                } else if (strcmp(command, "shortcut") == 0) {
                    // update player 1 to only have a single ship in position 0, 0
                    game_get_current()->players[1].ships = 1ull;
                } else {
                    cb_append(output_buffer,"Unknown Command: ");
                    cb_append(output_buffer, command);
                    cb_append(output_buffer, "\n");

                }
                /*
                if(gameon->status ==  PLAYER_0_TURN) {
                    cb_append(output_buffer, "\nPlayer 0 Turn");
                } else {
                    cb_append(output_buffer, "\nPlayer 1 Turn");
                }
                 */
                cb_write(fd, output_buffer);
                //cb_write(SERVER->server_thread, output_buffer);
                printf("%s", output_buffer);
                cb_reset(output_buffer);

                cb_append(output_buffer, "\nbattleBut (? for help) > ");
                cb_write(fd, output_buffer);
                printf("%s", output_buffer);
                //cb_write(SERVER->server_thread, output_buffer);
            }
        }
    }

}

void server_broadcast(char_buff *msg, int player) {
    // send message to all players
    if (player == 0) {
        char_buff *output_buffer = cb_create(2000);
        cb_append(output_buffer, "\nPlayer 1 says: ");
        cb_append(output_buffer, msg);
        cb_append(output_buffer, "\nbattleBut (? for help) > ");
        //cb_write(SERVER->player_sockets[0], output_buffer);
        cb_write(SERVER->player_sockets[1], output_buffer);
        printf("Player 1 says: %s\nbattlebit (? for helo) > ",msg );
        //cb_write(SERVER->server_thread, output_buffer);
        cb_reset(output_buffer);
    } else {
        char_buff *output_buffer = cb_create(2000);
        cb_append(output_buffer, "\nPlayer 0 says: ");
        cb_append(output_buffer, msg);
        cb_append(output_buffer, "\nbattleBut (? for help) > ");
        cb_write(SERVER->player_sockets[0], output_buffer);
        //cb_write(SERVER->server_thread, output_buffer);
        printf("Player 0 says: %s\nbattlebit (? for help) > ",msg );
        //cb_write(SERVER->player_sockets[1], output_buffer);
        cb_reset(output_buffer);
    }



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

    SERVER->server_thread = server_socket_fd;
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


        pthread_create(&SERVER->player_threads[count], NULL, handle_client_connect, count);

        count +=1;
        //if (count > 1) {
        //    break;
        //}

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


}
