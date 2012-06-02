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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*=============================================================================
* Program:  NodeBrain
*
* File:     nbsource.c
*
* Title:    NodeBrain Source File Processor
*
* Function:
*
*   This file provides routines that process a source file, implementing
*   source file directives that start with "%".
*
*=============================================================================
* Change History:
*
* Date       Name/Change
* ---------- -----------------------------------------------------------------
* 2010/11/07 Ed Trettevik (version 0.8.5 - split out from nbcmd.c)
* 2010/11/07 eat 0.8.5  nbParseSourceX names changed to nbSourceX
* 2010/11/07 eat 0.8.5  Supporting "\" at end of line for continuation
* 2011-02-19 eat 0.8.5  Fixed bug in IF with source substitution in skipped block
*            Was not acception "% " in a IF or ELSE block being skipped over
*=============================================================================
*/
#include <nbi.h>

/*
*  Get a line from a source file
*
*  This function is a wrapper for fgets and has the same parameters.
*  It calls fgets multiple times when lines end with a continuation
*  symbol "\".
*/
char *nbSourceGet(char *buffer,size_t size,FILE *file){
  char *buf=buffer;
  size_t len;
  char *last,*next;

  if(fgets(buf,size,file)==NULL) return(NULL);
  len=strlen(buf);
  last=buf+len-1;
  while(strchr(" \n\r",*last) && last>buf) last--;
  while(*last=='\\'){
    len=last-buf;
    size-=len;
    buf+=len;
    *buf=0;
    if(fgets(buf,size,file)==NULL) return(buffer);
    for(next=buf;*next==' ';next++);
    if(next>buf) strcpy(buf,next);
    len=strlen(buf);
    last=buf+len-1;
    while(strchr(" \n\r",*last) && last>buf) last--;
    }
  return(buffer);
  }

/*
*  Parse a source file, ignoring lines until terminating directive.
*
*  Terminating directive codes:
*
*    -1 - syntax error
*     0 - eof or exit (exit only during active parsing)
*     1 - endif
*     2 - else
*     3 - elseif
*
*  Til codes:
*
*     1 - endif, eof, or syntax error.
*     3 - elseif, else, endif, eof, syntax error.
*/
int nbSourceIgnoreTil(nbCELL context,FILE *file,char *buf,int til){
  int directive;
  char symid,*cursor,ident[256];

  //while(fgets(buf,NB_BUFSIZE,file)!=NULL){
  while(nbSourceGet(buf,NB_BUFSIZE,file)!=NULL){
    if(*buf=='%' && *(buf+1)!=' '){
      cursor=buf+1;
      symid=nbParseSymbol(ident,&cursor);
      if(strcmp(ident,"if")==0){
        directive=nbSourceIgnoreTil(context,file,buf,1);
        if(directive!=1) return(directive);  /* should be 0 or -1 */
        }
      else if(strcmp(ident,"endif")==0) return(1);
      else if(strcmp(ident,"else")==0){
        if(til>1) return(2);
        }
      else if(strcmp(ident,"elseif")==0){
        if(til>1) return(3);
        }
      /* ignore any other directives - but make sure we recognize them */
      else if(strcmp(ident,"assert")==0);
      else if(strcmp(ident,"default")==0);
      else if(strcmp(ident,"include")==0);
      else if(strcmp(ident,"quit")==0);
      else{
        outMsg(0,'E',"Directive not recognized.");
        return(-1);
        }
      }
    if(sourceTrace) outPut("!] %s",buf);
    }
  return(0);
  }

