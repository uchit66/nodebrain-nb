/*
* Copyright (C) 1998-2010 The Boeing Company
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
* Foundation, Inc., 59 Temple Place Suite 330, Boston, MA 02111-1307, USA.
*
*=============================================================================
* Program:  NodeBrain
*
* File:     nbcall.h
*
* Title:    Call Header (real numbers)
*
* Function:
*
*   This header defines routines that manage nodebrain "call" objects.
*
* See nbcall.c for more information.
*============================================================================
* Change History:
*
*    Date     Name/Change
* ----------  ---------------------------------------------------------------
* 2002-09-07  Ed Trettevik (original prototype)
* 2010-02-28 eat 0.7.9  Cleaned up -Wall warning message. (gcc 4.5.0)
*============================================================================
*/
#ifndef _NB_CALL_H_
#define _NB_CALL_H_

#include <nb/nbstem.h>

/* 
* NOTE: 
*
*   Effectively, we are extending COND with call by adding more types
*   and associated evaluation methods.  This structure must have the
*   same basic structure as COND (cell and two pointers).
*/
struct CALL{                /* Call cell - one or two operands */
  struct NB_CELL   cell;     /* cell header */
  /*struct NB_OBJECT *object;*/  /* return object */
  void *pad;
  struct NB_LIST   *list;    /* parameter list */
  };

extern struct TYPE *callTypeMod;
void initCall(NB_Stem *stem);

#endif
