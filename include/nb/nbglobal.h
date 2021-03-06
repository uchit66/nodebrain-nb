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
* File:     global.h
*
* Title:    Global Variables Header
*
* Function:
*
*   This header defines global variables.
*
* See nbglobal.c for more information.
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- ---------------------------------------------------------------
* 2003-03-15 eat 0.5.2  Created to simplify code restructuring.
* 2003-10-07 eat 0.5.5  Removed definition of nbin - see nbGets().
* 2006-05-12 eat 0.6.6  Included servejail, servedir, and serveuser.
* 2008-03-08 eat 0.7.0  removed serveipaddr
* 2009-12-28 eat 0.7.7  included msgTrace
* 2010-10-14 eat 0.8.4  Included servepid.
* 2010-10-16 eat 0.8.4  Included servegroup.
* 2012-12-25 eat 0.8.13 Included nb_charset.
* 2014-01-06 eat 0.9.00 Included performance testing options
*============================================================================
*/
#ifndef _NB_GLOBAL_H_
#define _NB_GLOBAL_H_

#if defined(NB_INTERNAL)

#include <nb/nbcell.h>
#include <nb/nbterm.h>

extern char *nb_charset;      // character set/encoding
/*
*  After the code settles down, we need to clean this up as much as 
*  possible and then work the remaining stuff into an environment
*  structure.
*/

extern char nb_flag_stop;     /* set by stop command */
extern char nb_flag_input;    /* set when arguments other than options are specified */
// extern char nb_flag_alert;    /* set when alert command is process*/
extern int  nb_mode_check;    /* check mode */ 

extern int  nb_opt_audit;     /* audit log for actions */
extern int  nb_opt_bail;      /* exit(255) on error message */
extern int  nb_opt_daemon;    /* option to daemonize at end of arguments */
extern int  nb_opt_prompt;    /* option to prompt for commands at end of arguments */
extern int  nb_opt_query;     /* option to automatically query at end of arguments */
extern int  nb_opt_servant;   /* option to enter servant mode at end of arguments */

extern int  nb_opt_shim;      /* shim object methods to enable tracing */
extern int  nb_opt_user;      /* option to automatically load user profile */
extern int  nb_opt_hush;      // suppress calls to log routines
extern int  nb_opt_stats;     // display statistics
extern int  nb_opt_boolnotrel;// boolean not relational (move not out of relational expressions)
extern int  nb_opt_test;      // display pretend time to simplify output diff

extern int sourceTrace;       /* debugging trace flag for source input */
extern int symbolicTrace;     /* debugging trace flag for symbolic sub */
extern int queryTrace;        /* debugging trace flag for query mode */
extern int msgTrace;          // debugging trace flag for message API
//extern int tlsTrace; see nb-tls.h  // debugging trace flag for TLS routines
// extern int echo;           /* display resolved statements before executing */

//extern char serveipaddr[16];  /* ip address */

// extern int  agent;         /* running as daemon */
extern int  service;          /* running as windows service */
extern char *bufin;           /* statement buffer */

extern FILE *lfile;           /* log file */
extern char lname[100];       /* log file name */

extern char myusername[64];   /* user name */
extern char mypath[256];      /* path name */
extern char *myname;          /* program name */
extern char *mycommand;       /* command */
extern char nb_hostname[128]; // hostname

//extern FILE *ofile;           /* output file - stdout */
extern FILE *jfile;           /* journal file */
extern char jname[100];       /* journal file name */
extern char servejail[256];     // chroot jail directory
extern char servedir[256];      // chdir working directory
extern char servepid[256];      // pid file
extern char serveuser[32];      // su user
extern char servegroup[32];     // sg group

//extern struct HASH   *localH;   /* local brain term hash */
//extern NB_Term       *rootGloss; // root glossary "_" 
extern NB_Term       *symGloss; /* symbolic variable "$" glossary */
extern NB_Term *nb_TypeGloss;    /* type glossary */

extern struct ACTION *actList;
extern struct ACTION *ashList;

#if defined(WIN32)
extern int  nb_Service;
extern int  nb_ServiceStopped;
extern char serviceName[];
extern char windowsPath[];
//extern LARGE_INTEGER alarmTime; /* timer intervals - SetWaitableTimer */
#endif

#endif // NB_INTERNAL

#if defined(WIN32)
__declspec(dllexport)
#endif
extern int nbIsInteractive(nbCELL context);

#endif
