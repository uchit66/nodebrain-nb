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
* File:     global.c
*
* Title:    Global Variables
*
* Function:
*
*   This file contains global variables.
*
* Note:
*
*   We plan to migrate away from the use of this file.  As an alternative,
*   an "environment" structure will be defined and passed as a parameter
*   to functions heavy into shared variables.  This will make it possible
*   for multiple environments to be active at one time.  A better 
*   alternative is to pass specific parameters between functions when
*   possible.  
*
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- ---------------------------------------------------------------
* 2003-03-15 eat 0.5.2  Created to simplify code restructuring.
* 2003-05-22 eat 0.5.3  Dropped global server_socket variable (see listeners)
* 2003-10-07 eat 0.5.5  Removed definition of nbin - see nbGets().
* 2006-05-12 eat 0.6.6  Included servejail, servedir, and serveuser.
* 2009-12-28 eat 0.7.7  Included msgTrace.
* 2010-10-14 eat 0.8.4  Included servepid.
* 2010-10-16 eat 0.8.4  Included servegroup.
* 2012-12-25 eat 0.8.13 Included nb_charset.
* 2014-01-06 eat 0.9.00 Included performance testing options.
*============================================================================
*/
#include <stdio.h> 

char *nb_charset="";     // character set/encoding

char nb_flag_stop=0;     /* set to 1 by stop command */
char nb_flag_input=0;    /* set when input argument provided */
//char nb_flag_alert=0;    /* set when alert command is processed */
int  nb_mode_check=0;    /* check mode - collecting in all odd modes */
                         /*  0 - Not in a check mode */
                         /*  1 - collecting   */
                         /*  2 - enforcing (check script *.nb#)  */
                         /*  4 - check mode   */
                         /*  8 - error        */

int  nb_opt_audit=1;     /* audit log for actions */
int  nb_opt_bail=0;      /* exit(NB_EXITCODE_BAIL) on error message */
int  nb_opt_daemon=2;    /* daemonized at end of arguments - 2=default */
int  nb_opt_prompt=0;    /* prompt for commands at end of arguments */
int  nb_opt_query=0;     /* automatically query at end of arguments */
int  nb_opt_servant=0;   /* enter servant mode at end of arguments */

int  nb_opt_shim=0;      /* shim object methods to enable tracing */
int  nb_opt_user=1;      /* automatically load user profile */
int  nb_opt_hush=0;      // suppress calls to log routines
int  nb_opt_stats=0;     // print statistics when terminating
int  nb_opt_boolnotrel=0;// boolean not relational - a<>1 ==> !(a=1), a<=1 ==> !(a>1), a>=1 ==> !(a<1)
int  nb_opt_test=0;      // test mode - display pretend time stamps to simplify output diff
int  nb_opt_safe=0;      // safe mode prevents unsaf interactions with host environment
                         // e.g. spawning child processes, source command, modules other than tree, and cache

int sourceTrace=0;       /* debugging trace flag for source input */
int symbolicTrace=0;     /* debugging trace flag for symbolic substitution */
int queryTrace=0;        /* debugging trace flag for query mode */
int msgTrace=0;          // debugging trace flag for message API */
/* int echo=1; */        /* display resolved statements before executing */


//char serveipaddr[16];    /* ip address */

int  agent=0;            /* running as daemon */
int  nb_Service=0;       /* running as windows service */
int  nb_ServiceStopped=0; // flag when stopped
//char bufin[16*1024];     /* NB_BUFSIZE statement buffer */
char *bufin=NULL;        // NB_BUFSIZE statement buffer */

FILE *lfile;             /* log file */
char lname[100];         /* log file name */

char myusername[64];     /* user name */
char mypath[256];        /* path name */
char *myname;            /* program name */
char *mycommand;         /* command */
char nb_hostname[128];   // hostname

//FILE *ofile;             /* output file - stdout */
FILE *jfile;             /* journal file */
char jname[100];         /* journal file name */

char servejail[256];     // chroot jail directory
char servedir[256];      // chdir working directory
char servepid[256];      // pid file
char serveuser[32];      // su user
char servegroup[32];     // sg group

//struct HASH    *localH;   /* local brain term hash */
//struct NB_TERM *rootGloss; // root glossary "_"
struct NB_TERM *symGloss;  /* symbolic variable "$" glossary */
struct NB_TERM *nb_TypeGloss;    /* type glossary */

#if defined(WIN32)
char windowsPath[512];
char serviceName[512];
#endif

