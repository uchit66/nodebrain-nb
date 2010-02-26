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
* File:     nbcell.c 
* 
* Title:    Cell Management Routines
*
* Function:
*
*   This header provides routines that manage NodeBrain cells (NB_Cell), an
*   extension of NB_Object.  These routines are used for change registration,
*   change publication, and overall control of the evalution process.
*

* Synopsis:
*
*   #include "nb.h"
*
*   void nbCellInit();
*
*   void nbCellType(
*     NB_Type *type, 
*     void (*solve)(),
*     NB_Object * (*eval)(),
*     void (*enable)(),
*     void (*disable)());
*
*   void nbCellTypeSub(
*     NB_Type *type,
*     int reg,                      
*     NB_Object *(*parse)(),
*     NB_Object *(*construct)(),
*     double (*evalDouble)(),
*     char *(*evalString)());
*  
*   void *nbCellNew(NB_Type *type,void **pool,short length);
*
*   void nbCellPrintSpecial(NB_Object *object);
*
*   NB_Object *nbCellEvalTrace(NB_Cell *object);
*   
*   NB_Object *nbCellSolve_(NB_Cell *cell);
*
*   NB_Object *nbCellCompute_(NB_Cell *cell);
*
*   void nbCellEnable(NB_Cell *publisher,NB_Object *subscriber);
*
*   void nbCellDisable(NB_Cell *publisher,NB_Object *subscriber);
*
*   void nbCellPublish(NB_Cell *publisher);
*
*   void nbCellLevel(NB_Cell *pub);
*
*   void nbCellShowImpact(NB_Cell *cell);
*
*   void nbCellAlert(NB_Cell *sub);
*
*   void nbCellReact();
*
*
* Methods:
*
*   Although we are not using object oriented features of C++ we are using an
*   object oriented conceptual model.  While this file provides methods available
*   to all cell types, unique types of cells are created by providing the following
*   methods.
*
*   Eval        - Evaluate uses existing parameter values
*
*   Solve       - Evaluate after requesting a solution to subordinate cells.
*
*   Compute     - Evaluate after requesting a computed value for subordinate cells.
*                 This method is for getting a value from a disabled cell.  By
*                 default, this method is set to nbCellCompute(), which calls
*                 eval on enabled cells, and enable/eval/disable on disabled
*                 cells.  Cell extensions should provide a more efficient method
*                 that avoids the overhead of enabling and disabling.
*
*                 Because the compute method operates on disabled cells, it returns
*                 a "grabbed" object which must be dropped by the caller when no
*                 longer needed.
*
*   Enable      - Evaluate after subscribing to (enabling) subordinate cells.
*                 This method provides for a cell value being automatically
*                 maintained via alerts (see alert method below) when subordinate
*                 cell values change.
*
*   Disable     - Unsubscribe to subordinate cells and set value to nb_Disabled.
*   
*   EvalDouble  - Specific evaluation routine for real number parameters 
*
*   EvalString  - Specific evaluation method for string parameters
*
*   Since NB_Cell extends NB_Object, the following methods are also available
*   for use.
*
*   Print       - Print a string representation of cell.
*
*   Destroy     - Drop referenced objects and release cell memory.
*
*   Alert       - By default, all cells use nbCellAlert() as their Alert method.
*                 This function schedules the cell for evaluation based on
*                 level of reference in a hierachy of cells.  A cell type
*                 designed for top level use may replace this method.  Others
*                 may replace it but call nbCellAlert() from the replacement.
*
*   Parse       - Optional parsing method.  Not currently called by cell methods
*                 provided here. 
*
*   Construct   - Optional cell initialization method.  Not currently called
*                 by cell methods provided here.
*
*
* 
* Description
*
*   The functions in this file are services to, and users of, cell methods. They
*   satisfy the common needs of a cell.
*
*   nbCellInit            - Initialize cell object environment
*   nbCellType            - Initialize methods for a new cell type
*   nbCellTypeSub         - Initialize methods for a sub type
*   nbCellNew             - Allocate a new cell instance
*   nbCellPrintSpecial    - Print special static object types used by cell routines
*   nbCellEvalTrace       - Shim function for tracing cell evaluation
*   nbCellSolve_          - Solve for unknown cell values
*   nbCellCompute_        - Compute a value for a disabled cell
*   nbCellEnable          - Subscribe to parameter function value changes
*   nbCellDisable         - Cancel subscription to function value changes
*   nbCellPublish         - Publish cell value change to all subscribers
*   nbCellLevel           - Adjust cell level relative to modified subordinates
*   nbCellAlert           - Standard cell alert method - schedules re-evaluation
*   nbCellReact           - React to published changes 
*
*   The table below is incomplete, but give a general idea of how the cell functions
*   provided here interact with the methods of a given type of cell.
*
*   Calling Function      Method          Called Functions
*   ----------------      ------------    ---------------------------------------
*   define/assert         Use/New         nbCellNew() to create a new instance
*   nbCellEnable()        Enable          nbCellEnable()
*   nbCellEnable()        Eval            optionally nbCellEnable/Disable()
*   nbAssert()                            nbCellPublish()
*   nbCellPublish()       Alert           nbCellAlert() schedules for nbCellReact()
*   nbCellReact()         Eval
*   nbCellDisable()       Disable         nbCellDisable()
*   nbCellDisable()       Destory         nbCellDisable() and dropObject()
*   nbCellSolve_          Solve           nbCellSolve() for subordinate cells
*   printObject()         Print           outPut()
*
*
*   The relationship between the enable and disable functions and methods is shown
*   in more detail here.  Evaluation and publishing up the cell hierarchy has a
*   similar recursive structure.
*
*   nbCellEnable(cell,object)
*      o  If object not null, place in cell's subscription list
*      o  If cell not disabled, return (we already have a value)
*      o  Call cell's enable method
*      o  Call cell's eval method and set cell.object.value. 
*
*   Cell.Enable(cell) - Method
*      o  Call nbCellEnable() for subordinate cells
*
*   nbCellDisable(cell,object)
*      o  If cell is a static object (not cell), return.
*      o  If cell is disabled, return (we don't have anything to do.
*      o  If object not null, remove object from cell's subscription list
*      o  If the subscription list is null, call Disable and set cell.object.value=nb_Disabled
*         [NB_Term is an exception that may not be disabled.]
*
*   Cell.Disable(cell) - Method
*      o  Call nbCellDisable() for subordinate cells as appropriate
*
*
*   For each type of cell, the following steps are required.
*
*   1) Define a structure using an NB_CELL header.
*
*        struct MYOBJ{
*          NB_Cell cell;
*          ...
*          };
*
*   2) Define a type structure;
*
*        NB_Type myobjType;
*
*   3) Write an "init" method to perform initialization functions.  In
*      particular, this method must call newType to define the type.
*
*        myobjType=newType(stem,myobjType,hash,attributes,myobjPrint,myobjDestroy);
*
*      Then nbCellType() is called to define cell methods.
*
*        nbCellType(solveMyobj,evalMyobj,enableMyobj,disableMyobj);
*
*      The method arguments are routines you write as described later.
*
*   4) Write a "locate" method.  This method will be called by other code
*      you write.  It should use the parameters passed to search for
*      the existance of a matching instance.  Return 1 if found and 0 if
*      not found.  The hash field of the TYPE structure and the next field
*      of the NB_Object structure are used along with the parameters to hash
*      search.  If found the object field of the TYPE structure is set to
*      the location of the object found.  If not found, the object field
*      of the TYPE structure is set to the instance where a new instance
*      should be inserted. 
*
*        myobject *myobjLocate(...){ ... }
*
*      This method is only required for objects you will need to be able
*      to locate.
*
*   5) Write a "use" method.  This method will be called by other
*      code you write.
*
*        myobject *myobjUse(...){ ... }
*
*      If you have a locate method, call it to see if an instance already
*      exists.  If so, just return the address returned by the locate method.
*      If you do not have a locate method, or the locate method returned zero,
*      you must call newObject or nbCellNew to allocate a new instance.
*
*        myobj=newObject(mytype,pool,length);
*
*        myobj=nbCellNew(mytype,pool,length);
*
*      The constructor must initialize all fields after the object header.
*      (The header is initialized by newObject or nbCellNew.)  If your object
*      structure contains pointers to other objects, you must call grabObject()
*      to increment the reference count.  You may also want to call nbCellEnable()
*      to subscribe to changes to the referenced object.
* 
*   6) Write an Eval method.  This method is responsible for taking action
*      when referenced variables change.  It will only be called for objects
*      that have subscribed to changes, and only when objects publish a
*      a change.
*
*        NB_Object *evalMyobj(struct MYOBJ *myobj){ ... }
*
*      You should consider carefully if you want to call nbCellEnable() and
*      nbCellDisable() in this method.  Under some conditions you may not
*      need to know the current value of other objects.  In particular,
*      when reacting to a change, if nobody has subscribed to the
*      reacting object, it may be appropriate to cancel subscriptions.
*      This may be preferable to cancelling subscriptions immediately
*      when all dependent subscriptions are concelled.  It also depends
*      on how resource intensive an unneccesary action is, and how often
*      subscriptions will be cancelled and reinstated.
*
*   7) Write an "enable" method.  This is only required for variable objects;
*      that is, objects that publish changes.  This method is called when a
*      subscription is registered to a disabled object.  This means some other
*      object now depends on the object being enabled, so you may need to call
*      nbCellEnable() to subscribe to referenced objects.  If you don't subscribe
*      or never cancel subscriptions, you may use the nullEnable method.
*
*      For constant objects, use a NULL enable method.
*      
*   8) Write a "disable" method.  This is only required for variable objects;
*      that is, objects that publish changes.  This method is called when
*      all subscriptions are cancelled.  In this case, nobody seems to care
*      about the object at the moment, so you may want to call nbCellDisable() 
*      to cancel subscriptions to referenced objects.  You may use the
*      nullDisable method for variable objects not requiring a disable method.
*
*      For constant objects, use a NULL disable method.
*
*   9) Write a "destroy" method.  This method is required to call
*      dropObject() for every object it referenced with a grabObject().
*
*        void myobjDestroy(struct MYOBJ *myobj){ ... }
*
*      If you have called nbCellEnable() to subscribe for changes to other
*      cells, you must call nbCellDisable() in the destroy method.
*
*  10) Write a "print" method. This method should print a symbolic 
*      representation of your object.
*
*=============================================================================
* Change History:
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2001/07/08 Ed Trettevik (original prototype version introduced in 0.2.8)
*             1) This code is a generalization of code previously imbedded
*                in nodebrain.c
*             2) split out from nbobject.h
* 2002/08/20 eat - version 0.4.0
*             1) nbCellType now sets the object type alert method to
*                nbCellAlert().  This was done to conform to a more general
*                model for object alerting.
* 2002/08/25 eat - version 0.4.0
*             1) moved MEMBER handling code to nblist.h
* 2003/11/03 eat 0.5.5  Renamed functions and variables and fixed some bugs.
*            1) nbCellDisable was disabling terms in error.
* 2005/05/08 eat 0.6.2  Enabled an object to subscribe multiple times to a cell
*            It is silly to burden an object with worrying about how many times
*            it has subscribed to a cell.  If an object subscribes to a cell
*            multiple times, we now assume it has a good reason and will
*            unsubscribe multiple times as well.  For example, addition (a+a)
*            will subscribe to "a" twice and unsubscribe twice.
* 2006/12/21 eat 0.6.6  Included loop protection in nbCellEnable() and nbCellLevel()
* 2007/01/02 eat 0.6.6  Slight enhancement to loop protection in nbCellLevel()
* 2010/02/25 eat 0.7.9  Cleaned up -Wall warning messages
*=============================================================================
*/
#include "nbi.h"
#if !defined(WIN32)
#include <sys/times.h>
#endif
 
