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
* File:     nb_servant.c 
*
* Title:    Servant Process Module (prototype)
*
* Function:
*
*   This module provides the "servant" skill.  It manages a child process that 
*   performs some external function and may communicate with NodeBrain by
*   exchanging messages on stdin, stdout or staderr.
*
* Synopsis:
*
*   define <term> node servant("<options>"):<command>
*
*   <options>         ::= no options supported yet
*
*   <command>         ::= ["|"] <servantCommand>
*
*   <servantCommand>  ::= See NodeBrain User's Guide  ("-" and "=" prefixes)
*                         See also nbMedullaProcessOpen() in nbmedulla.c
*
* Description
*
*   In NodeBrain terminology, a "servant" is a child process forked by NodeBrain
*   to perform an external function, which relies only on stdin, stdout, and stderr
*   for communication with NodeBrain.  This is a specific type of child process
*   supported by the Medulla API.
*
*   The servant skill module provides just one method of interfacing with
*   servants.  What you can do with a servant node is very similar to what you
*   can do with a servant command ("-" and "=").  However, servant nodes can be 
*   used multiple times and enable a servant to recieve messages on stdin in
*   addition to writing messages on stdout.
*   
*   The pipe symbol "|" before the "-" or "=" that begin a servant command indicates
*   that we want to send messages to the process on stdin using "+<context>:"
*   commands.  In this case, we create a pipe for NodeBrain to send messages to the
*   servants stdin.
*
*   If symbols "-" and "=" have the same meaning as when used as NodeBrain
*   commands.  The "-" specifies a blocking execution and "=" specifies a
*   non-blocking execution.  Actually, the syntax and symantecs of the
*   <servantCommand> are unmodified here.
*
*   However, here we have to fuss the reusability of the servant---starting
*   and restarting at the appropriate times.
*
*=============================================================================
* Enhancements:
*
*   We have a lot of things to work out here.  We'll need to add an atexit
*   function to nbMedullaProcessOpen so we find out when a process terminates.
*   Depending on the options used, we may need to restart it right away or
*   restart it the next time we have data to send to it.
*
*   We'll need to modularize spawnSystem() so we can call it when the "-"
*   option is used. For now we'll focus on the "=" option.
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2005/12/29 Ed Trettevik (original prototype version 0.6.4)
* 2007/12/26 eat 0.6.8  Inserted null closer in call to nbMedullaProcessOpen
* 2010-02-25 eat 0.7.9  Cleaned up -Wall warning messages
* 2012-10-16 eat 0.8.12 Checker updates
* 2012-10-17 eat 0.8.12 Checker updates
* 2012-12-15 eat 0.8.13 Checker updates
*=============================================================================
*/
#include "config.h"
#include <nb/nb.h>

typedef struct NB_MOD_SERVANT{
  nbCELL context;
  nbPROCESS process;
  char cmd[1024];     // command
  char log[256];      // logfile
  } nbServant;

// read a stderr line and write it to the log
static int logMsgReader(nbPROCESS process,int pid,void *session,char *msg){
  nbServant *servant=session;
  //nbLogMsg(servant->context,0,'T',"logMsgReader called");
  nbLogMsg(servant->context,0,'W',"%s",msg);
  return(0);
  }

// read a command and pass it to the interpreter
static int cmdMsgReader(nbPROCESS process,int pid,void *session,char *msg){
  nbServant *servant=session;
  //nbLogMsg(servant->context,0,'T',"cmdMsgReader called");
  nbCmd(servant->context,msg,1);
  return(0);
  }

// message writer stub - see servantCommand() method for actual writer
static int cmdMsgWriter(nbPROCESS process,int pid,void *session){
  return(0);
  }

