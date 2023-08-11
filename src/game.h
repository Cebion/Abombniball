/*
 * Abombniball
 * Copyright (C) 2001 Martin Donlon, akawaka@csn.ul.ie
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */


#ifndef GAME_H
#define GAME_H
#include "SDL.h"
#include "SDL_image.h"

#define BALL_DEAD       0x0000
#define BALL_FALLING    0x0100
#define BALL_EXPLODING  0x0200
#define BALL_IDLE       0x0300
#define BALL_MOVING     0x0400
#define BALL_JUMPING    0x1000

#define BALL_LEFT       0x0001
#define BALL_UP         0x0002
#define BALL_RIGHT      0x0003
#define BALL_DOWN       0x0004

#define BALL_STATEMASK  0x0f00
#define BALL_JUMPMASK   0xf000
#define BALL_DIRMASK    0x00ff

enum { GAME_INGAME,
       GAME_LEVELDONE,
       GAME_LEVELSUMMARY,
       GAME_LEVELBOMBS,
       GAME_NEWLEVEL,
       GAME_REINIT,
       GAME_LEVELTITLE,
       GAME_FELL,
       GAME_EXPLODE,
       GAME_DEATHMESSAGE,
       GAME_OVER,
       GAME_OVERMESSAGE
} GameStates;

      
typedef struct Tile{
  int visible;
  int solid;
  int direction;
  int jump;
  SDL_Surface *image;
  char name[32];
} Tile;

typedef struct GameGrid{
  int x, y;
  int width, height;
  int tiled_width, tiled_height;
  int tile_width, tile_height;
  int num_tiles;
  int jumps_allowed;
  Tile *tilemap;
  char name[32];
  char tune[256];
} GameGrid;

typedef struct Ball{
  struct Sprite *sprite;
  int state;
  int lives;
  int jumps;
  int score;
  int pixel_count;
  float next_update;
} Ball;

typedef struct Bomb{
  float next_tick;
  struct Sprite *sprite;
  struct Bomb *next;
} Bomb;

typedef struct Game{
  int level;
  int state;
 
  int num_levels;
  char **level_names;

  Ball ball;
  Bomb *bombs;
  Tile *tile_types;
  struct String *score_tag;
  struct String *score_text;
  struct String *hiscore_tag;
  struct String *hiscore_text;
  struct String *lives_text;
  struct String *jumps_text;
  SDL_Surface *bomb_images[10];
  GameGrid *grid;
  struct Gfx *gfx;
} Game;

struct Input;

Game *gameNew(struct Gfx *gfx);
void gameDelete(Game *game);
void gameSetGrid(Game *game, GameGrid *grid, int bomb_coords[]);
void gameReset(Game *game);
void gameRedrawGrid(Game *game);
int gameLoop(Game *game);
void gameUpdateBall(Game *game, struct Input *input);
void gameUpdateBombs(Game *game);
void gameGotBomb(Game *game);

GameGrid *gridNew(int tiled_width, int tiled_height,
                  int tile_width, int tile_height);
void gridDelete(GameGrid *grid);
Tile *gridGetTile(GameGrid *grid, int x, int y);

void gridFill(GameGrid *grid, Uint8 *tile_data, Tile *tile_types);
void gameHitTile(Game *game, int x, int y);

int gameLoadLevel(Game *game, char *levelname);

void strip_crud(char *ptr);
void strip_crud_space(char *str);
void get_coord(char *ptr, int *x, int *y);
#endif
