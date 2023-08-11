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


#include "game.h"
#include "gfx.h"
#include "input.h"
#include "main.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <SDL_mixer.h>

extern char *strdup(char *);

static char *_bomb_images[] = { "bomb-1.png",
                                "bomb-2.png",
                                "bomb-3.png",
                                "bomb-4.png",
                                "bomb-5.png",
                                "bomb-6.png",
                                "bomb-7.png",
                                "bomb-8.png",
                                "bomb-9.png",
                                "bomb.png"
};

static char *_ball_images[] = { "ball-1.png",
                                "ball-2.png",
                                "ball-3.png",
                                "ball-4.png",
                                "ball-5.png",
                                "ball-6.png",
                                "ball-7.png",
                                "ball-8.png",
};

static char _ball_frames[] = { 7, 6, 5, 4, 3, 3,
                              2, 2, 1, 1, 0, 0,
                              0, 0, 1, 1, 2, 2,
                              3, 3, 4, 5, 6, 7
};

#define NUM_TILES 14
static Tile _tile_types[NUM_TILES]={{0, 0, 0, 0, NULL, "blank" },
                                    {1, 1, 0, 0, NULL, "red" },
                                    {1, 1, 0, 0, NULL, "green" },
                                    {1, 1, 0, 0, NULL, "blue" },
                                    {1, -1, 0, 0, NULL, "solid" },
                                    {1, -1, BALL_UP, 0, NULL, "pushup"},
                                    {1, -1, BALL_DOWN, 0, NULL, "pushdown"},
                                    {1, -1, BALL_LEFT, 0, NULL, "pushleft"},
                                    {1, -1, BALL_RIGHT, 0, NULL, "pushright"},
                                    {1, -1, BALL_UP, 1, NULL, "jumpup"},
                                    {1, -1, BALL_DOWN, 1, NULL, "jumpdown"},
                                    {1, -1, BALL_LEFT, 1, NULL, "jumpleft"},
                                    {1, -1, BALL_RIGHT, 1, NULL, "jumpright"},
                                    {1, 2, 0, 0, NULL, "2left"}
};

#define BALL_UPDATE 0.016

Game *gameNew(struct Gfx *gfx){
  Game *game;
  int i;
  Tile *tile;
  char filename[40];
  FILE *fp;
  char line[256];
  
  game = (Game *) malloc( sizeof( Game ) );

  game->grid = NULL;
  game->bombs = NULL;
  game->gfx = gfx;
  
  game->level = 0;
  game->state = GAME_REINIT;
  
  for( i = 0; i < 10 ; i++ ){
    game->bomb_images[i] = gfxLoadImage(
                            mainCreateFilename( GFX, _bomb_images[i] ) );
    if( !game->bomb_images[i] ) return NULL;
    SDL_SetColorKey( game->bomb_images[i],
                      SDL_SRCCOLORKEY,
                      SDL_MapRGB( game->bomb_images[i]->format, 0, 255, 0 ) );
  }
  game->ball.sprite = spriteNew( gfx, 24, 24, 8 );
  game->ball.next_update = 0;
  
  game->ball.lives = 3;
  game->ball.score = 0;
  
  for( i =0; i < 8; i++ ){
    game->ball.sprite->frames[i] = gfxLoadImage(
                                    mainCreateFilename( GFX, _ball_images[i] ));
    if( !game->ball.sprite->frames[i] ) return NULL;
    SDL_SetColorKey( game->ball.sprite->frames[i], SDL_SRCCOLORKEY,
            SDL_MapRGB( game->ball.sprite->frames[i]->format, 0, 255, 255 ) );
  }
  
  gfxAddSprite( gfx, game->ball.sprite );
  
  game->tile_types = ( Tile* ) malloc( sizeof( Tile ) * NUM_TILES );
  memcpy( game->tile_types, _tile_types, sizeof( Tile ) * NUM_TILES );
  for( i = 0; i < NUM_TILES; i++ ){
    tile = &game->tile_types[i];
    if( tile->visible ){
      strcpy( filename, tile->name );
      strcat( filename, ".png" );
      tile->image = gfxLoadImage( mainCreateFilename( GFX, filename ) );
      if( !tile->image ) return NULL;
    }
  } 

  fp = fopen( mainCreateFilename( LEVELS, "level_list" ), "r" );
  
  if( !fp ){
    errorSet( "GAME", "Could not read level list" );
    return NULL;
  }
  fgets( line, 255, fp );
  strip_crud( line );
  game->num_levels = atoi( line );

  game->level_names = ( char** ) malloc( sizeof( char* ) * game->num_levels );
  for( i = 0; i < game->num_levels; i++ ){
    fgets( line, 255, fp );
    strip_crud( line );
    game->level_names[i] = strdup( line );
  }
 
  fclose( fp );
  
  game->score_tag = stringNew( gfx, 5 );
  stringMove( game->score_tag, 438, 0 );
  stringSet( game->score_tag, "SCORE");
  
  game->score_text = stringNew( gfx, 5 );
  stringMove( game->score_text, 438, 35 );
  stringSet( game->score_text, "00000");

  game->hiscore_tag = stringNew( gfx, 8 );
  stringMove( game->hiscore_tag, 395, 150 );
  stringSet( game->hiscore_tag, "HI SCORE");
  
  game->hiscore_text = stringNew( gfx, 5 );
  stringMove( game->hiscore_text, 438, 185 );
  stringSet( game->hiscore_text, "00000");
   
  game->lives_text = stringNew( gfx, 8 );
  stringMove( game->lives_text, 395, 300 );
  
  game->jumps_text = stringNew( gfx, 8 );
  stringMove( game->jumps_text, 395, 415 );
  
  return game;
}