//extern int nb_opt_shim;
//extern int trace;
//extern int queryTrace;

#define maxLevel 100              /* highest level */

NB_Link   *regfun;          /* registered functions - fun(list) */
NB_Link   *change=NULL;     /* change condition list */
NB_TreeNode **evalVector;      /* function evaluation tree */
NB_TreeNode **evalVectorTop;   /* top level tree */

/* condition function values
*/

nbCELL NB_CELL_DISABLED;
#if defined(WIN32)
__declspec(dllexport) 
#endif
nbCELL NB_CELL_UNKNOWN;
#if defined(WIN32)
__declspec(dllexport) 
#endif
nbCELL NB_CELL_PLACEHOLDER;
#if defined(WIN32)
__declspec(dllexport) 
#endif
nbCELL NB_CELL_FALSE;
NB_Object *NB_OBJECT_FALSE;
#if defined(WIN32)
__declspec(dllexport) 
#endif
nbCELL NB_CELL_TRUE;
NB_Object *NB_OBJECT_TRUE;


/*
*  Initialization Cells 
*/
void nbCellInit(){
  NB_OBJECT_TRUE=(NB_Object *)useReal((double)1);
  NB_OBJECT_TRUE->refcnt=(unsigned int)-1;         /* flag as perminent object */
  NB_OBJECT_FALSE=(NB_Object *)useReal((double)0);
  NB_OBJECT_FALSE->refcnt=(unsigned int)-1;        /* flag as perminent object */
  NB_CELL_DISABLED=(NB_Cell *)nb_Disabled;
  NB_CELL_UNKNOWN=(NB_Cell *)nb_Unknown;
  NB_CELL_PLACEHOLDER=(NB_Cell *)nb_Placeholder;
  NB_CELL_FALSE=(NB_Cell *)NB_OBJECT_FALSE;
  NB_CELL_TRUE=(NB_Cell *)NB_OBJECT_TRUE;
  evalVector=calloc(maxLevel,sizeof(void *));
  evalVectorTop=evalVector;
  }

