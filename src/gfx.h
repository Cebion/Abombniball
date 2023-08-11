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


#ifndef GFX_H
#define GFX_H
#include "SDL.h"
#include "SDL_image.h"

#define FONT_WIDTH 30
#define FONT_HEIGHT 35
#define FONT_GLYPHS 37

typedef struct Rect{
  SDL_Rect rect;
  struct Rect *next;
} Rect;

typedef struct Sprite{
  int x, y;
  int width, height;
  int num_frames;
  int current_frame;
  int priority;
  int hidden;
  SDL_Surface **frames;
  SDL_Surface *backing_store;
  SDL_Rect restore_rect;
  
  struct Sprite *next;
} Sprite;

typedef struct String{
  struct Gfx *gfx;
  int length;
  char *string;
  int need_redraw;

  int x,y;
  int width, height;
  SDL_Surface *image;
  SDL_Surface *backing_store;
  SDL_Rect restore_rect;
  struct String *next;
} String;

typedef struct Gfx{
  SDL_Surface *screen;
  float frame_delta;
  float time;
  int frame_count;
  Uint32 last_time;
  
  Sprite *sprites;
  String *strings;
  
  SDL_Surface *glyphs[FONT_GLYPHS];
  
  Rect *_dirty_rects;
  Rect *_clean_rects;

  void *_rect_mem;
} Gfx;

Gfx *gfxInit(int width, int height, int num_rects);
void gfxDirtyRectangle(Gfx *gfx, SDL_Rect *rect);
void gfxUpdate(Gfx *gfx);
void gfxQuit(Gfx *gfx);
void gfxAddSprite(Gfx *gfx, Sprite *sprite);
void gfxRemoveSprite(Gfx *gfx, Sprite *sprite);
SDL_Surface *gfxLoadImage(char *filename);

Sprite *spriteNew(Gfx *gfx, int width, int height, int num_frames);
void spriteDelete(Sprite *sprite);

String *stringNew(Gfx *gfx, int length);
void stringDelete(String *str);
void stringSet(String *str, char *string);
void stringMove(String *str, int x, int y);

#endif
