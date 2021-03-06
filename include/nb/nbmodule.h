/*
* Copyright (C) 2014 Ed Trettevik <eat@nodebrain.org>
*
* NodeBrain is free software; you can modify and/or redistribute it under the
* terms of either the MIT License (Expat) or the following NodeBrain License.
*
* Permission to use and redistribute with or without fee, in source and binary
* forms, with or without modification, is granted free of charge to any person
* obtaining a copy of this software and included documentation, provided that
* the above copyright notice, this permission notice, and the following
* disclaimer are retained with source files and reproduced in documention
* included with source and binary distributions. 
*
* Unless required by applicable law or agreed to in writing, this software is
* distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.
*
*=============================================================================
* Program:  NodeBrain
*
* File:     nbmodule.h
*
* Title:    Module Header
*
* Function:
*
*   This header defines routines that manage module objects.
*
* See nbmodule.c for more information.
*============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- ----------------------------------------------------------------
* 2003-04-22 eat 0.5.4  Introduced prototype
* 2005-04-22 eat 0.6.2  Included suffix parameter to nbModuleSymbol()
* 2005-04-24 eat 0.6.2  Included path option and NB_MODULE_PATH environment variable
* 2005-05-14 eat 0.6.3  module handle renamed address and added new handle
* 2010-02-28 eat 0.7.9  Cleaned up -Wall warning messages (gcc 4.5.0)
* 2013-01-11 eat 0.8.13 Checker updates
*============================================================================
*/
#ifndef _NB_MODULE_H_
#define _NB_MODULE_H_

#if defined(NB_INTERNAL)

#include <nb/nbstem.h>
#include <nb/nbnode.h>

struct NB_MODULE{            /* module object */
  struct NB_OBJECT object;   /* object header */
  struct STRING   *path;     /* module path - overrides native pathing */
  struct STRING   *name;     /* file name - may include a path - will after loaded if path specified */
  struct NB_LIST  *args;     /* arguments for module initialization */
  struct STRING   *text;     /* text for module initialization */
  void            *handle;   /* handle provided by module initialization method */
  void            *address;  /* Dynamic load module address */
  };

extern struct TYPE *moduleType;
extern struct NB_TERM *moduleC;

void nbModuleInit(NB_Stem *stem);
void nbModuleBind(nbCELL context,char *name,char *msg,size_t msglen);
NB_Term *nbModuleDeclare(NB_Term *context,char *name,char *cursor);
void *nbModuleSymbol(NB_Term *context,char *ident,char *suffix,void **moduleHandleP,char *msg,size_t msglen);

#if defined(WIN32)
HINSTANCE nbModuleLoad(char *name,int export,char *msg,size_t msglen);
#else
void *nbModuleLoad(char *name,int export,char *msg,size_t msglen);
#endif
void nbModuleShowInstalled(nbCELL context);

NB_Facet *nbSkillGetFacet(NB_Skill *skill,const char *ident);

#endif // NB_INTERNAL

//**********************************
// External API

#if defined(WIN32)
_declspec (dllexport)
#endif
extern int nbSkillDeclare(nbCELL context,void *(*bindFunction)(),void *moduleHandle,char *moduleName,char *skillName,nbCELL arglist,char *text);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern int nbSkillSetMethod(nbCELL context,nbCELL skill,int methodId,void *method);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbCELL nbSkillFacet(nbCELL context,nbCELL skillHandle,const char *ident);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern int nbSkillMethod(nbCELL context,nbCELL facetHandle,int methodId,void *method);
#endif