void gameDelete(Game *game){
  Bomb *bomb;
  int i;
  
  if( game->grid ) gridDelete( game->grid );
  
  while( game->bombs ){
    bomb = game->bombs;
    gfxRemoveSprite( game->gfx, bomb->sprite );
    spriteDelete( bomb->sprite );
    game->bombs = bomb->next;
    free( bomb );
  }
  
  gfxRemoveSprite( game->gfx, game->ball.sprite );
  for( i = 0; i < 8; i++ )
    SDL_FreeSurface( game->ball.sprite->frames[i] );
  spriteDelete( game->ball.sprite );

  for( i = 0; i < NUM_TILES; i++ )
    SDL_FreeSurface( game->tile_types[i].image );
  free( game->tile_types );

  for( i = 0; i < 10 ; i++ )
    SDL_FreeSurface( game->bomb_images[i] );
  
  for( i = 0; i < game->num_levels; i++ )
    free( game->level_names[i] );
  free( game->level_names );
  
  stringDelete( game->score_tag );
  stringDelete( game->score_text );
  stringDelete( game->hiscore_tag );
  stringDelete( game->hiscore_text );
  stringDelete( game->lives_text );
  stringDelete( game->jumps_text );
  
  free( game );

}

void gameSetGrid(Game *game, GameGrid *grid, int *bomb_coords){
  Bomb *bomb, *prev;
  game->grid = grid;
  
  prev = NULL;
  
  while( *bomb_coords != -1 ){
    bomb = ( Bomb* ) malloc( sizeof( Bomb ) );
    bomb->sprite = spriteNew( game->gfx, game->bomb_images[0]->w,
                                         game->bomb_images[0]->h,
                                         10 );
    memcpy(bomb->sprite->frames, game->bomb_images, sizeof(SDL_Surface*) * 10);
    bomb->sprite->x = grid->x + ( grid->tile_width * bomb_coords[0] );
    bomb->sprite->y = grid->y + ( grid->tile_height * bomb_coords[1] );
    bomb->sprite->current_frame = 9;
    bomb->next_tick = 0;
    bomb_coords += 2;
    
    /* gfxAddSprite( game->gfx, bomb->sprite ); */
    
    bomb->next = NULL;
    if( !prev )
      game->bombs = bomb;
    else
      prev->next = bomb;

    prev = bomb;
  }
}

