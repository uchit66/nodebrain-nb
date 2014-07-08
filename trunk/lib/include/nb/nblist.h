/*
* Copyright (C) 2014 Ed Trettevik <eat@nodebrain.org>
*
* NodeBrain is free software; you can modify and/or redistribute it under the
* terms of either the MIT License (Expat) or the following NodeBrain License.
*
* Permission to use and redistribute with or without fee, in source and binary
* forms, with or without modification, is granted free of charge to any person
* obtaining a copy of this software, provided that the above copyright notice,
* this permission notice, and the following disclaimer are retained with source
* code and reproduced in documentation included with binary distributions. 
*
* Unless required by applicable law or agreed to in writing, this software is
* distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.
*
*==============================================================================
* Program:  NodeBrain
*
* File:     nblist.h 
* 
* Title:    Object List Management Routines (prototype)
*
* Function:
*
*   This header defines routines that manage NodeBrain object lists.
*
* See nblist.c for more information.
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2002-08-25 Ed Trettevik (original prototype version introduced in 0.4.0)
*             1) Some of this code was split out from nbcell.h
* 2002-08-28 eat - version 0.4.0 A7
*             1) Created LIST object and replace old LIST structure with
*                MEMBER structure.
* 2005-04-08 eat 0.6.2  API function definitions moved to nbapi.h
* 2010-02-28 eat 0.7.9  Cleaned up -Wall warning messages. (gcc 4.5.0)
*=============================================================================
*/
#ifndef _NB_LIST_H_
#define _NB_LIST_H_

#include <nb/nbcell.h>

#if defined(NB_INTERNAL)

#include <nb/nbstem.h>
struct NB_LIST{           /* object list cell */
  struct NB_CELL  cell;
  struct NB_LINK  *link;  /* points to link for first member */
  };

typedef struct NB_LIST NB_List;
typedef void **NB_Pointer;

extern NB_Link *nb_LinkFree;
extern struct HASH *listH;
extern struct TYPE *nb_ListType;

/* List member methods */
void listInsert(NB_Link **memberP,void *object);
void listRemove(NB_Link **memberP,void *object);
struct NB_LINK *newMember(struct NB_TERM *context,char **cursor);
void *dropMember(NB_Link *member);

/* List methods */
int listInsertUnique(NB_Link **memberP,void *object);

NB_List *parseList(struct NB_TERM *context,char **cursor);
NB_List *useList(NB_Link *link);
NB_List *nbListCompute(NB_List *cellList);

void solveList(NB_List *list);
struct NB_OBJECT *evalList(NB_List *list);
void enableList(NB_List *list);
void printList(NB_List *list);
void nbListShowAll(void);
void printMembers(NB_Link *member);
void listInit(NB_Stem *stem,size_t n); // 2012-12-31 eat - n from int to size_t

void nbListFree(struct NB_LINK *member);

#endif // !NB_INERNAL

//**********************************************
// External API

typedef void*  nbSET;         // Position pointer within a set of cells (e.g. lists) 

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbCELL nbListCreate(nbCELL context,char **cursor);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern void *nbListOpen(nbCELL context,nbCELL cell);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern void *nbListInsertCell(nbCELL context,nbSET *setP,nbCELL cell);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbCELL nbListRemoveCell(nbCELL context,nbSET *setP);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbCELL nbListGetCell(nbCELL context,nbSET *setP);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbCELL nbListGetCellValue(nbCELL context,nbSET *setP);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern void nbListEmpty(nbCELL context,nbSET *set);

#endif