#if !defined(WIN32)
/*
*  Cell alarm trace shim
*/
void nbCellAlarmTrace(NB_Cell *cell){
  //NB_Object *value;
  clock_t ticks;
  struct tms tms_times;

  times(&tms_times);
  ticks=tms_times.tms_utime;
  cell->object.type->shim->alarm(cell);
  outPut("Shim trace Alarm %s(",cell->object.type->name);
  printObject(cell);
  outPut(") ");
  times(&tms_times);
  ticks=tms_times.tms_utime-ticks;
  outPut(" ticks=%le  utime=%le stime=%le\n",(double)ticks,(double)tms_times.tms_utime,(double)tms_times.tms_stime);
  outFlush();
  cell->object.type->shim->alarmTicks+=ticks;
  }

/*
*  Cell alert trace shim
*/
void nbCellAlertTrace(NB_Cell *cell){
  //NB_Object *value;
  clock_t ticks;
  struct tms tms_times;

  times(&tms_times);
  ticks=tms_times.tms_utime;
  cell->object.type->shim->alert(cell);
  outPut("Shim trace Alert %s(",cell->object.type->name);
  printObject(cell);
  outPut(") ");
  times(&tms_times);
  ticks=tms_times.tms_utime-ticks;
  outPut(" ticks=%le  utime=%le stime=%le\n",(double)ticks,(double)tms_times.tms_utime,(double)tms_times.tms_stime);
  outFlush();
  cell->object.type->shim->alertTicks+=ticks;
  }