void gameReset(Game *game){
  char text[12];
  SDL_Rect rect = { 0, 0, 640, 480 };
  SDL_FillRect( game->gfx->screen, NULL, 0 );
  gfxDirtyRectangle( game->gfx, &rect );
  game->ball.jumps = game->grid->jumps_allowed;
  sprintf( text, "JUMPS: %d", game->ball.jumps );
  stringSet( game->jumps_text, text );
  sprintf( text, "LIVES: %d", game->ball.lives );
  stringSet( game->lives_text, text );
  game->ball.state = BALL_IDLE;
  gameRedrawGrid(game);
  game->gfx->time = 0;
  game->ball.next_update = 0;

  game->score_tag->need_redraw = 1;
  game->score_text->need_redraw = 1;
  game->hiscore_tag->need_redraw = 1;
  game->hiscore_text->need_redraw = 1;
  game->lives_text->need_redraw = 1;
  game->jumps_text->need_redraw = 1;
  
}

void gameRedrawGrid(Game *game){
  int x, y, i;
  SDL_Rect rect;
  Tile *tile;
  
  i=0;
  for( y = 0; y < game->grid->tiled_height; y++ ){
    for( x = 0; x < game->grid->tiled_width; x++ ){
      
      tile = &game->grid->tilemap[i];
      i++;
      rect.x = game->grid->x + game->grid->tile_width * x;
      rect.y = game->grid->y + game->grid->tile_width * y;
      rect.w = game->grid->tile_width;
      rect.h = game->grid->tile_height;
      
      if( tile->image && tile->visible )
        SDL_BlitSurface( tile->image, NULL, game->gfx->screen, &rect );
    }
  }

  rect.x = 0;
  rect.y = 0;
  rect.w = game->gfx->screen->w;
  rect.h = game->gfx->screen->h;
  
  gfxDirtyRectangle( game->gfx, &rect);
}

GameGrid *gridNew(int tiled_width, int tiled_height,
                  int tile_width, int tile_height){
  GameGrid *grid;
  
  grid = (GameGrid *) malloc( sizeof( GameGrid ) );

  grid->tilemap = (Tile *) malloc( sizeof( Tile ) *
                                    tiled_width * tiled_height );

  memset( grid->tilemap, 0, sizeof( Tile ) * tiled_width * tiled_height );
  
  grid->x = grid->y = 0;
  grid->width = tiled_width * tile_width;
  grid->height = tiled_height * tile_height;

  grid->tile_width = tile_width;
  grid->tile_height = tile_height;
  grid->tiled_width = tiled_width;
  grid->tiled_height = tiled_height;
  grid->num_tiles = tiled_width * tiled_height;
  return grid;
}

void gridDelete(GameGrid *grid){
  free( grid->tilemap );
  free( grid );
}

Tile *gridGetTile(GameGrid *grid, int x, int y){
  return &grid->tilemap[ x + ( y * grid->tiled_width ) ];
}

void gridFill(GameGrid *grid, Uint8 *tile_data, Tile *tile_types){
  int i;

  for( i = 0; i < grid->num_tiles; i++ )
    memcpy( &grid->tilemap[i], &tile_types[ tile_data[i] ], sizeof( Tile ) );
}