/*
*  Parse %if directive
*
*     %if(<condition>) [; # comment]
*     ...
*     %elseif(<condition>) [; # comment]
*     ...
*     %else [; # comment]
*     ...
*     %endif [; # comment]
*
* Symbolic substitution has already been applied to the %if directive and is applied
* here for the %elseif directive.
*/
int nbSourceIf(nbCELL context,FILE *file,char *buf,char *cursor){
  int directive,doif=0;
  NB_Object *object,*value;
  NB_Term *addrContextSave=addrContext;
  while(1){ /* until we bail with no more alternatives */
    while(*cursor==' ') cursor++;
    if(*cursor!='('){
      outMsg(0,'E',"Expecting '(' at \"%s\".",cursor);
      return(0);
      }
    /* get condition object */
    cursor++;
    addrContext=symContext;
    if(NULL==(object=grabObject(nbParseCell(symContext,&cursor,0)))){
      outMsg(0,'E',"Error in %cif condition - script terminating.");
      addrContext=addrContextSave;
      return(-1);
      }
    if(sourceTrace){
      outPut("Condition: ");
      printObject(object);
      outPut("\n");
      }
    addrContext=addrContextSave;
    if((value=object->type->compute(object))==NB_OBJECT_TRUE) doif=1;
    dropObject(object);
    if(sourceTrace){
      outPut("Value: ");
      printObject(value);
      outPut("\n");
      }
    dropObject(value);
    if(doif){  /* true - process to terminator */
      switch(nbSourceTil(context,file)){
        case -1: return(-1);
        case 0:
          outMsg(0,'E',"End of file before %cendif directive.",'%');
          break;
        case 1: /* %endif */
          return(1);
        case 2: /* %else */
          return(nbSourceIgnoreTil(context,file,buf,1));
          break;
        case 3: /* %elseif */
          return(nbSourceIgnoreTil(context,file,buf,1));
          break;
        default:
          outMsg(0,'L',"Unrecognized return code from nbSourceTil()");
          return(-1);
        }
      }
    else{ /* condition is not true, ignore to next alternative */
      directive=nbSourceIgnoreTil(context,file,bufin,3); /* look for elseif,else or endif */
      if(directive<0 || directive==1) return(directive);
      if(directive==0){
        outMsg(0,'E',"End of source file reached before %cendif directive.",'%');
        return(-1);
        }
      if(directive==2){
        directive=nbSourceTil(context,file);
        if(directive<0 || directive==1) return(directive);
        if(directive==0){
          outMsg(0,'E',"End of source file reached before %cendif directive.",'%');
          return(-1);
          }
        if(directive==2){
          outMsg(0,'E',"Directive %celse not expected.",'%');
          return(-1);
          }
        }
      if(directive!=3){
        outMsg(0,'L',"nbSourceIf(): Unexpect code returned by nbSourceIgnoreTil().");
        return(-1);
        }
      /* if directive is 3 "elseif" then we loop back to try again */
      if(sourceTrace) outPut("] %s",bufin);
      buf=nbSymSource((NB_Cell *)symContext,bufin);
      if(buf==NULL) return(-1);
      if(symbolicTrace) outPut("cmd1$ %s",buf);
      cursor=buf+7; /* step over %elseif */
      }
    }
  }

/*
*  Parse source file until error, eof, or terminating directive
*
*  Return Codes:
*
*   -1 - syntax error
*    0 - end-of-file or exit
*    1 - endif
*    2 - else
*    3 - elseif
*/
int nbSourceTil(nbCELL context,FILE *file){
  char *cursor,symid,ident[256],*buf;
  //while(fgets(bufin,NB_BUFSIZE,file)!=NULL){
  while(nbSourceGet(bufin,NB_BUFSIZE,file)!=NULL){
    /* When we're not in check mode ignore check command "~" */
    if((nb_mode_check==0 && *bufin!='~') || (nb_mode_check && outCheck(NB_CHECK_LINE,bufin))){
      if(sourceTrace) outPut("] %s",bufin);
      buf=bufin;
      while(*buf==' ') buf++;
      if(*buf=='%' && *(buf+1)==' '){
        buf=nbSymCmd((NB_Cell *)symContext,bufin,"%{}");
        if(buf==NULL) return(-1);
        if(symbolicTrace) outPut("cmd2$ %s\n",buf);
        }
      /* handle directive statements %if(), %elseif( ), %else, %endif */
      if(*buf=='%'){
        cursor=buf+1;
        symid=nbParseSymbol(ident,&cursor);
        if(symid=='t'){
          if(strcmp(ident,"quit")==0) return(0);
          else if(strcmp(ident,"endif")==0) return(1);
          else if(strcmp(ident,"else")==0) return(2);
          else if(strcmp(ident,"elseif")==0) return(3);
          else if(strcmp(ident,"if")==0) nbSourceIf(context,file,buf,cursor);
          else if(strcmp(ident,"assert")==0 && nbLet(cursor,symContext,0)!=0) return(-1);
          else if(strcmp(ident,"default")==0 && nbLet(cursor,symContext,1)!=0) return(-1);
          else if(strcmp(ident,"include")==0) nbSource(context,cursor);
          else{
            outMsg(0,'E',"Directive \"%s\" not recognized.",ident);
            return(-1);
            }
          }
        else{
          outMsg(0,'E',"Expecting directive identifier following %c.",'%');
          return(-1);
          }
        }
      else{
        // 2009-01-31 eat - uncommented
        if(!nb_ClockAlerting) nbClockAlert();
        nbCmd((NB_Cell *)context,buf,1);
        }
      }
    }
  return(0);
  }