/*
*  Cell evaluation trace shim
*/
NB_Object *nbCellEvalTrace(NB_Cell *cell){
  NB_Object *value;
  clock_t ticks;
  struct tms tms_times;

  times(&tms_times);
  ticks=tms_times.tms_utime;
  value=cell->object.type->shim->eval(cell);
  outPut("Shim trace Eval %s(",cell->object.type->name);
  printObject(cell);
  outPut(")=> ");
  printObject(value);
  times(&tms_times);
  ticks=tms_times.tms_utime-ticks;
  outPut(" ticks=%le  utime=%le stime=%le\n",(double)ticks,(double)tms_times.tms_utime,(double)tms_times.tms_stime);
  outFlush();
  cell->object.type->shim->evalTicks+=ticks;
  return(value);
  }
#endif

void nbCellType(NB_Type *type,void (*solve)(),NB_Object * (*eval)(),void (*enable)(),void (*disable)()){
#if !defined(WIN32)
  struct NB_TYPE_SHIM *shim;
#endif
  
  type->alert=&nbCellAlert;
  type->alarm=&nbCellAlert; /* time conditions handle an alarm like an alert */
                            /* rules provide a separate method for alarms    */
  if(solve!=NULL) type->solve=solve;
  type->compute=&nbCellCompute_;
  if(eval!=NULL) type->eval=eval;
  if(enable==NULL) type->enable=&enableBug;
  else type->enable=enable;
  if(disable==NULL) type->disable=&disableBug;
  else type->disable=disable;
#if !defined(WIN32)
  if(nb_opt_shim){
    shim=malloc(sizeof(struct NB_TYPE_SHIM));
    memset(shim,0,sizeof(struct NB_TYPE_SHIM));
    type->shim=shim;
    shim->alarm=type->alarm;
    type->alarm=&nbCellAlarmTrace;
    shim->alert=type->alert;
    type->alert=&nbCellAlertTrace;
    shim->eval=type->eval;
    type->eval=&nbCellEvalTrace;
    }
#endif
  }

/*
*   A sub type handles variation within a type family.
*     For example, a bunch of two parameter math functions
*     can share some methods but need different eval sub
*     methods.
*/
void nbCellTypeSub(
  NB_Type *type,
  int reg,                      /* 1 if the function is to be registered */
  NB_Object *(*parse)(),
  NB_Object *(*construct)(),
  double (*evalDouble)(),
  char *(*evalString)()){

  if(parse!=NULL)   type->parse=parse;
  if(construct!=NULL)  type->construct=construct;
  if(evalDouble!=NULL) type->evalDouble=evalDouble;
  if(evalString!=NULL) type->evalString=evalString;
  if(reg) listInsert(&regfun,type);
  }

