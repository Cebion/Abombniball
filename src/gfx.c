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


#include "gfx.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "main.h"

Gfx *gfxInit(int width, int height, int num_rects){
  Gfx *temp_gfx;
  SDL_Surface *font_image;
  SDL_PixelFormat *format;
  SDL_Rect rect;
  
  int i;

  temp_gfx = (Gfx *) malloc( sizeof(Gfx) );
  
  temp_gfx->screen = SDL_SetVideoMode( width, height, 0,
                                        SDL_SWSURFACE/*|SDL_FULLSCREEN*/ );

  if( !temp_gfx->screen ){
    free( temp_gfx );
    errorSet(("GFX"), SDL_GetError());
    return NULL;
  }

  temp_gfx->frame_delta = 0.0001;
  temp_gfx->frame_count = 0;
  temp_gfx->last_time = SDL_GetTicks();
  temp_gfx->time = 0;
  temp_gfx->sprites = NULL;
  temp_gfx->strings = NULL;
  
  temp_gfx->_dirty_rects = NULL;
  temp_gfx->_clean_rects = (Rect *) malloc( sizeof(Rect) * num_rects );
  temp_gfx->_rect_mem = temp_gfx->_clean_rects;
  
  for( i = 0; i < num_rects; i++ )
    temp_gfx->_clean_rects[i].next = &temp_gfx->_clean_rects[i+1];

  temp_gfx->_clean_rects[num_rects-1].next = NULL;
  
  format = temp_gfx->screen->format;
  font_image = gfxLoadImage( mainCreateFilename( GFX, "font.png" ) );
  for( i = 0; i < FONT_GLYPHS; i++ ){
    temp_gfx->glyphs[i] = SDL_CreateRGBSurface( SDL_SWSURFACE,
                                                FONT_WIDTH,
                                                FONT_HEIGHT,
                                                format->BitsPerPixel,
                                                format->Rmask,
                                                format->Gmask,
                                                format->Bmask,
                                                format->Amask );
    rect.x = FONT_WIDTH * i;
    rect.y = 0;
    rect.w = FONT_WIDTH;
    rect.h = FONT_HEIGHT;
    SDL_BlitSurface( font_image, &rect, temp_gfx->glyphs[i], NULL );
    SDL_SetColorKey( temp_gfx->glyphs[i], SDL_SRCCOLORKEY,
                      SDL_MapRGB( format, 0, 255, 255 ) );
  }
  SDL_FreeSurface( font_image );
  
                                                
  return temp_gfx;
}

void gfxQuit(Gfx *gfx){
  int i;
  
  if( !gfx ){
    errorSet("GFX", "Invalide Gfx context");
    return;
  }
  
  free( gfx->_rect_mem );
  
  for( i = 0; i < FONT_GLYPHS; i++ )
    SDL_FreeSurface( gfx->glyphs[i] );
  
  free( gfx );
}

void gfxDirtyRectangle(Gfx *gfx, SDL_Rect *rect){
  Rect *temp_rect;

  if( !gfx ){
    errorSet("GFX", "Invalid Gfx context");
    return;
  }

  if( !rect ){
    errorSet("GFX", "Invalid SDL_Rect");
    return;
  }

  if( !(temp_rect = gfx->_clean_rects) ){
    errorSet("GFX", "No clean rectangles available");
    return;
  }

  gfx->_clean_rects = temp_rect->next;
  memcpy( &temp_rect->rect, rect, sizeof( SDL_Rect ) );
  temp_rect->next = gfx->_dirty_rects;
  gfx->_dirty_rects = temp_rect;
}