int gameLoop(Game *game){
  Input input;
  char score[7];
  float next_time;
  String *text;
  Mix_Music *tune = 0;
  Bomb *current_bomb = NULL;

  input.quit = 0;
  input.fire = 0;
  input.direction = 0;
  
  while( !input.quit ){
    switch( game->state ){
      case GAME_INGAME:
        inputHandle( &input );
        gameUpdateBall( game, &input );
        if( game->bombs )
          gameUpdateBombs( game );
        else
          game->state = GAME_LEVELDONE;
        break;

      case GAME_FELL:
        next_time = game->gfx->time + 1.0;
        game->state = GAME_DEATHMESSAGE;
        text = stringNew( game->gfx, 12 );
        stringMove( text, 20, 200 );
        stringSet( text, "Careful Now" );
        break;

      case GAME_EXPLODE:
        next_time = game->gfx->time + 1.0;
        game->state = GAME_DEATHMESSAGE;
        text = stringNew( game->gfx, 12 );
        stringMove( text, 0, 200 );
        stringSet( text, "Buuuuurrrnnn" );
        break;
        
      case GAME_DEATHMESSAGE:
        if( game->gfx->time > next_time ){
          game->ball.lives--;
          if( game->ball.lives == -1 )
            game->state = GAME_OVER;
          else
            game->state = GAME_REINIT;
          stringDelete( text );
        }
        else
          text->need_redraw = 1;
        break;
        
      case GAME_OVER:
        next_time = game->gfx->time + 3.0;
        game->state = GAME_OVERMESSAGE;
        text = stringNew( game->gfx, 10 );
        stringMove( text, 50, 200 );
        stringSet( text, "Game Over" );
        if( tune ){
          Mix_HaltMusic();
          Mix_FreeMusic( tune );
        }
        tune = NULL;
        break;
      
      case GAME_OVERMESSAGE:
        if( game->gfx->time > next_time ){
          stringDelete( text );
          input.quit = 1;
        }
        else
          text->need_redraw = 1;
        break;

      case GAME_LEVELDONE:
        next_time = game->gfx->time + 2.0;
        game->state = GAME_LEVELSUMMARY;
        text = stringNew( game->gfx, 10 );
        stringMove( text, 50, 200 );
        stringSet( text, "Well Done" );
        break;

      case GAME_LEVELSUMMARY:
        if( game->gfx->time > next_time ){
          game->state = GAME_NEWLEVEL;
          stringDelete( text );
        }
        else
          text->need_redraw = 1;
        break;
      
      case GAME_NEWLEVEL:
        game->ball.score += 500 + ( 100 * game->ball.jumps );
        sprintf( score, "%05d", game->ball.score );
        stringSet( game->score_text, score );
        game->level++;
        if( tune ){
          Mix_HaltMusic();
          Mix_FreeMusic( tune );
        }
        tune = NULL;
        if( game->level == game->num_levels ){
          game->state = GAME_OVER;
          break;
        }

      case GAME_REINIT:
        if( gameLoadLevel( game, game->level_names[game->level] ) )
          return -1;
        gameReset( game );
        if( !tune ){
          tune = Mix_LoadMUS( mainCreateFilename( AUDIO, game->grid->tune ) );
          if( !tune )
            errorSet("AUDIO", "Could not load music");
          else
            Mix_PlayMusic( tune, -1 );
        }
        next_time = game->gfx->time + 1.0;
        game->state = GAME_LEVELBOMBS;
        current_bomb = game->bombs;
        text = NULL;
        break;

      case GAME_LEVELBOMBS:
        if( game->gfx->time > next_time ){
          if( current_bomb ){
            gfxAddSprite( game->gfx, current_bomb->sprite );
            next_time += 1.0;
            current_bomb = current_bomb->next;
          }
          else{
            next_time = game->gfx->time + 1.5;
            game->state = GAME_LEVELTITLE;
            text = stringNew( game->gfx, strlen( game->grid->name ) );
            stringMove( text, 200 - ( text->width / 2.0 ), 200 );
            stringSet( text, game->grid->name );
          }
        }
        else if( text )
          text->need_redraw = 1;
        break;
            
      case GAME_LEVELTITLE:
        if( game->gfx->time > next_time ){
          game->state = GAME_INGAME;
          game->gfx->time = 0.0;
          stringDelete( text );
        }
        else
          text->need_redraw = 1;
        break;

      default:
        break;
    }
        
    gfxUpdate( game->gfx );
  }
  
  if( tune ){
    Mix_HaltMusic();
    Mix_FreeMusic( tune );
  }
  tune = NULL;
  return 0;
}