/*
*  Create a new cell instance
*/
void *nbCellNew(NB_Type *type,void **pool,int length){
  NB_Cell *cell;
  cell=newObject(type,pool,length);
  cell->object.value=nb_Disabled;
  cell->sub=NULL;
  cell->level=0;
  return(cell);
  }

/*
*  Solve for a cell value (typically a cell represents a cell expression).
*/  
NB_Object *nbCellSolve_(NB_Cell *cell){
  if(cell==NULL) return(nb_Unknown);  
  if(cell->object.value!=nb_Unknown && cell->object.value!=nb_Disabled) return(cell->object.value);
  if(trace || queryTrace) {
    if(trace) outMsg(0,'T',"nbCellSolve_() called");
    outPut("cell:");
    printObject(cell);
    outPut("\n");
    }
  if(trace) outMsg(0,'T',"nbCellSolve_() calling cell's \"solve\" method.");
  cell->object.type->solve(cell);
  if(trace || queryTrace) {  
    if(trace) outMsg(0,'T',"nbCellSolve_() returning");
    outPut("cell: ");
    printObject(cell);
    outPut("\nSolution: ");
    printObject(cell->object.value);   
    outPut("\n");
    }    
  return(cell->object.value);
  }

/*
*  Default computation method for disabled cells.
*
*    A disabled cell is one that hasn't subscribed for alerts from
*    subordinate cells.
*
*    This routine has overhead in the enable/disable calls that
*    can be eliminated by providing an alternate compute method
*    that calls subordinates compute methods.  This one is provided
*    for convenience now, but should be replaced per cell type.
*
*    To prevent the nbCellDisable() call from destroying the result
*    object, we grab it before calling nbCellDisable().  It is the
*    the caller's responsibility to drop the returned object when
*    it is no longer needed.
*/
NB_Object *nbCellCompute_(NB_Cell *cell){
  NB_Object *object;
  if(cell->object.value!=nb_Disabled) return(grabObject(cell->object.value));
  nbCellEnable(cell,NULL);
  object=grabObject(cell->object.value);
  nbCellDisable(cell,NULL);
  return(object);
  }
 
/*
*  Enable value change publication from a cell to a subscriber object).
*  
*    o  It is safe to enable a NULL publisher or a non-publishing object.
*       These calls are ignored.  This is done so the caller doesn't need to
*       test object pointers before calling.
*
*    o  We are currently maintaining an ordered list, although this
*       shouldn't be necessary.  The caller should be responsible for only
*       substribing once.  For now we order the list and watch for logic 
*       errors.
*
*    o  A subscriber is not required to be a cell---may be a simple object.
*/
//void nbCellEnable(NB_Cell *pub,NB_Object *sub){
void nbCellEnable(NB_Cell *pub,NB_Cell *sub){
  //NB_Link *list;
  //NB_Link **listP;
  if(pub->object.value==(NB_Object *)pub) return;

  if(trace){
    outMsg(0,'T',"nbCellEnable() called - linking subscriber");
    outPut("subscriber: ");
    printObject(sub);
    outPut("\n"); 
    outPut("publisher : ");
    printObject(pub);
    outPut(" = ");
    printObject(pub->object.value);
    outPut("\n");
    }

  if(sub!=NULL){
    NB_TreePath treePath;
    NB_TreeNode *treeNode;
    treeNode=(NB_TreeNode *)nbTreeLocate(&treePath,sub,(NB_TreeNode **)&pub->sub);
    if(treeNode==NULL){
      treeNode=(NB_TreeNode *)nbAlloc(sizeof(NB_TreeNode));
      treeNode->key=sub;
      nbTreeInsert(&treePath,treeNode);
      }
    }
  if(trace) outMsg(0,'T',"nbCellEnable() completed subscription");
  if(pub->object.value!=nb_Disabled) return; // already know the value
  //pub->object.value=nb_Unknown;           // set value to Unknown to avoid looping
  if(trace) outMsg(0,'T',"nbCellEnable() calling publisher's enable method.");
  pub->object.type->enable(pub); /* pub's enable method */
  if(trace) outMsg(0,'T',"nbCellEnable() calling publisher's evaluate method.");
  pub->object.value=grabObject(pub->object.type->eval(pub));  /* pub's evaluation method */
  if(sub!=NULL && sub->object.value!=(NB_Object *)sub && sub->level<=pub->level){
    ((NB_Cell *)sub)->level=pub->level+1;
    nbCellLevel((NB_Cell *)sub);
    }
  if(trace){
    outMsg(0,'T',"nbCellEnable() returning");
    outPut("Function: ");
    printObject(pub);
    outPut("\nResult:");
    printObject(pub->object.value);
    outPut("\n"); 
    }
  }

