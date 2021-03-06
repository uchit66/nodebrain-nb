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
* Program:  nodebrain
*
* File:     nbsynapse.c 
* 
* Title:    Synapse Management Routines
*
* Function:
*
*   This header provides routines that manage NodeBrain synapses (NB_Synapse), an
*   extension of NB_Object.  These routines provide a mechanism on top of the
*   cell subscription and publication scheme that enables specific actions in
*   response to changes in specific cells.
*
*   
* Synopsis:
*
*   (Internal):
*
*   #include "nb.h"
*
*   void nbSynapseInit();
*
*   (API):
*
*   #include "nbapi.h"
*
*   void nbSynapseInit(NB_Stem *stem);
*   nbCELL nbSynapseOpen(nbCELL context,void *skillHandle,void *nodeHandle,nbCELL cell,
*          void (*handler)(nbCELL context,void *skillHandle,void *nodeHandle,nbCELL cell)); 
*   void nbSynapseSetTimer(nbCELL synapse,int seconds);
*   void *nbSynapseClose(nbCELL synapse);
*
*
* Description
*
*   nbSynapseInit()    - Initialize synapse object environment
*   nbSynapseOpen()    - Construct a synapse object and subscribe to specific cell
*   nbSynapseClose()   - unsubscribe to specific cell and destroy synapse 
*
*   A synapse is a simple object that subscribes to a single cell and calls a
*   handler function whenever the cell changes.  The call to the handler is similar
*   to that of a skill module method.
*
*   We do not provide much visibility for synapses.  For now we are willing to
*   allow the handle and handler to be invisible; that is, we do not provide
*   for the display of handles and handlers.  To support the "show impact"
*   option, we provide a minimal print method for a synapse.  
*
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2004-12-02 Ed Trettevik (original prototype version introduced in 0.6.2)
* 2007-07-21 eat 0.6.8  Modified to simplify use within skill modules.
* 2008-05-24 eat 0.7.0  Modified to include nbSynapseSetTimer function
* 2010-02-28 eat 0.7.9  Cleaned up -Wall warning messages. (gcc 4.5.0)
* 2010-06-16 eat 0.8.2  Modified nbSynapseSetTimer to cancel timer when interval is zero
* 2014-05-04 eat 0.9.02 Replaced newType with nbObjectType
*=============================================================================
*/
#include <nb/nbi.h>

struct NB_SYNAPSE *nb_SynapsePool;
struct TYPE *nb_SynapseType;

void nbSynapsePrint(struct NB_SYNAPSE *synapse){
  outPut("(synapse)");
  }

void nbSynapseAlert(struct NB_SYNAPSE *synapse){
  (*synapse->handler)(synapse->context,synapse->skillHandle,synapse->nodeHandle,synapse->cell);
  }

/*
*  Initialization Cells 
*/
void nbSynapseInit(NB_Stem *stem){
  nb_SynapseType=nbObjectType(stem,"synapse",0,0,nbSynapsePrint,NULL);
  nb_SynapseType->alert=nbSynapseAlert;
  nb_SynapseType->alarm=nbSynapseAlert;
  }

/*
*  Open a synapse
*/
NB_Cell *nbSynapseOpen(NB_Cell *context,void *skillHandle,void *nodeHandle,NB_Cell *cell,
  void (*handler)(NB_Cell *context,void *skillHandle,void *nodeHandle,NB_Cell *cell)){

  NB_Synapse *synapse;
  synapse=newObject(nb_SynapseType,(void **)&nb_SynapsePool,sizeof(struct NB_SYNAPSE));
  synapse->context=context;
  synapse->skillHandle=skillHandle;
  synapse->nodeHandle=nodeHandle;
  synapse->cell=cell;
  synapse->handler=handler;
  if(trace) nbLogMsg(context,0,'T',"nbSynapseOpen: calling nbAxonEnable");
  if(cell) nbAxonEnable(cell,(NB_Cell *)synapse); /* subscribe to cell */
  return((NB_Cell *)synapse);
  }

/*
* Schedule a synapse to fire after a specified number of seconds
*
*   You can schedule a synapse to fire after zero seconds to cause it to fire 
*   immediately after giving other events an opportunity to fire.
*
* 2010-06-16 eat 0.8.2 - modified to cancel timer when seconds is zero
*/
void nbSynapseSetTimer(nbCELL context,nbCELL synapse,int seconds){
  time_t at;
  //outMsg(0,'T',"nbSynapseSetTimer: seconds=%d synapse=%p",seconds,synapse);
  if(seconds){
    time(&at);
    seconds+=at;
    }
  nbClockSetTimer(seconds,synapse);
  }

/*
*  Close a synapse and obtain the handle
*    
*    The caller is responsible for disposing of the handle if necessary
*
*  Returns: handle pointer  
*/
void *nbSynapseClose(NB_Cell *context,NB_Cell *synapse){
  if(((NB_Synapse *)synapse)->cell) nbAxonDisable(((NB_Synapse *)synapse)->cell,(NB_Cell *)synapse);
  synapse->object.next=(NB_Object *)nb_SynapsePool;
  nb_SynapsePool=(NB_Synapse *)synapse;
  return(NULL);
  }