/*
*  Include command file with symbolic substitution
*/
void nbSource(nbCELL context,char *cursor){
  FILE *file;
  char filename[256],*fcursor;
  //char buf[NB_BUFSIZE];
  long fileloc; /* file location test */
  NB_Term *symContextSave=symContext;  /* save symbolic context */

  symContext=nbTermNew(symContext,"%",nbNodeNew());
  while(*cursor==' ') cursor++;
  if(*cursor=='~'){
    nb_mode_check=4;  /* flag check mode, will flag 2 bit if check script (name ends with "~") */
    cursor++;
    }
  if(*cursor=='"'){
    cursor++;
    fcursor=strchr(cursor,'"');
    if(fcursor==NULL){
      outMsg(0,'E',"Unbalanced quote in file name");
      symContext=symContextSave;
      return;
      }
    if(sizeof(filename)-1<fcursor-cursor){
      outMsg(0,'E',"File name too long - %d bytes supported",sizeof(filename)-1);
      symContext=symContextSave;
      return;
      }
    strncpy(filename,cursor,fcursor-cursor);
    *(filename+(fcursor-cursor))=0;
    cursor=fcursor+1;
    }
  else{
// On Windows we allow unquoted file names with spaces, but require
// a comma before assertions.
//
//      > c:\Program Files\nodebrain\nb.exe "c:\Program Files\nodebrain\service\client.nb"
//
// On Unix/Linux we allow a space as a file name delimiter (for backward compatibility)
//
#if defined(WIN32)
    for(fcursor=filename;*cursor!=0 && *cursor!=';' && *cursor!=',';fcursor++){
#else
    for(fcursor=filename;*cursor!=' ' && *cursor!=0 && *cursor!=';' && *cursor!=',';fcursor++){
#endif
      *fcursor=*cursor;
      cursor++;
      }
    *fcursor=0;
    }
  while(*cursor==' ') cursor++;
  if(*cursor==',') cursor++;
  else while(*cursor==' ') cursor++;
  if(*cursor!=0 && *cursor!=';'){
    if(nbLet(cursor,symContext,0)!=0) return;
    }
  if(strcmp(filename,"-")==0) nbParseStdin(1);
  else if(strcmp(filename,"=")==0) nbParseStdin(0);
  else{
    if((file=fopen(filename,"r"))==NULL)
      outMsg(0,'E',"Source file \"%s\" not found.",filename);
    else{
      if(nb_mode_check) outCheck(NB_CHECK_START,filename);
      if(nbSourceTil(context,file)!=0){
        if(nb_mode_check) outCheck(NB_CHECK_STOP,NULL);
        outMsg(0,'E',"Source file \"%s\" terminated with errors.",filename);
        }
      else{
        if(nb_mode_check) outCheck(NB_CHECK_STOP,NULL);
        fileloc=ftell(file);
        outMsg(0,'I',"Source file \"%s\" included. size=%u",filename,fileloc);
        }
      fclose(file);
      }
    }
  termUndef(symContext);      // undefine symbolic context
  symContext=symContextSave;  // restore symbolic context
  }