/*
*  Cancel an object's subscription to cell changes.
*
*    We are assuming an unordered list here, but assume an object is
*    only referenced by one list entry.
*/
//void nbCellDisable(NB_Cell *pub,NB_Object *sub){
void nbCellDisable(NB_Cell *pub,NB_Cell *sub){
  //NB_Link *list;
  //NB_Link **listP;
  if(trace){
    outMsg(0,'T',"nbCellDisable() called");
    printObject(pub);
    outPut("\n");
    }
  if(pub->object.value==(NB_Object *)pub) return; /* static object */
  if(pub->object.value==nb_Disabled) return;
  if(sub!=NULL){
    NB_TreePath treePath;
    NB_TreeNode *treeNode;
    treeNode=nbTreeLocate(&treePath,sub,(NB_TreeNode **)&pub->sub);
    if(treeNode!=NULL){
      nbTreeRemove(&treePath);
      nbFree((NB_Object *)treeNode,sizeof(NB_TreeNode));  // this should be a macro
      }
    }
  if(pub->sub==NULL){
    pub->object.type->disable(pub);
    /* We make an exception for terms who stay enabled when their defined
    *  to have the value of a static object (not variable).  Perhaps we
    *  should leave it up to the disable functions, but all except NB_Term
    *  are currently depending on nbCellDisable() to turn the lights out.
    */
    if(pub->object.type!=termType){
      dropObject(pub->object.value);
      pub->object.value=nb_Disabled;
      }
    }
  if(trace) outMsg(0,'T',"nbCellDisable() returning");
  }

/*
*  Publish change to all subscriber objects
*/
void nbCellPublish(NB_Cell *pub){
  NB_TreeIterator treeIterator;
  NB_TreeNode *treeNode;
  if(trace){
    outMsg(0,'T',"nbCellPublish() called");
    printObject(pub);
    outPut("\n");
    }
  if(pub->object.value==(NB_Object *)pub) return; /* static object */
  NB_TREE_ITERATE(treeIterator,treeNode,pub->sub){
    ((NB_Object *)treeNode->key)->type->alert((NB_Cell *)treeNode->key); /* could be any object */
    NB_TREE_ITERATE_NEXT(treeIterator,treeNode) 
    }
  }

/*
*  Adjust cell levels
*    This function is called by nbTermAssign()
*    when expressions are assigned to a term 
*    with a level greater than or equal to the term.
*
*  We are not concerned with minimizing an expression's level,
*  so we never adjust levels when a term is assigned to an
*  expression with a level more than 1 below the term.
*
*  This function is just an experiment.  We need to make level
*  adjustment automatic within the enable (subscribe) and
*  disable (unsubscribe) process.  This function only adjusts
*  subscribed functions.
*  
*/
void nbCellLevelRecurse(NB_Cell *pub,NB_Cell *start){
  NB_TreeIterator treeIterator;
  NB_TreeNode *treeNode;
  if(trace) outMsg(0,'T',"nbCellLevelRecurse() called");
  if(pub==start){
    outMsg(0,'E',"Results are undefined for circular cell expressions.");
    printObject((NB_Object *)start);
    outPut("\n");
    start->level=0; // set level to zero to avoid looping
    return;
    }
  NB_TREE_ITERATE(treeIterator,treeNode,pub->sub){
    if(((NB_Cell *)treeNode->key)->level<=pub->level){
      ((NB_Cell *)treeNode->key)->level=pub->level+1;
      nbCellLevelRecurse((NB_Cell *)treeNode->key,start);
      }
    NB_TREE_ITERATE_NEXT(treeIterator,treeNode)
    }
  }

void nbCellLevel(NB_Cell *pub){
  NB_TreeIterator treeIterator;
  NB_TreeNode *treeNode;
  if(trace) outMsg(0,'T',"nbCellLevel() called");
  NB_TREE_ITERATE(treeIterator,treeNode,pub->sub){
    if(((NB_Cell *)treeNode->key)->level<=pub->level){
      ((NB_Cell *)treeNode->key)->level=pub->level+1;
      nbCellLevelRecurse((NB_Cell *)treeNode->key,pub);
      }
    NB_TREE_ITERATE_NEXT(treeIterator,treeNode)
    }
  }

