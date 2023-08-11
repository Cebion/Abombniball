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


#include "input.h"

void inputHandle(Input *input){
  SDL_Event event;

  while( SDL_PollEvent(&event) ){
    switch( event.type ){
      case SDL_KEYDOWN:
        switch( event.key.keysym.sym ){
          case SDLK_q:
          case SDLK_h:
          case SDLK_LEFT:
            input->direction = INPUT_LEFT;          
            break;
          case SDLK_e:
          case SDLK_k:
          case SDLK_UP:
            input->direction = INPUT_UP;          
            break;
          case SDLK_w:
          case SDLK_l:
          case SDLK_RIGHT:
            input->direction = INPUT_RIGHT;          
            break;
          case SDLK_j:
          case SDLK_r:
          case SDLK_DOWN:
            input->direction = INPUT_DOWN;          
            break;
          case SDLK_LCTRL:
          case SDLK_RCTRL:
          case SDLK_RETURN:
          case SDLK_SPACE:
          case SDLK_t:
            input->fire = 1;          
            break;
          case SDLK_ESCAPE:
            input->quit = 1;
            break;
          default:
            break;
        }
        break;
      
      case SDL_KEYUP:
        switch( event.key.keysym.sym ){
          case SDLK_q:
          case SDLK_h:
          case SDLK_LEFT:
            if( input->direction == INPUT_LEFT)
              input->direction = 0;          
            break;
          case SDLK_e:
          case SDLK_k:
          case SDLK_UP:
            if( input->direction == INPUT_UP)
              input->direction = 0;          
            break;
          case SDLK_w:
          case SDLK_l:
          case SDLK_RIGHT:
            if( input->direction == INPUT_RIGHT)
              input->direction = 0;          
            break;
          case SDLK_j:
          case SDLK_r:
          case SDLK_DOWN:
            if( input->direction == INPUT_DOWN)
              input->direction = 0;          
            break;
          case SDLK_LCTRL:
          case SDLK_RCTRL:
          case SDLK_RETURN:
          case SDLK_SPACE:
          case SDLK_t:
            input->fire = 0;
            break;
          default:
            break;
        }
        break;
        
      case SDL_QUIT:
        input->quit = 1;
        break;

      default:
        break;
    }
  }
}

