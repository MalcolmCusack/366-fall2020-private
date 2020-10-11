//
// Created by carson on 5/20/20.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "game.h"

// STEP 10 - Synchronization: the GAME structure will be accessed by both players interacting
// asynchronously with the server.  Therefore the data must be protected to avoid race conditions.
// Add the appropriate synchronization needed to ensure a clean battle.

static game * GAME = NULL;

void game_init() {
    if (GAME) {
        free(GAME);
    }
    GAME = malloc(sizeof(game));
    GAME->status = CREATED;
    game_init_player_info(&GAME->players[0]);
    game_init_player_info(&GAME->players[1]);
}

void game_init_player_info(player_info *player_info) {
    player_info->ships = 0;
    player_info->hits = 0;
    player_info->shots = 0;
}

int game_fire(game *game, int player, int x, int y) {
    // Step 5 - This is the crux of the game.  You are going to take a shot from the given player and
    // update all the bit values that store our game state.
    //
    //  - You will need up update the players 'shots' value
    //  - you You will need to see if the shot hits a ship in the opponents ships value.  If so, record a hit in the
    //    current players hits field
    //  - If the shot was a hit, you need to flip the ships value to 0 at that position for the opponents ships field
    //
    //  If the opponents ships value is 0, they have no remaining ships, and you should set the game state to
    //  PLAYER_1_WINS or PLAYER_2_WINS depending on who won.
}

unsigned long long int xy_to_bitval(int x, int y) {
    // Step 1 - implement this function.  We are taking an x, y position
    // and using bitwise operators, converting that to an unsigned long long
    // with a 1 in the position corresponding to that x, y
    //
    // x:0, y:0 == 0b1 (the one is in the first position)
    // x:1, y: 0 == 0b10 (the one is in the second position)
    // ....
    // x:0, y: 1 == 0b100000000 (the one is in the eighth position)
    //
    // you will need to use bitwise operators and some math to produce the right
    // value.

    if ((x < 8) && (x > -1) && (y < 8) && (y > -1)) {
        return (1ull << ((x) + (8 * y)));
    } else {
        return 0;
    }

}

struct game * game_get_current() {
    return GAME;
}

int game_load_board(struct game *game, int player, char * spec) {
    // Step 2 - implement this function.  Here you are taking a C
    // string that represents a layout of ships, then testing
    // to see if it is a valid layout (no off-the-board positions
    // and no overlapping ships)
    //
    // if it is valid, you should write the corresponding unsigned
    // long long value into the Game->players[player].ships data
    // slot and return 1
    //
    // if it is invalid, you should return -1

    int x, y, length, count = 0;

    char *shiptypes;

    if (spec == NULL) {
        return -1;
    }

    if (strlen(spec) == 15) {



        for (int i=0; i < strlen(spec); i++) {
            // grabs what type of ship

            if (i % 3 == 0) {

                for (int j=0; j < strlen(spec); j++) {

                    if (tolower(spec[i]) == tolower(spec[j])) {
                        ++count;
                    }
                    if (count > 1) {
                        return -1;

                    }
                }
                count = 0;
                //carrier
                if (spec[i] == 'C') {
                    // horizontal
                    length = 5;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if(length + x > 7 && y < 8) {
                        return -1;
                    }
                    add_ship_horizontal(&game->players[player], x, y, length);
                    //printf("%llu", xy_to_bitval(x, y));

                } else if (spec[i] == 'c') {
                    // vertical
                    length = 5;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';

                    if (length + y > 7 && x < 8) {
                        return -1;
                    }
                    add_ship_vertical(&game->players[player], x, y, length);
                    // printf("%c", spec[i]);
                    //battleship
                } else if (spec[i] == 'B') {
                    //horizontal
                    length = 4;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if(length + x > 7 && y < 8) {
                        return -1;
                    }
                    add_ship_horizontal(&game->players[player], x, y, length);

                } else if (spec[i] == 'b') {
                    // vertical
                    length = 4;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if (length + y > 7 && x < 8) {
                        return -1;
                    }
                    add_ship_vertical(&game->players[player], x, y, length);

                    //destroyer
                } else if (spec[i] == 'D') {
                    //horizontal
                    length = 3;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if(length + x > 7 && y < 8) {
                        return -1;
                    }
                    if (add_ship_horizontal(&game->players[player], x, y, length)) {
                        return -1;
                    }

                } else if (spec[i] == 'd') {
                    // vertical
                    length = 3;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if (length + y > 7 && x < 8) {
                        return -1;
                    }
                    if (add_ship_vertical(&game->players[player], x, y, length) == -1) {
                        return -1;
                    }

                    //Sub
                } else if (spec[i] == 'S') {
                    //horizontal
                    length = 3;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if(length + x > 7 && y < 8) {
                        return -1;
                    }
                    add_ship_horizontal(&game->players[player], x, y, length);

                } else if (spec[i] == 's') {
                    // vertical
                    length = 3;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if (length + y > 7 && x < 8) {
                        return -1;
                    }
                    add_ship_vertical(&game->players[player], x, y, length);

                    //patrol
                } else if (spec[i] == 'P') {
                    //horizontal
                    length = 2;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if(length + x > 7 && y < 8) {
                        return -1;
                    }
                    add_ship_horizontal(&game->players[player], x, y, length);

                } else if (spec[i] == 'p') {
                    // vertical
                    length = 2;
                    x = (int) spec[(i + 1)] - '0';
                    y = (int) spec[(i + 2)] - '0';
                    if (length + y > 7 && x < 8) {
                        return -1;
                    }
                    add_ship_vertical(&game->players[player], x, y, length);


                } else {
                    return -1;
                }
            }
        }

    } else {
        printf("%s", spec);
        return -1;
    }
    return 1;
}

int add_ship_horizontal(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively

    //player->ships
    int mask = xy_to_bitval(x, y);
    int temp = length;
    if (mask & player->ships) {
        return -1;
    } else if(length + x > 7 && y < 8) {
        return -1;
    } else {
        while(temp>0){
            player->ships = (mask | player->ships);
            //(player->ships << mask);
            x=x+1;
            temp = temp-1;
            add_ship_horizontal(player, x, y, temp);

        }
        return 1;
    }
}

int add_ship_vertical(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively

    int mask = xy_to_bitval(x, y);
    int temp = length;
    if (mask & player->ships) {
        return -1;
    } else if (length + y > 7 && x < 8) {
        return -1;
    } else {
        while(temp>0){
            player->ships = (mask | player->ships);
            y= y + 1;
            temp = temp - 1;
            add_ship_vertical(player, x, y, temp);
        }
        return 1;
    }
}