/*
*  Print cells direct substribers  
*/  
void nbCellShowSub(NB_Cell *pub){
  NB_TreeIterator treeIterator;
  NB_TreeNode *treeNode;
  int level;
  if(pub->object.value==(NB_Object *)pub) return; /* static object */
  NB_TREE_ITERATE(treeIterator,treeNode,pub->sub){
    if(((NB_Object *)treeNode->key)->value==(NB_Object *)treeNode->key) level=pub->level+1;
    else level=((NB_Cell *)treeNode->key)->level;
    outPut("[%d]: ",level);
    printObject((NB_Object *)treeNode->key);
    outPut("\n");
    NB_TREE_ITERATE_NEXT(treeIterator,treeNode)
    }
  }
  
/*
*  Print objects subscribing to a cell  
*/  
void nbCellShowImpact(NB_Cell *cell){
  NB_TreeIterator treeIterator;
  NB_TreeNode *treeNode;
  NB_Link *member;
  NB_Link *vector[256];
  int v,level,top=0;
 
  if(cell->object.value==(NB_Object *)cell) return; /* static object */
  for(v=0;v<256;v++) vector[v]=NULL;  /* Init vector */
  v=cell->level;
  listInsertUnique(&vector[v],cell);  /* start with specified cell */
  top=v;
  for(;v<=top;v++){
    for(member=vector[v];member!=NULL;member=member->next){
      outPut("[%d]: ",v);
      printObject(member->object);
      if(member->object->type==termType){
        outPut(" == ");
        printObject(((NB_Term *)member->object)->def);
        }
      outPut("\n");
      if(member->object->value!=(NB_Object *)member->object){ /* cell - not static object */
        NB_TREE_ITERATE(treeIterator,treeNode,((NB_Cell *)member->object)->sub){
          cell=(NB_Cell *)treeNode->key;
          if(cell->object.value==(NB_Object *)cell) level=cell->level+1;
          else level=cell->level;
          if(level>v && level<256){
            listInsertUnique(&vector[level],(NB_Object *)cell);
            if(level>top) top=level;
            }
          else{
            if(level<=v) outMsg(0,'L',"Cell level error in following cell.");
            else outMsg(0,'L',"Cell level is too high to continue up.");
            outPut("[%d]: ",level);
            printObject(cell->object);
            outPut("\n");
            }
          NB_TREE_ITERATE_NEXT(treeIterator,treeNode)
          }
        }
      }
    nbListFree(vector[v]); /* free the list we just scanned */
    }
  }
 
/*
*  Alert method for cell object.  This function places a cell in a list
*  for evaluation based on reference level. It is normally called indirectly
*  by nbCellPublish() via the object alert method pointer.
*/
void nbCellAlert(NB_Cell *sub){
  NB_TreePath treePath;
  NB_TreeNode *treeNode,**treeNodeP;
  //outMsg(0,'T',"nbCellAlert() called");
  if(sub->object.value==(NB_Object *)sub) return; /* static object */
  //outMsg(0,'T',"nbCellAlert() inserting object in eval list level(%d).",sub->level);
  treeNodeP=evalVector+sub->level;
  if(treeNodeP>evalVectorTop) evalVectorTop=treeNodeP;
  treeNode=(NB_TreeNode *)nbTreeLocate(&treePath,sub,treeNodeP);
  if(treeNode==NULL){
    treeNode=(NB_TreeNode *)nbAlloc(sizeof(NB_TreeNode));
    treeNode->key=sub;
    nbTreeInsert(&treePath,treeNode);
    }
  }