void gameUpdateBall(Game *game, struct Input *input){
  Ball *ball = &game->ball;
  int grid_x, grid_y; 
  int step_size;
  Tile *tile;
  char jumps[12];

  grid_x = ( ball->sprite->x - game->grid->x ) / game->grid->tile_width;
  grid_y = ( ball->sprite->y - game->grid->y ) / game->grid->tile_height;
  
  if( ( ball->state & BALL_STATEMASK ) == BALL_IDLE ){
    ball->sprite->current_frame = _ball_frames[0];
    
    tile = gridGetTile( game->grid, grid_x, grid_y );
    if( !tile->solid ){
      ball->state = BALL_FALLING;
    }
    else{
      if( tile->direction ){
        ball->state = BALL_MOVING;
        ball->pixel_count = 24;
        ball->state |= tile->direction;
        if( tile->jump )
          ball->state |= BALL_JUMPING;
      }
      else if( input->direction ){
        ball->state = BALL_MOVING;
        ball->pixel_count = 24;
        switch( input->direction ){
          case INPUT_LEFT:
            ball->state |= BALL_LEFT;
            break;
          case INPUT_UP:
            ball->state |= BALL_UP;
            break;
          case INPUT_RIGHT:
            ball->state |= BALL_RIGHT;
            break;
          case INPUT_DOWN:
            ball->state |= BALL_DOWN;
            break;
          default:
            printf("ARGH!\n");
            break;
        }
        if( ball->jumps > 0 && input->fire ){
          ball->state |= BALL_JUMPING;
          ball->jumps--;
          sprintf( jumps, "JUMPS: %d", ball->jumps );
          stringSet( game->jumps_text, jumps );
        }
        gameHitTile( game, grid_x, grid_y );
      }
    }
  }
    
  while( ball->next_update < game->gfx->time ){
  
    switch( ball->state & BALL_STATEMASK ){
      case BALL_DEAD:
        input->quit = 1;
        break;

      case BALL_EXPLODING:
        game->state = GAME_EXPLODE;
        break;
        
      case BALL_FALLING:
        game->state = GAME_FELL;
        break;

      case BALL_IDLE:
        break;
  
      case BALL_MOVING:
        if( ball->state & BALL_JUMPMASK )
          step_size = 2;
        else
          step_size = 1;
          
        switch( ball->state & BALL_DIRMASK ){
          case BALL_LEFT:
            ball->sprite->x -= step_size;
            ball->pixel_count--;
            break;
  
          case BALL_UP:
            ball->sprite->y -= step_size;
            ball->pixel_count--;
            break;
  
          case BALL_RIGHT:
            ball->sprite->x += step_size;
            ball->pixel_count--;
            break;
  
          case BALL_DOWN:
            ball->sprite->y += step_size;
            ball->pixel_count--;
            break;
  
          default:
            break;
        }
  
        ball->sprite->current_frame = _ball_frames[ 23 - ball->pixel_count ];
      
        if( !ball->pixel_count ){
          ball->state &= ~BALL_STATEMASK;
          ball->state |= BALL_IDLE;
        }
        break;
      
      default:
        break;
    }
    ball->next_update += BALL_UPDATE;
  }
}

void gameUpdateBombs(Game *game){

  if( ( game->ball.state & BALL_STATEMASK ) == BALL_IDLE ){
    Bomb *bomb;
    Ball *ball = &game->ball;
    
    bomb = game->bombs;
    while( bomb ){
      if( bomb->sprite->x == ball->sprite->x &&
          bomb->sprite->y == ball->sprite->y ){
        if( bomb == game->bombs ){
          gameGotBomb( game );
          bomb = game->bombs;
        }
        else{
          ball->state = BALL_EXPLODING;
          bomb = bomb->next;
        }
      }
      else
        bomb = bomb->next;
    }
  }
  if( game->bombs && game->bombs->next_tick < game->gfx->time ){
    if( game->bombs->sprite->current_frame == 0 ){
      if( ( game->ball.state & BALL_STATEMASK ) != BALL_DEAD )
        game->ball.state = BALL_EXPLODING;
      return;
    }
    game->bombs->sprite->current_frame--;
    game->bombs->next_tick += 2.0;
  }
}

void gameGotBomb(Game *game){
  Bomb *bomb = game->bombs;
  char score[6];
  game->bombs = bomb->next;
  gfxRemoveSprite( game->gfx, bomb->sprite );
  spriteDelete( bomb->sprite );
  free( bomb );
  if( game->bombs ){
    game->bombs->next_tick = game->gfx->time;
  }
  game->ball.score += 50;
  sprintf( score, "%05d", game->ball.score );
  stringSet( game->score_text, score );
}

  
  
