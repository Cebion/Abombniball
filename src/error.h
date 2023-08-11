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


#ifndef ERROR_H
#define ERROR_H

extern char _internal_error[];

#define DEBUG

#ifdef DEBUG
#define errorSet(sys,msg) \
do{\
  sprintf(_internal_error, "%s::%s  (%s:%d)", sys, msg, __FILE__, __LINE__);\
  fprintf(stderr, "%s\n", _internal_error);\
}while(0)
#else
#define errorSet(sys,msg) \
  sprintf(_internal_error, "%s::%s  (%s:%d)", sys, msg, __FILE__, __LINE__);
#endif

#define errorGet() _internal_error

#endif