//******************************************************
// Skill methods
//******************************************************
// Construct a servant
static void *servantConstruct(nbCELL context,void *skillHandle,nbCELL arglist,char *text){
  nbServant *servant;
  static unsigned char filecount=0;
  //int len;

  if(strlen(text)>=sizeof(servant->cmd)){
    nbLogMsg(context,0,'E',"Command text len=%d too long for buffer len=%d\n",strlen(text),sizeof(servant->cmd));
    return(NULL);
    }
  servant=nbAlloc(sizeof(nbServant));
  servant->context=context;
  servant->process=NULL;
  snprintf(servant->cmd,sizeof(servant->cmd),"%s",text);  // 2012-10-16 eat - length checked above // 2013-01-17 eat - VID 6572
  filecount++;  // modulo 256
  snprintf(servant->log,sizeof(servant->log),"servant.%8.8d.%3.3d.out",(int)time(NULL),filecount);
  nbListenerEnableOnDaemon(context);
  return(servant);
  }

static int servantAssert(nbCELL context,void *skillHandle,nbServant *servant,nbCELL arglist,nbCELL value){
  return(0);
  }

static int servantEnable(nbCELL context,void *skillHandle,nbServant *servant,nbCELL arglist,nbCELL value){
  char msg[NB_MSGSIZE];
  nbLogMsg(context,0,'I',"Enabling %s",servant->cmd);
  servant->process=nbMedullaProcessOpen(NB_CHILD_TERM|NB_CHILD_SESSION,servant->cmd,servant->log,servant,NULL,cmdMsgWriter,cmdMsgReader,logMsgReader,msg,sizeof(msg));
  if(servant->process==NULL){
    nbLogMsg(context,0,'E',"%s",msg);
    return(1);
    }
  nbLogMsg(context,0,'I',"Enabled [%d] %s",nbMedullaProcessPid(servant->process),servant->cmd);
  return(0);
  }

static int servantDisable(nbCELL context,void *skillHandle,nbServant *servant,nbCELL arglist,nbCELL value){
  return(0);
  }

static int servantCommand(nbCELL context,void *skillHandle,nbServant *servant,nbCELL arglist,char *text){
  char msgbuf[NB_BUFSIZE];
  int len;

  if(servant->process==NULL){
    nbLogMsg(context,0,'E',"Servant not enabled");
    return(1);
    }
  if(strlen(text)>=sizeof(msgbuf)){
    nbLogMsg(context,0,'E',"Command too long for buffer");
    return(1);
    }
  sprintf(msgbuf,"%s\n",text);
  // for now assume the process is running and accepts commands
  if(nbMedullaProcessStatus(servant->process)&NB_MEDULLA_PROCESS_STATUS_ENDED){
    nbLogMsg(context,0,'E',"Servant ended - restart not yet supported");
    return(1);
    }
  else if(!(nbMedullaProcessStatus(servant->process)&NB_MEDULLA_PROCESS_STATUS_STARTED)){
    nbLogMsg(context,0,'E',"Servant not started - auto start not yet supported");
    return(1);
    }
  else len=nbMedullaProcessPut(servant->process,msgbuf);
  return(0);
  }

/*
*  Show skull
*/
static int servantShow(nbCELL context,void *skillHandle,nbServant *servant,int option){
  return(0);
  }

static void *servantDestroy(nbCELL context,void *skillHandle,nbServant *servant,int option){
  nbFree(servant,sizeof(nbServant));
  return(NULL);
  }

#if defined(_WINDOWS)
_declspec (dllexport)
#endif
extern void *servantBind(nbCELL context,void *moduleHandle,nbCELL skill,nbCELL arglist,char *text){
  nbSkillSetMethod(context,skill,NB_NODE_CONSTRUCT,servantConstruct);
  nbSkillSetMethod(context,skill,NB_NODE_ASSERT,servantAssert);
  nbSkillSetMethod(context,skill,NB_NODE_ENABLE,servantEnable);
  nbSkillSetMethod(context,skill,NB_NODE_DISABLE,servantDisable);
  nbSkillSetMethod(context,skill,NB_NODE_SHOW,servantShow);
  nbSkillSetMethod(context,skill,NB_NODE_DESTROY,servantDestroy);
  nbSkillSetMethod(context,skill,NB_NODE_COMMAND,servantCommand);
  return(NULL);
  }
