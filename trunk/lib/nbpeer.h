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
* File:     nbpeer.h
*
* Title:    Peer API Header
*
* Function:
*
*   This header defines routines that implement the NodeBrain Peer API. 
*
* See nbpeer.c for more information.
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2010/01/07 eat 0.7.7  (original prototype) 
*=============================================================================
*/
#ifndef _NBPEER_H_
#define _NBPEER_H_       /* never again */

#define NB_PEER_BUFLEN 64*1024  // buffer length

typedef struct NB_PEER{
  int           flags;   // see NB_PEER_FLAG_* 
//  int           sd;      // socket we listen on - may preceed tls for now
//  nbTLSX        *tlsx;
  nbTLS         *tls;
  unsigned char *wbuf;
  unsigned char *wloc;
  unsigned char *rbuf;
  unsigned char *rloc;
  void          *handle;
  int  (*producer)(nbCELL context,struct NB_PEER *peer,void *handle);
  int  (*consumer)(nbCELL context,struct NB_PEER *peer,void *handle,void *data,int len);
  void (*shutdown)(nbCELL context,struct NB_PEER *peer,void *handle,int code);
  } nbPeer;

#define NB_PEER_FLAG_WRITE_WAIT  1
#define NB_PEER_FLAG_READ_WAIT   2
#define NB_PEER_FLAG_WRITE_ERROR 4

// API

extern nbPeer *nbPeerConstruct(nbCELL context,char *uriName,char *uri,nbCELL tlsContext,void *handle,
  int (*producer)(nbCELL context,nbPeer *peer,void *handle),
  int (*consumer)(nbCELL context,nbPeer *peer,void *handle,void *data,int len),
  void (*shutdown)(nbCELL context,nbPeer *peer,void *handle,int code));

extern void nbPeerModify(nbCELL context,nbPeer *peer,void *handle,
  int (*producer)(nbCELL context,nbPeer *peer,void *handle),
  int (*consumer)(nbCELL context,nbPeer *peer,void *handle,void *data,int len),
  void (*shutdown)(nbCELL context,nbPeer *peer,void *handle,int code));

extern int nbPeerListen(nbCELL context,nbPeer *peer); 

extern int nbPeerConnect(nbCELL context,nbPeer *peer,void *handle,
  int (*producer)(nbCELL context,nbPeer *peer,void *handle),
  int (*consumer)(nbCELL context,nbPeer *peer,void *handle,void *data,int len),
  void (*shutdown)(nbCELL context,nbPeer *peer,void *handle,int code));

extern int nbPeerSend(nbCELL context,nbPeer *peer,void *data,int len);

extern int nbPeerShutdown(nbCELL context,nbPeer *peer,int code);

extern int nbPeerDestroy(nbCELL context,nbPeer *peer);

#endif