void gfxUpdate(Gfx *gfx){
  Rect *temp_rect;
  Sprite *sprite;
  String *string;
  SDL_Rect rect;
  
  if( !gfx ){
    errorSet("GFX", "Invalid Gfx context");
    return;
  }
  
  string = gfx->strings;
  while( string ){
    if( string->need_redraw ){
      SDL_BlitSurface( string->backing_store, NULL,
                        gfx->screen, &string->restore_rect );
      string->restore_rect.x = string->x;
      string->restore_rect.y = string->y;
      string->restore_rect.w = string->width;
      string->restore_rect.h = string->height;
      SDL_BlitSurface( gfx->screen, &string->restore_rect,
                        string->backing_store, NULL );
    }
    string = string->next;
  }

  /* Save background */
  sprite = gfx->sprites;
  while( sprite ){
    if( !sprite->hidden ){
      rect.x = sprite->x;
      rect.y = sprite->y;
      rect.w = sprite->width;
      rect.h = sprite->height;
      SDL_BlitSurface( gfx->screen, &rect, sprite->backing_store, NULL );
    }
    sprite = sprite->next;
  }

  /* Draw sprites */
  sprite = gfx->sprites;
  while( sprite ){
    if( !sprite->hidden ){
      rect.x = sprite->x;
      rect.y = sprite->y;
      SDL_BlitSurface( sprite->frames[sprite->current_frame],
                        NULL,
                        gfx->screen,
                        &rect );
      gfxDirtyRectangle( gfx, &rect );
    }

    sprite = sprite->next;
  }
  
  string = gfx->strings;
  while( string ){
    if( string->need_redraw ){
      SDL_BlitSurface( string->image, NULL,
                        gfx->screen, &string->restore_rect );
      string->need_redraw = 0;
      gfxDirtyRectangle( gfx, &string->restore_rect );
    }
    string = string->next;
  }

  while( gfx->_dirty_rects ){
    temp_rect = gfx->_dirty_rects;
    SDL_UpdateRects( gfx->screen, 1, &temp_rect->rect );
    gfx->_dirty_rects = temp_rect->next;
    temp_rect->next = gfx->_clean_rects;
    gfx->_clean_rects = temp_rect;
  }
  
  sprite = gfx->sprites;

  while( sprite ){
    if( !sprite->hidden ){
      rect.x = sprite->x;
      rect.y = sprite->y;
      SDL_BlitSurface( sprite->backing_store, NULL, gfx->screen, &rect);
      gfxDirtyRectangle( gfx, &rect );
    }
    sprite = sprite->next;
  }

  
  gfx->frame_count++;
  if( !( gfx->frame_count%10 ) ){
    Uint32 current_time=SDL_GetTicks();
    gfx->frame_delta = ( current_time - gfx->last_time ) / 10000.0;
    gfx->last_time = current_time;
  }
  gfx->time += gfx->frame_delta;
}


SDL_Surface *gfxLoadImage(char *filename){
  SDL_Surface *temp1, *temp2;
  char error[256];

  
  temp1 = IMG_Load( filename );
  strcpy( error, "Could not load image " );
  strcat( error, filename );
  if( !temp1 ){
    errorSet( "GFX", error );
    return NULL;
  }

  temp2 = SDL_DisplayFormat( temp1 );
  SDL_FreeSurface( temp1 );

  return temp2;
}

void gfxAddSprite(Gfx *gfx, Sprite *sprite){
  Sprite *prev, *next, *current;

  if( !gfx->sprites ){
    gfx->sprites = sprite;
    sprite->next = NULL;
    return;
  }

  if( gfx->sprites->priority >= sprite->priority ){
    next = gfx->sprites;
    gfx->sprites = sprite;
    sprite->next = next;
    return;
  }

  prev = gfx->sprites;
  current = gfx->sprites->next;

  while( current ){
    if( current->priority >= sprite->priority ){
      sprite->next = current;
      prev->next = sprite;
      return;
    }
    prev = current;
    current = current->next;
  }

  prev->next = sprite;
  sprite->next = NULL;
}

void gfxRemoveSprite(Gfx *gfx, Sprite *sprite){
  Sprite *prev, *current;

  if( gfx->sprites == sprite ){
    gfx->sprites = gfx->sprites->next;
    sprite->next = NULL;
    return;
  }

  prev = gfx->sprites;
  current = prev->next;

  while( current ){
    if( current == sprite ){
      prev->next = current->next;
      current->next = NULL;
      return;
    }
    prev = current;
    current = current->next;
  }
}

Sprite *spriteNew(Gfx *gfx, int width, int height, int num_frames){
  Sprite *sprite;
  SDL_PixelFormat *fmt;
  
  sprite = (Sprite *) malloc( sizeof( Sprite ) );

  sprite->x = 0;
  sprite->y = 0;
  sprite->width = width;
  sprite->height = height;
  sprite->num_frames = num_frames;
  sprite->current_frame = 0;
  sprite->priority = 0;
  sprite->hidden = 0;
  
  fmt = gfx->screen->format;
  
  sprite->backing_store = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                               width,
                                               height,
                                               fmt->BitsPerPixel,
                                               fmt->Rmask,
                                               fmt->Gmask,
                                               fmt->Bmask,
                                               fmt->Amask);

  sprite->frames = ( SDL_Surface** ) malloc( sizeof( SDL_Surface* ) *
                                              num_frames );

  memset( sprite->frames, 0, sizeof( SDL_Surface* ) * num_frames );

  return sprite;
}

