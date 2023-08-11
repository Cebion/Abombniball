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


#include <stdio.h>
#include <stdlib.h>
#include "abombniball.h"
#include "gfx.h"
#include "game.h"
#include "input.h"
#include "error.h"
#include "main.h"
#include <string.h>

enum{ INIT = 1,
      IDLE,
      GAME,
      REDRAW,
      QUIT,
      MENU_MAIN
} Actions;

#define MAX_MENU 10

typedef struct MenuItem{
  char name[32];
  int action;
} MenuItem;

static MenuItem main_menu[] = { { "Start", GAME },
                                { "Quit", QUIT },
                                { "", -1 }
};


static MenuItem *current_menu = main_menu;
static String *menu_text[MAX_MENU] = { NULL, NULL, NULL, NULL, NULL,
                                       NULL, NULL, NULL, NULL, NULL
};
static int menu_length = 0;
static int pointer_pos = 0;

int mainTitle(Gfx *gfx);
void mainSetMenu(Gfx *gfx, MenuItem *items);

char *abombniball_dir;
char _temp_filename[256];

int main(int argc, char *argv[]){
  Gfx *gfx;
  
  abombniball_dir = getenv( "ABOMBNIBALL_DIR" );

  if( !abombniball_dir ){
    errorSet( "MAIN", "ABOMBNIBALL_DIR is not set, using \"./\"" );
    abombniball_dir = "./";
  }
  
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)){
    errorSet( "MAIN", SDL_GetError() );
    exit( -1 );
  }
  
  gfx = gfxInit( 640, 480, 256 );
  if( !gfx ){
    errorSet( "MAIN", "Could not create graphcis context" );
    exit( -1 );
  }
  
  if( Mix_OpenAudio( 22050, AUDIO_S16, 2, 4096 ) ){
    errorSet( "AUDIO", SDL_GetError() );
    exit( -1 );
  }
  
  if( !mainTitle( gfx ) ){
    errorSet( "MAIN", "Undefined error" );
  }
  
  gfxQuit( gfx );
  
  Mix_HaltMusic();
  Mix_CloseAudio();
  SDL_Quit();

  exit( 0 ); 
}

int mainTitle(Gfx *gfx){
  Game *game;
  Mix_Music *title_tune = NULL;
  SDL_Surface *title;
  SDL_Rect full_rect = { 0, 0, 640, 480 };
  Sprite *pointer;
  int state, quit = 0;
  Input input = { 0, 0, 0 };
  
  state = INIT;

  while( !quit ){
    switch( state ){
      case INIT:
        title = gfxLoadImage( mainCreateFilename( GFX, "title.png" ) );
        if( !title ) return 0;
        pointer = spriteNew( gfx, 24, 24, 1 );
        pointer->frames[0] = gfxLoadImage(
                                mainCreateFilename( GFX, "ball-1.png" ) );
        if( !pointer->frames[0] ) return 0;
        SDL_SetColorKey( pointer->frames[0],
                         SDL_SRCCOLORKEY,
                         SDL_MapRGB( pointer->frames[0]->format, 0, 255, 255));
        
        gfxAddSprite( gfx, pointer );
      case REDRAW:
        SDL_BlitSurface( title, NULL, gfx->screen, NULL );
        gfxDirtyRectangle( gfx, &full_rect );
        state = IDLE;
        mainSetMenu( gfx, main_menu );
        pointer_pos = 0;
        pointer->hidden = 0;
        if( title_tune )
          Mix_FreeMusic( title_tune );
        title_tune = Mix_LoadMUS( mainCreateFilename( AUDIO, "title.xm" ) );
        if( !title_tune ){
          errorSet( "AUDIO", "Could not load title.xm" );
          return 0;
        }
        Mix_PlayMusic(title_tune, -1);
        break;

      case GAME:
        mainSetMenu( gfx, NULL );
        Mix_FadeOutMusic( 1000 );
        while( Mix_FadingMusic() == MIX_FADING_OUT )
          SDL_Delay( 100 );
        Mix_FreeMusic( title_tune );
        title_tune = NULL;
        pointer->hidden = 1;
        game = gameNew( gfx );
        if( !game ){
          errorSet( "MAIN", "Could not create game" );
          return 0;
        }
        if( gameLoop( game ) ){
          errorSet( "MAIN", "Error during game" );
          return 0;
        }
        gameDelete( game );
        state = REDRAW;
        break;

      case IDLE:
        input.fire = 0;
        input.direction = 0;
        inputHandle( &input );
        if( input.direction == INPUT_UP && pointer_pos > 0 )
          pointer_pos--;
        if( input.direction == INPUT_DOWN && pointer_pos < menu_length - 1 )
          pointer_pos++;
        
        if( input.fire ){
          state = current_menu[pointer_pos].action;
          break;
        }
        
        pointer->x = 220;
        pointer->y = 205 + ( pointer_pos * 40 );
        gfxUpdate( gfx );
        break;

      case QUIT:
        quit = 1;
        SDL_FreeSurface( title );
        gfxRemoveSprite( gfx, pointer );
        SDL_FreeSurface( pointer->frames[0] );
        spriteDelete( pointer );
        Mix_FadeOutMusic( 1000 );
        while( Mix_FadingMusic() == MIX_FADING_OUT )
          SDL_Delay( 100 );
        Mix_HaltMusic();
        Mix_FreeMusic( title_tune );
        break;

      default:
        break;
    }
  }
  return 1;
}

void mainSetMenu(Gfx *gfx, MenuItem *items){
  int i;
  MenuItem *item;
  
  for( i = 0; i < MAX_MENU; i++ ){
    if( menu_text[i] ){
      stringDelete( menu_text[i] );
      menu_text[i] = NULL;
    }
  }
  
  current_menu = items;
  item = items;
  menu_length = 0;
  if( items == NULL ) return;

  i = 0;
  
  while( item->action != -1 ){
    menu_text[i] = stringNew( gfx, strlen( item->name ) );
    stringSet( menu_text[i], item->name );
    stringMove( menu_text[i], 250, 200 + ( i * 40 ) );
    item++;
    i++;
  }
  menu_length = i;
}

char *mainCreateFilename( int section, char *filename ){
  strcpy( _temp_filename, abombniball_dir );

  switch( section ){
    case GFX:
      strcat( _temp_filename, "/gfx/" );
      break;

    case LEVELS:
      strcat( _temp_filename, "/levels/" );
      break;

    case AUDIO:
      strcat( _temp_filename, "/audio/" );
      break;

    default:
      strcat( _temp_filename, "/" );
      break;
  }

  strcat( _temp_filename, filename );

  return _temp_filename;
}

    
  
