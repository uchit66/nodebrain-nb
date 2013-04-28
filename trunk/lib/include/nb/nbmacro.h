/*
* Copyright (C) 1998-2013 The Boeing Company
*                         Ed Trettevik <eat@nodebrain.org>
*
* NodeBrain is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*=============================================================================
* Program:  NodeBrain
*
* File:     nbmacro.h 
* 
* Title:    Macro Object Header
*
* Function:
*
*   This header defines routines that manage nodebrain string macro objects.
*
* See nbmacro.c for more information.
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2003-11-17 Ed Trettevik (original prototype)
* 2010-02-28 eat 0.7.9  Cleaned up -Wall warning messages (gcc 4.5.0)
*=============================================================================
*/
#ifndef _NB_MACRO_H_
#define _NB_MACRO_H_

struct NB_MACRO{
  struct NB_OBJECT  object;      /* object header */
  struct NB_TERM   *context;     /* local context */
  struct NB_LINK   *parms;       /* parameter list - positional terms */
  struct NB_LINK   *defaults;    /* default parameter assertion */
  struct STRING    *string;      /* model string to resolve */
  };

typedef struct NB_MACRO NB_Macro;

extern struct TYPE *nb_MacroType;

void nbMacroInit(NB_Stem *stem);
NB_Macro *nbMacroParse(NB_Cell *context,char **source);
NB_Link *nbMacroParseTermList(NB_Cell *context,char **source);
char *nbMacroSub(NB_Cell *context,char **cursorP);
NB_String *nbMacroString(NB_Cell *context,char **source);
int openCreate(char *filename, int flags, mode_t mode);
int openRead(char *filename, int flags);

#endif
