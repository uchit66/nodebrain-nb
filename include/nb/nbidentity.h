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
* File:     nbidentity.h 
* 
* Title:    Identity Object Header
*
* Function:
*
*   This header defines routines for managing identity objects.
*
* See nbidentity.c for more information.
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2003-03-15 eat 0.5.1  Created to conform to new makefile
* 2010-02-28 eat 0.7.9  Cleaned up -Wall warning messages. (gcc 4.5.0)
*=============================================================================
*/
#ifndef _NB_IDENTITY_H_
#define _NB_IDENTITY_H_

#if defined(NB_INTERNAL)

#include <nb/nbstem.h>

typedef struct IDENTITY {
  struct NB_OBJECT object;   /* object header */
  struct STRING *name;       /* identity name */
  //vli1024 modulus;           /* encryption modulus */
  //vli1024 exponent;          /* encryption exponent */
  //vli1024 private;           /* private decryption exponent */
  unsigned long user;        /* user number */
  unsigned char authority;   /* basic authority */
  } *nbIDENTITY;

extern NB_Term       *identityC;      /* public identity context */
//extern struct HASH   *identityH;      /* identity hash */
extern struct TYPE   *identityType;   /* identity object type */

extern struct IDENTITY *defaultIdentity;   /* default identity */

//#if defined(WIN32)
//#if defined(LIBNB_EXPORTS)
//__declspec(dllexport)
//#else
//__declspec(dllimport)
//#endif
//#endif

extern struct IDENTITY *clientIdentity;    /* client identity */

#define AUTH_CONNECT 1    /* connection permission */
#define AUTH_ASSERT  2    /* assert,alert,cache permission */
#define AUTH_DEFINE  4    /* define,undefine permission */
#define AUTH_DECLARE 32   /* declare permission (owner) */
#define AUTH_SYSTEM  64   /* shell command permission */
#define AUTH_CONTROL 128  /* grant,set,stop (owner) */

#define AUTH_GUEST   1    /* guest can connect and use unprotected commands */
#define AUTH_PEER    7    /* connect, assert, define */
#define AUTH_USER    127  /* peer plus declare and system permissions */

#define AUTH_OWNER   255  /* unlimited permissions */

// 2012-10-17 eat - Setting a maximum length on identity names until the Peer
//                  module is converted to OpenSSL.  The native authentication
//                  code in Peer can not handle the larger maximum length of a
//                  term. 
#define NB_IDENTITY_MAXLEN 48  // maximum length of identity

/* functions */
void initIdentity(NB_Stem *stem);
struct IDENTITY *nbIdentityNew(char *name,unsigned char authority);
struct IDENTITY *getIdentity(char *identity);

//typedef struct IDENTITY *nbIDENTITY;
// temporary export until 0.7.0
//#if defined(WIN32)
//#if defined(_USRDLL)
//__declspec(dllimport)
//#else
//__declspec(dllexport)
//#endif
//#endif
//extern nbIDENTITY clientIdentity;    /* client identity */

#else  // !NB_INTERNAL

//*****************************
// External API

typedef void *nbIDENTITY;

#endif // !NB_INTERNAL 

#define NB_AUTH_CONNECT 1    /* connection permission */
#define NB_AUTH_ASSERT  2    /* assert,alert,cache permission */
#define NB_AUTH_DEFINE  4    /* define,undefine permission */
#define NB_AUTH_DECLARE 32   /* declare permission (owner) */
#define NB_AUTH_SYSTEM  64   /* shell command permission */
#define NB_AUTH_CONTROL 128  /* grant,set,stop (owner) */

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbIDENTITY nbIdentityGet(nbCELL context,char *ident);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern char *nbIdentityGetName(nbCELL context,nbIDENTITY identity);

/*
*  The client identity is a global variable, logically a register in a
*  virtual machine.  The nbIdentityGetClient and nbIdentitySetClient
*  functions are used to switch identities.
*/
#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbIDENTITY nbIdentityGetActive(nbCELL context);

#if defined(WIN32)
_declspec (dllexport)
#endif
extern nbIDENTITY nbIdentitySetActive(nbCELL context,nbIDENTITY identity);
#endif