/*
*  React to cell changes
*
* This function is used to adjust the value of cells when the values
* of subordinate cells change.  It is done by cell level, so changes
* purculate up the branches of multiple dependent trees.  Reevaluation
* is performed by calling a cell's eval method.
*
* Purculation is performed by level: 0, 1, 2, etc.  This ensures that
* the value of all parameters have been adjusted before a cell is
* re-evaluated.  (We do not allow for circular definitions.)
*
* A cell's Eval method is expected to return a newly evaluated value
* without modifying "cell.object.value".  This is left for nbCellReact()
* to do after comparing for a change.  This also allows Eval methods
* to bail out with a value in one statement - return(value).
*
* Because all objects have Alert methods, we are able to publish
* changes to objects that do not use the nbCellAlert() method, and
* are not placed in the evalVector used here.
* 
* Note: Think about how subscription lists might be organized to
* reduce re-evaluation.  For example, if 1000 test conditions are
* established for a single variable, and they are all testing for
* something like X>"something", if we ordered the subscription list
* by the values of "something" we wouldn't have to test them all.
* Some method of bounding the change may be appropriate.
*/
void nbCellReact(){
  NB_TreeIterator treeIterator;
  NB_TreeNode *treeNode,**treeNodeP,*freeNode;
  NB_Cell *cell;
  NB_Object *value;
  for(treeNodeP=evalVector;treeNodeP<=evalVectorTop;treeNodeP++){
    //outMsg(0,'T',"nbCellReact: before evalVector=%x rootP=%x root=%x",evalVector,treeNodeP,*treeNodeP);
    treeNode=*treeNodeP;
    *treeNodeP=NULL;
    NB_TREE_ITERATE2(treeIterator,treeNode,treeNode){
      cell=(NB_Cell *)treeNode->key;
      if(trace){
        outMsg(0,'T',"nbCellReact() calling object's eval method."); 
        outPut("Function:");
        printObject(cell);
        outPut("\n");
        }
      value=cell->object.type->eval(cell);
      if(trace){
        outPut("Returned:");
        printObject(value);
        outPut("\n");
        }
      if(value!=cell->object.value){
        if(cell->object.value!=NULL) dropObject(cell->object.value);
        cell->object.value=grabObject(value);
        nbCellPublish(cell);
        if(trace) outMsg(0,'T',"nbCellReact() published change.");
        }
      freeNode=treeNode;
      NB_TREE_ITERATE_NEXT2(treeIterator,treeNode)
      nbFree((NB_Object *)freeNode,sizeof(NB_TreeNode));
      }
    }
  evalVectorTop=evalVector;  /* set top level */ 
  }

//******************************
// External API

NB_Cell *nbCellCreate(nbCELL context,char *cellExpression){
  nbCELL cell;
  char *cursor=cellExpression;
  cell=grabObject(nbParseCell((NB_Term *)context,&cursor,0));
  if(*cursor!=0){
    outMsg(0,'E',"nbCellCreate() called with unrecognized syntax: %s",cellExpression);
    cell=dropObject(cell);
    }
  return(cell);
  }

NB_Cell *nbCellParse(nbCELL context,char **cellExpression){
  return(nbCellGrab(context,(nbCELL)nbParseCell((NB_Term *)context,cellExpression,0)));
  }

NB_Cell *nbCellGetValue(NB_Cell *context,NB_Cell *cell){
  return((NB_Cell *)(cell->object.value));
  }

int nbCellGetType(NB_Cell *context,NB_Cell *cell){
  return(cell->object.type->apicelltype);
  }
NB_Cell *nbCellCreateString(NB_Cell *context,char *string){
  return(nbCellGrab(context,(NB_Cell *)useString(string)));
  }

char *nbCellGetString(NB_Cell *context,NB_Cell *cell){
  if(cell==NULL || cell->object.type!=strType) return(NULL);
  return(((NB_String *)cell)->value);
  }

NB_Cell *nbCellCreateReal(NB_Cell *context,double real){
  return((NB_Cell *)useReal(real));
  }

double nbCellGetReal(NB_Cell *context,NB_Cell *cell){
  if(cell==NULL || cell->object.type!=realType) return(0);
  return(((NB_Real *)cell)->value);
  }

NB_Cell *nbCellGrab(NB_Cell *context,NB_Cell *cell){
  return(grabObject(cell));
  }
NB_Cell *nbCellDrop(NB_Cell *context,NB_Cell *cell){
  return(dropObject(cell));
  }

void nbCellRelease(NB_Cell *context,NB_Cell *cell){
  if(cell->object.refcnt==0) dropObject(cell);
  }

void nbCellPub(NB_Cell *context,NB_Cell *cell){
  nbCellPublish(cell);
  }

void nbCellShow(NB_Cell *context,NB_Cell *cell){
  NB_Term *addrContextSave=addrContext;
  addrContext=(NB_Term *)context;
  printObject(cell);
  addrContext=addrContextSave;
  }

NB_Cell *nbCellEvaluate(NB_Cell *context,NB_Cell *cell){
  return((NB_Cell *)cell->object.type->eval(cell));
  }

/* 2004/08/28 eat - compute method now expected to return grabbed cell */
NB_Cell *nbCellCompute(NB_Cell *context,NB_Cell *cell){
  return((NB_Cell *)cell->object.type->compute(cell));
  }

NB_Cell *nbCellSolve(NB_Cell *context,NB_Cell *cell){
  return((NB_Cell *)nbCellSolve_(cell));
  }