void gameHitTile(Game *game, int x, int y){
  Tile *tile;
  char score[6];
  SDL_Rect rect;

  tile = &game->grid->tilemap[ ( y * game->grid->tiled_width ) + x ];
  tile->solid--;

  if( !tile->solid ){
    game->ball.score += 10;
    sprintf( score, "%05d", game->ball.score );
    stringSet( game->score_text, score );
    tile->visible = 0;
    rect.x = ( x * game->grid->tile_width ) + game->grid->x;
    rect.y = ( y * game->grid->tile_height ) + game->grid->y;
    rect.w = game->grid->tile_width;
    rect.h = game->grid->tile_height;
    SDL_FillRect( game->gfx->screen, &rect, 0 );
    gfxDirtyRectangle( game->gfx, &rect );
  }
}

int gameLoadLevel(Game *game, char *levelname){
  Bomb *bomb;
  GameGrid *grid;
  FILE *fp;
  int reading_grid=0;
  char name[17];
  char line[256];
  char tune[256];
  Tile *tile_types[256];
  char *tile_name, tile_char;
  int jumps = 3;
  int i;
  int tiles_read = 0;
  char read_char;
  
  int bomb_coords[256];
  int bomb_count=0;
  int start_x, start_y;
  int grid_width, grid_height;
  
  tune[0] = '\0';
  
  memset( tile_types, 0, sizeof( Tile* ) * 256 );
  if( game->grid ) gridDelete( game->grid );
  while( game->bombs ){
    bomb = game->bombs;
    gfxRemoveSprite( game->gfx, bomb->sprite );
    spriteDelete( bomb->sprite );
    game->bombs = bomb->next;
    free( bomb );
  }
  
  fp = fopen( mainCreateFilename( LEVELS, levelname ), "r" );
  if( !fp ){
    errorSet( "GAME", "Could not load level" );
    return -1;
  }
  while( !reading_grid && fgets( line, 255, fp ) ){
    switch( line[0] ){
      case '\"':
        strip_crud_space( &line[2] );
        strncpy( name, &line[2], 16 );
        name[16] = '\0';
        break;

      case '!':
        tile_char = line[2];
        tile_name = &line[4];
        strip_crud( tile_name ); 
        for( i = 0; i < NUM_TILES; i++ ){
          if( !strcmp( game->tile_types[i].name, tile_name ) )
            tile_types[ (int) tile_char ] = &game->tile_types[i];
        }
        break;

      case 'd':
        get_coord( &line[2], &grid_width, &grid_height );
        break;
       
      case 's':
        get_coord( &line[2], &start_x, &start_y );
        break;

      case 'b':
        get_coord( &line[2], &bomb_coords[bomb_count],
                             &bomb_coords[bomb_count+1] );
        bomb_coords[bomb_count+2] = -1;
        bomb_count += 2;
        break;
      case 'j':
        strip_crud( &line[2] );
        jumps = atoi( &line[2] );
        break;
      case 'm':
        strip_crud( &line[2] );
        strcpy( tune, &line[2] );
        break;
      case '$':
        reading_grid = 1;
        break;

      default:
        break;
    }
  }

  grid = gridNew( grid_width, grid_height, 24, 24 );
  
  while( tiles_read < grid->num_tiles && ( read_char = getc( fp ) ) != EOF ){
    if( tile_types[(int) read_char] ){
      memcpy( &grid->tilemap[tiles_read],
              tile_types[(int) read_char],
              sizeof( Tile ) );
      tiles_read++;
    }
  }
  grid->jumps_allowed = jumps;
  strcpy( grid->name, name );
  strcpy( grid->tune, tune );
  gameSetGrid( game, grid, bomb_coords );
  game->ball.sprite->x = grid->x + start_x * 24;
  game->ball.sprite->y = grid->y + start_y * 24;
  
  fclose( fp );
  return 0;
}
    

void strip_crud(char *str){
  char *ptr=str;
  while( isgraph(*ptr) )
    ptr++;
  *ptr = '\0';
}

void strip_crud_space(char *str){
  char *ptr=str;
  while( isgraph(*ptr) || *ptr == ' ' )
    ptr++;
  *ptr = '\0';
}

void get_coord(char *str, int *x, int *y){
  char *ptr=str;

  while( *ptr != ' ' )
    ptr++;
  *ptr = '\0';

  *x = atoi( str );
  
  ptr++;
  strip_crud( ptr );
  *y = atoi( ptr );
}
  