void spriteDelete(Sprite *sprite){

  SDL_FreeSurface( sprite->backing_store );
  free( sprite->frames );
  free( sprite );

}

String *stringNew(Gfx *gfx, int length){
  String *str;
  
  str = ( String* ) malloc( sizeof( String ) );

  str->gfx = gfx;
  str->length = length;
  str->string = ( char* ) malloc( length + 1 );
  str->string[0] = '\0';
  str->x = 0;
  str->y = 0;
  str->width = FONT_WIDTH * length;
  str->height = FONT_HEIGHT;
  str->need_redraw = 0;
  str->restore_rect.x = - str->width;
  str->restore_rect.y = 0;
  str->restore_rect.w = 0;
  str->restore_rect.h = 0;
  str->next = NULL;
  str->backing_store = SDL_CreateRGBSurface( SDL_SWSURFACE,
                                             FONT_WIDTH * length,
                                             FONT_HEIGHT,
                                             gfx->screen->format->BitsPerPixel,
                                             gfx->screen->format->Rmask,
                                             gfx->screen->format->Gmask,
                                             gfx->screen->format->Bmask,
                                             gfx->screen->format->Amask );
  str->image = SDL_CreateRGBSurface( SDL_SWSURFACE,
                                     FONT_WIDTH * length,
                                     FONT_HEIGHT,
                                     gfx->screen->format->BitsPerPixel,
                                     gfx->screen->format->Rmask,
                                     gfx->screen->format->Gmask,
                                     gfx->screen->format->Bmask,
                                     gfx->screen->format->Amask );
  SDL_SetColorKey( str->image, SDL_SRCCOLORKEY,
                    SDL_MapRGB( str->image->format, 0, 255, 255 ) );
  SDL_FillRect( str->image, NULL,
                SDL_MapRGB( str->image->format, 0, 255, 255 ) );
  
  str->next = gfx->strings;
  gfx->strings = str;
  
  return str;
}

void stringDelete(String *str){
  String *prev, *current;

  SDL_BlitSurface( str->backing_store, NULL,
                   str->gfx->screen, &str->restore_rect );
  gfxDirtyRectangle( str->gfx, &str->restore_rect );

  if( str->gfx->strings == str )
    str->gfx->strings = str->next;
  else{
    prev = str->gfx->strings;
    current = str->gfx->strings->next;
    while( current ){
      if( current == str ){
        prev->next = current->next;
        current = NULL;
      }
      else{
        prev = current;
        current = current->next;
      }
    }
  }
      
  SDL_FreeSurface( str->image );
  SDL_FreeSurface( str->backing_store );
  
  free( str->string );

  free( str );
}

void stringSet(String *str, char *string){
  char *ptr;
  int i;
  SDL_Rect rect; 
  ptr = str->string;
  
  if( strlen( string ) < str->length )
    strcpy( str->string, string );
  else{
    strncpy( str->string, string, str->length );
    str->string[str->length] = '\0';
  }

  SDL_FillRect( str->image, NULL,
                SDL_MapRGB( str->image->format, 0, 255, 255 ) );
  i = 0;
  while( *ptr != '\0' ){
    rect.x = i * FONT_WIDTH;
    rect.y = 0;
    rect.w = FONT_WIDTH;
    rect.h = FONT_HEIGHT;
    if( isalpha( *ptr ) ){
      SDL_BlitSurface( str->gfx->glyphs[toupper( *ptr ) - 'A'], NULL,
                        str->image, &rect );
    }
    else if( isdigit( *ptr ) || *ptr == ':' ){
      SDL_BlitSurface( str->gfx->glyphs[toupper( *ptr ) - '0' + 26], NULL,
                        str->image, &rect );
    }
    ptr++;
    i++;
  }
  str->need_redraw = 1;
}

void stringMove(String *str, int x, int y){
  str->x = x;
  str->y = y;
  str->need_redraw = 1;
}


