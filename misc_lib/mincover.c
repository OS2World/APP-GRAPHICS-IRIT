/*****************************************************************************
* mincover.c - computes the minimal set cover of interval(s).		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by Robert Sayegh, November, 2004                                    *
*****************************************************************************/

#include "misc_loc.h"

#define LOCAL_ARRAY_SIZE 1000

#ifdef DEBUG
IRIT_SET_DEBUG_PARAMETER(_DebugMinConverRLC, FALSE);
#endif /* DEBUG */

IRIT_STATIC_DATA int
    MaxGaurdians = 5;/* Default number of views for the MultiRange solution.*/

typedef struct RLNode {
    IrtRType Right;
    IrtRType Left;
    int      Attr;
    struct RLNode *Pnext;
    struct RLNode *Pprev;
} RLNodeStruct;

typedef struct RangeList {
    int Size;
    int IsOrdered;
    int IsMultiRange;
    RLNodeStruct *Plist;
} RLStruct;

/* RL General List & Node functions. */
#define LIST_HEAD(l)  l -> Plist -> Pnext
#define LIST_TAIL(l)  l -> Plist

static RLNodeStruct *RLNewNode(IrtRType l, IrtRType r, int Attr);
static void RLDeleteNode(RLNodeStruct *node);
static RLStruct *RLNew(void);
static void RLDelete(RLStruct *List);
static void RLAdd(RLStruct *List, IrtRType l, IrtRType r, int Attr);

/* Single Range MinCover */
static void RLRemove(RLStruct *List, RLNodeStruct *p);
static void RLInsertBefore(RLStruct *List, RLNodeStruct *q, RLNodeStruct *p);
static void RLInsertSort(RLStruct *List);
static RLNodeStruct *FindRightmost(RLStruct *List,
				   RLNodeStruct *Curr,
				   IrtRType Tol);
static int *FindSRCyclicCover(RLStruct *List, IrtRType Tol);

/* Multi Range MinCover */
static int GenerateNextSubset(int Mask[], int S, int Size);
static int *FindMRCyclicCover(RLStruct *List, IrtRType Tol);

/* Static allocation of unification sets - reduce runtime allocations. */
typedef struct LocalAllocationArray {
    RLNodeStruct *Data;
    struct LocalAllocationArray *Pnext;
} LocalArrayStruct;

struct {
    int Size;			     /* Nu5mber of LoaclArrays held in List.*/
    int CurrIndex;	      /* Index of nex node to use from Ptail array. */
    LocalArrayStruct *Phead; 
    LocalArrayStruct *Ptail;  /* Latest Array in use (not last in the List).*/
} LocalNodesList;

static void RLResetLocalArrays(void); 
static void RLDeleteLocalArrays(void); 
static RLNodeStruct* RLNewLocalNode(void);

static void RLAddLocal(RLStruct *List, IrtRType l, IrtRType r);
static RLStruct *RLNewSetLocal(void);
static int RLUnifyWithRangeLocal(RLStruct *Set, 
				  IrtRType L, IrtRType R, 
				  IrtRType Tol);
static int UnifyMRIntervalsLocal(RLStruct *List, 
				  RLNodeStruct **Hash, 
				  int *Mask, int G,
				  IrtRType Tol);


#ifdef DEBUG
static void RLNodePrint(RLNodeStruct *node);
static void RLPrint(RLStruct *List);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets the maximum number of views allowed to generate a min cover.        M
*   Actually this is the maximum number of subsets to be generated           M
*   by the FindMRCyclicCover() function.                                     M
*                                                                            *
* PARAMETERS:                                                                M
*   g:   Maximum number of gaurdians required.                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: The previously used number of gaurdians.                            M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritRLAdd, IritRLFindCyclicCover, IritRLDelete                           M
*									     *
* KEYWORDS:                                                                  M
*    IritRLSetGaurdiansNumber                                                M
*****************************************************************************/
int IritRLSetGaurdiansNumber(int g)
{
    int PrevGaurdians = MaxGaurdians;

    MaxGaurdians = g;
    return PrevGaurdians;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initializes a new RLNodeStruct.                                          *
*                                                                            *
* PARAMETERS:                                                                *
*    l:       Left specifier of the range.                                   *
*    r:       Right specifier of the range.                                  *
*    Attr:    Attribute of the range (usually it's serial number in list).   *
*                                                                            *
* RETURN VALUE:                                                              *
*    RLNodeStruct *: returns a new allocated range node -                    *
*                    this function does not link the node to any list.       *
*****************************************************************************/
static RLNodeStruct *RLNewNode(IrtRType l, IrtRType r, int Attr)
{
    RLNodeStruct *Node;

    if ((l > r) || (l < 0.0 && r < 0.0) || (l > 1.0 && r > 1.0))
	return NULL;

    Node = (RLNodeStruct *) IritMalloc(sizeof(RLNodeStruct));

    /* Cyclic transformation of ranges */
    if (r > 1) {
	Node -> Left  = l - 1.0;
	Node -> Right = r - 1.0;
    }
    else {
	Node -> Left  = l;
	Node -> Right = r;
    }

    Node -> Attr  = Attr;

    Node -> Pprev = NULL;
    Node -> Pnext = NULL;

    return Node;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Cleanup of RLNodeStruct.                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*    node: A node to be deleted from the list                                *
*                                                                            *
* RETURN VALUE:                                                              *
*    void                                                                    *
*****************************************************************************/
static void RLDeleteNode(RLNodeStruct *node)
{
    IritFree(node);
    return;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Initializes a new Range List (RL) - which is an empty double link list    M
*  The list specifier is Plist*, which points, always, at the tail.          M
*                                                                            *
* PARAMETERS:                                                                M
*   None		                                                     M
*                                                                            *
* RETURN VALUE:                                                              M
*   VoidPtr:  The new allocated list structure.                              M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritRLAdd, IritRLFindCyclicCover, IritRLDelete                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRLNew                                                                M
*****************************************************************************/
VoidPtr IritRLNew(void)
{
    return (void *) RLNew();
}

static RLStruct *RLNew(void) 
{
    RLStruct
	*l = (RLStruct *) IritMalloc(sizeof(RLStruct));

    l -> Size = 0;
    l -> IsOrdered = FALSE;
    l -> IsMultiRange = 0;
    
    l -> Plist = NULL;
	
    return l;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Cleanup of RLStruct (a whole list)                                       M
*                                                                            *
* PARAMETERS:                                                                M
*   RLC: A list to be deleted.                                               M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritRLNew, IritRLAdd, IritRLFindCyclicCover                              M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRLDelete                                                             M
*****************************************************************************/
void IritRLDelete(VoidPtr RLC)
{
    RLDelete((RLStruct *) RLC);
}

static void RLDelete(RLStruct *List)
{
    RLNodeStruct *b, *e, *p;

    if (List -> Size == 0)
	return;

    b = LIST_HEAD(List);
    e = LIST_TAIL(List);

    List -> Plist = e -> Pprev;	       /* The loop will not cover this one. */
    for (p = b; p != e; p = p -> Pnext) 
	RLDeleteNode(p -> Pprev);
    RLDeleteNode(List -> Plist);

    IritFree(List);    
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Creates and links a new range node to an existing list of nodes.          M
*                                                                            *
* PARAMETERS:                                                                M
*   RLC:         An existing list which will be added a new node.            M
*   l, r, Attr:  Node details; Left, Right and Attribute.                    M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritRLNew, IritRLFindCyclicCover, IritRLDelete                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRLAdd                                                                M
*****************************************************************************/
void IritRLAdd(VoidPtr RLC, IrtRType l, IrtRType r, int Attr)
{
    RLAdd((RLStruct *) RLC, l, r, Attr);
}

static void RLAdd(RLStruct *List, IrtRType l, IrtRType r, int Attr)
{
    RLNodeStruct *Node;

    Node = RLNewNode(l, r, Attr);
    if (Node == NULL) 
	return;
	
    /* Init a cyclic double-link list. */
    if (List -> Size == 0) {
	Node -> Pprev = Node;
	Node -> Pnext = Node;
    }
    /* Add at the tail - best heuristic for our input. */
    else {
        RLNodeStruct * b,*e;
        b = LIST_HEAD(List);
	e = LIST_TAIL(List);
	
	/* If last node has same attribute, we have a MultiRange problem. */
	if (Node -> Attr == e -> Attr)
	    List -> IsMultiRange = 1;

	/* Notice that node (b) never changes in this function. */
	e -> Pnext = Node;
	b -> Pprev = Node;
	Node -> Pprev = e;
	Node -> Pnext = b;
    }
    List -> Plist = Node;
    List -> Size++;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Links a new range Node using pre-allocated Nodes in LocalNodesList.       *
*                                                                            *
* PARAMETERS:                                                                *
*    List:        A pointer to the 'shell' List handled by LocalNodesList.   *
*    l, r:	  Node details; Left, Right.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*    void                                                                    *
*****************************************************************************/
static void RLAddLocal(RLStruct *List, IrtRType l, IrtRType r)
{
    RLNodeStruct
        *Node = RLNewLocalNode();

    /* Init a cyclic double-link list. */
    if (List -> Size == 0) {
	Node -> Pprev = Node;
	Node -> Pnext = Node;
    }
    /* Add at the tail - best heuristic for our input. */
    else {
        RLNodeStruct *b, *e;
        b = LIST_HEAD(List);
	e = LIST_TAIL(List);
	
	/* Notice that node (b) never changes in this function. */
	e -> Pnext = Node;
	b -> Pprev = Node;
	Node -> Pprev = e;
	Node -> Pnext = b;
    }

    Node -> Left = l;
    Node -> Right = r;
    Node -> Attr = -1;

    List -> Plist = Node;
    List -> Size++;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Removes the linking pointers (only) of a node from an RL.                 *
*  (used for general operations on lists, e.g. sorting)                      *
*                                                                            *
* PARAMETERS:                                                                *
*    List: The list (RL) being manipulated.                                  *
*    p:    The node to be removed.                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*    void                                                                    *
*****************************************************************************/
static void RLRemove(RLStruct *List, RLNodeStruct *p)
{
    if (List -> Size <= 1) {
	List -> Plist = NULL;
	List -> Size = 0;
    }
    else {					/* Works even if size = 2. */
	RLNodeStruct *a, *b;
	a = p -> Pprev;
	b = p -> Pnext;

	a -> Pnext = b;
	b -> Pprev = a;

	if (List -> Plist == p)
	    List -> Plist = a;
	List -> Size--;
    }
    
    /* Just for the sake of a clean code. */
    p -> Pnext = NULL;
    p -> Pprev = NULL;

    return;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Re-links (an exisiting) node after another specified node in the RL       *
*  (used for general operations on lists, e.g. sorting)                      *
*                                                                            *
* PARAMETERS:                                                                *
*    List: The list being manipulated                                        *
*    q,p :    Locate (p) after (q)                                           *
*                                                                            *
* RETURN VALUE:                                                              *
*    void                                                                    *
*****************************************************************************/
static void RLInsertBefore(RLStruct *List, RLNodeStruct *q, RLNodeStruct *p)
{
    p -> Pprev = q -> Pprev;
    p -> Pnext = q;
    q -> Pprev -> Pnext = p;
    q -> Pprev = p;

    List -> Size++;

    return;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  Sorts an RL using InsertSort algorithm.                                   *
*  This specific algortithm has been chosen as it is expected that most      *
*  are already in their correct place (due to the scanning algorithm).       *
*                                                                            *
* PARAMETERS:                                                                *
*    List:  The list (RL) being sorted.                                      *
*                                                                            *
* RETURN VALUE:                                                              *
*    void                                                                    *
*****************************************************************************/
static void RLInsertSort(RLStruct *List)
{
    RLNodeStruct *b, *e, *p, *q, *next;
    int i, size;

    if (List -> Size <= 1)
	return;

    b = LIST_HEAD(List);
    e = LIST_TAIL(List);
    size = List -> Size;/* List -> Size may change due to redundant ranges. */

    if (List -> Size == 2) {
	if (b -> Left > e -> Left)
	    List -> Plist = b;
	return;
    }

    /* ELSE (Size >= 3). */
    p = b -> Pnext;
    for (i=1; i<size; i++) {
	next = p -> Pnext; /* P might move during sort, calc next node now! */

#	ifdef DEBUG
	IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
	    printf("====== sorting i=%d, (%d) ===================\n",
		   i, p -> Attr);
	    RLPrint(List);
	}
#	endif /* DEBUG */

	for (q=p -> Pprev; q!=e; q=q -> Pprev) {
	    /* First eliminate redundant ranges. */

	    /* If p is redundant, just remove and delete it. */
	    if ((p -> Left >= q -> Left) && (p -> Right <= q -> Right)) {
#		ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
		    printf("removing redundant node (p):"); 
		    RLNodePrint(p);
		}
#		endif /* DEBUG */
		RLRemove(List, p);
		RLDeleteNode(p);
		p = NULL;
		break;
	    }

	    /* Or else if one of the q's is redundant, still remove and     */
	    /* delete but don't break the loop, as many q's may be	    */
	    /* detected...						    */
	    else if ((p -> Left <= q -> Left) && (p -> Right >= q -> Right)) {
#		ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
		    printf("removing redundant node (q):"); 
		    RLNodePrint(q);
		}
#		endif /* DEBUG */
		RLRemove(List,q);
		RLDeleteNode(q);
		q = p; /* Restart q; this happens only with p's neighbors. */
	    }

	    /* Insert sort test. */
	    else if (q -> Left < p -> Left)
		break;
	    
	}

	if (p != NULL  &&  q != p -> Pprev) {
	    RLRemove(List,p);
	    RLInsertBefore(List,q -> Pnext, p);
	}

	p = next;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  This is the core function of finding a cover of the range [0,1]           *
*  Given a sorted list (RL) and an anchor node, this function finds the      *
*  rightmost node which intersects with the nachor.                          *
*                                                                            *
* PARAMETERS:                                                                *
*    List: The list (RL) being handled.                                      *
*    Curr: The last node found to be rightmost, in the previous iteration.   *
*    Tol:  Accuracy of merges of sets.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*    RLNodeStruct *: The righmost node which intersects with (Curr)          *
*****************************************************************************/
static RLNodeStruct *FindRightmost(RLStruct *List,
				   RLNodeStruct *Curr,
				   IrtRType Tol)
{
    RLNodeStruct *p, *Max;
    IrtRType Leftbound;

    if (List -> Size == 0)
	return NULL;

    Leftbound = Curr -> Right;
    Max = Curr;

    for (p = Curr -> Pnext; 
	 (p -> Left <= Leftbound + Tol) && (p != LIST_HEAD(List)); 
	 p = p -> Pnext) {
    	if (p -> Right >= Max -> Right)
	    Max = p;
    }

    if ((Max == Curr) || (Max -> Left > Leftbound + Tol))
	return NULL;
 
#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
	printf("** Max is :");
	RLNodePrint(Max);
    }
#   endif /* DEBUG */

    return Max;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function finds a cyclic cover for Single Range Intervals,           *
*   where each itnerval consists of one range [r,l] only.                    *
*                                                                            *
* PARAMETERS:                                                                *
*    List: List of single range intervals	                             *
*    Tol:  Accuracy of merges of sets.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*    RLStruct*: A vector of indices (attributes) of covering set, terminated *
*		by -1.							     *
*****************************************************************************/
static int *FindSRCyclicCover(RLStruct *List, IrtRType Tol)
{
    int i, Success, *RetVal;
    RLNodeStruct *Origin, *Curr, *Next;
    RLStruct *Cover, *MinCover;

    MinCover = (RLStruct *) IritRLNew();

    /* First of all, sort the RL. */
    if (!List -> IsOrdered)
	RLInsertSort(List);

    for (Origin = LIST_HEAD(List);
	 (Origin -> Left <= 0.0) && (Origin != LIST_TAIL(List));
	 Origin = Origin -> Pnext) {

#	ifdef DEBUG
	IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
	    printf("--- ->  Origin is :");
	    RLNodePrint(Origin);
	}
#	endif /* DEBUG */
     
	Cover = (RLStruct *)RLNew();
        RLAdd(Cover, Origin -> Left, Origin -> Right, Origin -> Attr);

	Curr = Origin;
	Success = TRUE;
	do {
	    Next = FindRightmost(List, Curr, Tol);
	    if (Next == NULL || Next == Curr) {
		Success = FALSE;
		break;
	    }

	    Curr = Next;
	    RLAdd(Cover, Curr -> Left, Curr -> Right, Curr -> Attr);
	}
	while (Curr -> Right < (1 + Origin -> Left)); /* Origin -> Left < 0. */


	if (Success && 
	    ((Cover -> Size < MinCover -> Size) || (MinCover -> Size == 0))) {
	    RLDelete(MinCover);
	    MinCover = Cover;
	}
	else {
	    RLDelete(Cover);
	}
    }

    RetVal = (int *)IritMalloc(sizeof(int) * (MinCover -> Size + 1));

    if (MinCover -> Size > 0) {
        for (i = 0, Curr = LIST_HEAD(MinCover);
	     i < MinCover -> Size;
	     i++, Curr = Curr -> Pnext)
	    RetVal[i] = Curr -> Attr;
    }
    else
        i = 0;

    RetVal[i] = -1;

    RLDelete(MinCover);

    return RetVal;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function creates a new 'shell' of a Range Se			     *
*     It need not allocate any nodes later, only use the already	     *
*   allocated ones in LocalNodesList structure.                              *
*                                                                            *
* PARAMETERS:                                                                *
*   None		                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   RLStruct *: A new RangeList with IsOrdered member set to 1.              *
*****************************************************************************/
static RLStruct *RLNewSetLocal(void) 
{
    RLStruct
	*l = (RLStruct *) IritMalloc(sizeof(RLStruct));

    RLResetLocalArrays();           /* Will use Nodes from LocalNodesList. */

    l -> Size = 0;
    l -> IsOrdered = 1;
    l -> IsMultiRange = 1;

    l -> Plist = NULL;

    return l;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Reset the LocalNodesList structure of pointers to arrays for a new run.  *
*   If this is the first Reset, then reset all pointers and Size.            *
*   Otherwise keep the data structure intact, and reset the CurrNode,        *
*   by using the first Node in Phead array, in order to reuse the            *
*   already-allocated arrays.                                                *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RLResetLocalArrays(void) 
{
    static int Flag = 0;

    if (Flag) {
	LocalNodesList.Ptail = LocalNodesList.Phead;
        LocalNodesList.CurrIndex = 0;
    }
    else {
	LocalNodesList.Phead = NULL;
	LocalNodesList.Ptail = NULL;
	LocalNodesList.Size = 0;
        LocalNodesList.CurrIndex = 0;

	Flag=1;
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Deletes all LocalArrays tht were allocated during the program run.       *
*   !! It is forbidden to access/use the LocalNodesList after calling        *
*      this function !!                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RLDeleteLocalArrays(void) 
{
    int i;
    LocalArrayStruct *p, *q;

    q = LocalNodesList.Phead;
    for (i = 0; i < LocalNodesList.Size; i++) {
	p = q -> Pnext;
	IritFree(q -> Data);
	IritFree(q);
	q = p;
    }

    LocalNodesList.Phead = NULL;
    LocalNodesList.Ptail = NULL;
    LocalNodesList.Size = 0;
    LocalNodesList.CurrIndex = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Keeps track of LocalNodesList and returns the currently                  *
* available RLNodeStruct from the preallocated arrays.                       *
*   If neccessary allocate a new array of Nodes.                             *
*   !! Actually this function simulates an array which me be expanded.       *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static RLNodeStruct *RLNewLocalNode(void)
{
    RLNodeStruct *RetNode;

    /* Allocate a new array, all previously allocated nodes are used... */
    if (LocalNodesList.Ptail == NULL || 
	LocalNodesList.CurrIndex >= LOCAL_ARRAY_SIZE) {

        /* If there exists a ready unused array then use it! */
	if (LocalNodesList.Ptail != NULL && 
	    LocalNodesList.Ptail -> Pnext != NULL)
	    LocalNodesList.Ptail = LocalNodesList.Ptail -> Pnext;

	/* Either this is the first call, or ran-out of allocated        */
	/* arrays --> allocate a new one!                        	 */
	else {
	    LocalArrayStruct * newArray;

	    /* Initialize a new Array. */
	    newArray = (LocalArrayStruct *)IritMalloc(sizeof(LocalArrayStruct));
	    newArray -> Data = (RLNodeStruct *) 
				IritMalloc(LOCAL_ARRAY_SIZE * sizeof(RLNodeStruct));
	    newArray -> Pnext = NULL;
    
	    /* Add the new array to the local arrays List. */
	    if (LocalNodesList.Size == 0) {
		LocalNodesList.Phead = newArray;
		LocalNodesList.Ptail = newArray;
		LocalNodesList.Size = 1;
	    } else {
    		LocalNodesList.Ptail -> Pnext = newArray;
		LocalNodesList.Ptail = newArray;
		LocalNodesList.Size ++;
	    }
	}

	/* Init the new array for usage... */
	LocalNodesList.CurrIndex = 0;
    }

    RetNode = &(LocalNodesList.Ptail -> Data[ LocalNodesList.CurrIndex ]);
    LocalNodesList.CurrIndex ++;

    return RetNode;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Given a subset Mask from the set {0...Size} generate the next subset.    *
*   subsets are egnerated lexicographically, such that all subsets of size k *
*   will be generated before subsets k+1.                                    *
*                                                                            *
* PARAMETERS:                                                                *
*    Mask[]: The last generated subset                                       *
*    S:      The current size of Mask                                        *
*    Size:   The size of the set for which subsets are generated.            *
*                                                                            *
* RETURN VALUE:                                                              *
*    int:    The size of the new generated Mask.                             *
*****************************************************************************/
static int GenerateNextSubset(int Mask[], int S, int Size) 
{
    int Pos;

    for (Pos = S - 1; Pos >= 0; Pos--)
        if (Mask[Pos] < (Size - (S - 1 - Pos))) {
	    Mask[Pos]++;
	    break;
	}

    if (Pos < 0) {
        Pos = 0;
	Mask[Pos] = 1;
	S++;
    }

    for (Pos++; Pos < S; Pos++)
        Mask[Pos] = Mask[Pos - 1] + 1;


#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
        int Q;
	printf("NEW MASK : ");
	for (Q = 0; Q < S; Q++) 
	    printf("%d ",Mask[Q]);
	printf("\n");
    }
#   endif /* DEBUG */

    return S;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function unifyies a list of ranges with a new range as does         *
*   RLUnifyWithRange. However, this function uses the Nodes maintained in    *
*   the LocalNodeList structure.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*    List: An ascending-ordered list of range nodes.                         *
*    L: Left value of the range.                                             *
*    R: Right value of the range.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*    int:  1 or 0, specifying whether the list contains the range [0,1].     *
*****************************************************************************/
static int RLUnifyWithRangeLocal(RLStruct *Set, 
				 IrtRType L,
				 IrtRType R, 
				 IrtRType Tol)
{
    int i;
    IrtRType lastR;
    RLNodeStruct *l, *r, *n;

    /* If the list is still empty. */
    if (Set -> Size == 0) {
	RLAddLocal(Set, L, R);
	if ((L - Tol) <= 0.0 && (R + Tol) >= 1.0)
	    return 1;
	return FALSE;
    }

    /* First anchor L within the ranges of Set, or just add new node [L,R]. */
    l = LIST_HEAD(Set);
    for (i = 0; i < Set -> Size; i++, l = l -> Pnext)
	if (L <= l -> Right)	
	    break;

    if (i == Set -> Size) { /* Reached the end of list without intersection. */
	RLAddLocal(Set, L, R);
	return FALSE;
    }

    if ((R+Tol) < l -> Left) {
	RLAddLocal(Set, L, R);
        
	/* Node should not necessarily remain at the tail. */
        n = LIST_TAIL(Set);
        RLRemove(Set, n);
	RLInsertBefore(Set, l, n);
	return FALSE;
    }

    l -> Left = IRIT_MIN(l -> Left, L);

    
    /* Second anchor R ... remove all redundant ranges in between. */
    lastR = l -> Right;
    r = l -> Pnext;
    while (r != LIST_HEAD(Set)) { 
	if ((R + Tol) < r -> Left)
	    break;

	n = r -> Pnext;
	lastR = r -> Right;
	RLRemove(Set,r);
	/* IritFree(r); - It's all in static memory! */  
	r = n;
    } 

    l -> Right = IRIT_MAX(lastR, R);

    /* Now check whether the new Set covers the range [0,1]. */
    if (Set->Size == 1) {
	if ((Set -> Plist -> Left - Tol) <= 0.0 &&
	    (Set -> Plist -> Right + Tol) >= 1.0)
	    return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*  This function finds a cyclic cover for Multil Range Intervals,            *
*  where each itnerval consists of many ranges [r,l] with the same Attribute *
*                                                                            *
*  !! Currently We assume that the Attributes start from 1 and are given     *
*    in a serial mode (all 1's then all 2's - without any gaps ...           *
*                                                                            *
* PARAMETERS:                                                                *
*    List: List of single range intervals	                             *
*    Tol:  Accuracy of merges of sets.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*    RLStruct*: A vector of indices (attributes) of covering set, terminated *
*		by -1.							     *
*****************************************************************************/
static int *FindMRCyclicCover(RLStruct *List, IrtRType Tol)
{
    int i, Ret;
    int SetSize;	      /* The number of different views (attributes). */
    int G;			        /* Current number of gaurdians used. */
    int MaxG;	      /* calculate the maximum number of gaurdians possible. */
    int *Mask;					    /* Subset of Attributes. */
    RLNodeStruct **Hash;	 /* Quick links to Nodes with specific Attr. */
    RLNodeStruct *p;

    /* Init a hash of all attributes in List. */
    SetSize = List -> Plist -> Attr;		    /* Attr's starts from 1. */
    Hash = (RLNodeStruct **) IritMalloc(sizeof(RLNodeStruct *)
					         * (SetSize + 1)); /* +zero. */
    
    Hash[0] = NULL; /* Just in case. */
    p = LIST_HEAD(List);
    Hash[p -> Attr] = p;
    for (i = 1; i < List -> Size; i++) {
	p = p -> Pnext;
	if (p -> Attr != p -> Pprev -> Attr)
	    Hash[p -> Attr] = p;
    }

    /* Initialize subset enumerating. */
    MaxG = IRIT_MIN(MaxGaurdians, SetSize);	     /* Maximum SubSet size. */
    Mask = (int *) IritMalloc(sizeof(int) * (MaxG+1)); /* A (-1) for return. */

    G = 1;
    Mask[0] = SetSize;		       /* Will start from subsets of size 2. */
    for (i = 1; i < MaxG; i++)
	Mask[i] = -1;
    
    G = GenerateNextSubset(Mask, G, SetSize);

    while (G <= MaxG) {
	Ret = UnifyMRIntervalsLocal(List, Hash, Mask, G, Tol);

	if (Ret) {
	    Mask[G] = -1;

#	    ifdef DEBUG
	    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
	        int Q;
		printf("\n\nSOLUTION:\n");
		for (Q = 0; Q <= MaxG; Q++)  
		    printf("%4d ", Mask[Q]);
		printf("\n");
	    }
#	    endif /* DEBUG */

	    return Mask;
	}

	G = GenerateNextSubset(Mask, G, SetSize);
    }

    IritFree(Hash);

    Mask[0] = -1;

#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
        int Q;
	printf("\n\nSOLUTION:\n");
	for (Q=0; Q<=MaxG; Q++)  
	    printf("%4d ",Mask[Q]);
	printf("\n");
    }
#   endif /* DEBUG */

    return Mask; /* No solution. */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function behaves like UnifyMRIntervals, but calls the relevant      *
*   functions that deal with the Nodes maintained in LocalNodesList.         *
*                                                                            *
* PARAMETERS:                                                                *
*    List: A List of ranges, multi ranges (same Attr) are expected.          *
*    Hash: Pointers to the beginning of each unique Attribute.               *
*          We assume that all ranges with the same Attr are consecutive      *
*    Mask: A mask of the Hash (relevant subset of ranges) to be unified.     *
*    G:    The size of the mask (number of gaurdians).                       *
*                                                                            *
* RETURN VALUE:                                                              *
*    int: 0 or 1. Failure or Success.                                        *
*****************************************************************************/
static int UnifyMRIntervalsLocal(RLStruct *List, 
				 RLNodeStruct **Hash, 
				 int *Mask,
				 int G,
				 IrtRType Tol)
{
    int i,
        Ret = -1;
    RLNodeStruct *p;
    RLStruct
	*Set = RLNewSetLocal();

    /* For every attribute in Mask. */
    for (i = 0; i < G; i++) { 
        int index = Mask[i];

	/* For every range with this attribute. */
	for (p = Hash[index];
	     p -> Attr == Hash[index] -> Attr;
	     p = p -> Pnext) {
            if (p -> Left <= 0.0) {
	        Ret = RLUnifyWithRangeLocal(Set, 0.0, p -> Right, Tol);
		Ret = RLUnifyWithRangeLocal(Set, 1 + p -> Left, 1.0, Tol);
	    }
	    else if (p -> Right >= 1.0) {
	        Ret = RLUnifyWithRangeLocal(Set, 0.0, p -> Right - 1.0, Tol);
		Ret = RLUnifyWithRangeLocal(Set, p -> Left, 1.0, Tol);
	    }
	    else {
	        Ret = RLUnifyWithRangeLocal(Set, p -> Left, p -> Right, Tol);
	    }

	    if (Ret)
	        break;
	}
    }


#   ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugMinConverRLC) {
        printf("Unification of mask is:\n");
	RLPrint(Set);
    }
#   endif /* DEBUG */

    RLDeleteLocalArrays();
    IritFree(Set); /* This was only a 'shell' dealing with a static memory. */

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*  Finds a cyclic cover from nodes in RL to cover [0,1]                      M
*  This function decides the type of the cover SignleRange or MultiRange     M
* and calls the suitable internal function to find a cover.                  M 
*                                                                            *
* PARAMETERS:                                                                M
*   RLC:  A list (RL) of candidate ranges to cover [0,1].                    M
*   Tol:  Accuracy of merges of sets.					     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int *:    A vector of indices (attributes) of covering set, terminated   M
*	      by -1.							     M
*                                                                            *
* SEE ALSO:                                                                  M
*   IritRLNew, IritRLAdd, IritRLDelete                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   IritRLFindCyclicCover                                                    M
*****************************************************************************/
int *IritRLFindCyclicCover(VoidPtr RLC, IrtRType Tol)
{
    RLStruct
	*List = (RLStruct *) RLC;

    if (List -> IsMultiRange)
	return FindMRCyclicCover(List, Tol);
    else 
	return FindSRCyclicCover(List, Tol);
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints an RL List.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   List: List to print.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RLPrint(RLStruct *List)
{
    RLNodeStruct *b, *e, *p;

    printf("Printing list : size = %d\n", List -> Size);
    if (List -> Size == 0)
	return;
    b = LIST_HEAD(List);
    e = LIST_TAIL(List);

    for (p = b; p != e; p = p -> Pnext)  
	RLNodePrint(p);

    RLNodePrint(p);

    return;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints on RL node.                                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Node:   Node to print.                                                   *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void RLNodePrint(RLNodeStruct *Node)
{
    printf(" [%d]   l=%12f  r=%12f \n",
	   Node -> Attr, Node -> Left, Node -> Right);
}

#endif /* DEBUG */

#ifdef DEBUG_MAIN_MIN_COVER

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Initializes an new RL using a file to describe the ranges.               M
*                                                                            *
* PARAMETERS:                                                                M
*   FileName:   The file name containing the required details                M
*                                                                            *
* RETURN VALUE:                                                              M
*   RLStruct *: A new constrcuted RL                                         M
*                                                                            *
* SEE ALSO                                                                   M
*   RLNew                                                                    M
*                                                                            M
* KEYWORDS:                                                                  M
*   RLNewFromFile, RLNew                                                     M
*****************************************************************************/
RLStruct *RLNewFromFile(const char *FileName)
{
    double l, r;/* Using fscanf with %lf specifier, the type must be double. */
    FILE *f;
    RLStruct *List;

    List = RLNew();

    f = fopen(FileName, "r");
    if (f == NULL)
	return List;

    while (!feof(f)) {
	fscanf(f,"%lf%lf", &l, &r);
	RLAdd(List, l, r, List -> Size);
    }

    fclose(f);
    return List;
}

int main()
{

    int i;
    RLStruct
        *L = RLNewSetLocal();

    for (i = 0; i < 21; i++) {
	RLAddLocal(L, i / 100.0, 1.0);
    }
    RLPrint(L);
    printf("============>Size = %d\n", LocalNodesList.Size);

    {
        RLStruct
	    *L1=RLNewSetLocal();

	for (i = 0; i < 15; i++) {
	    RLAddLocal(L1, (i + 31) / 100.0, 1.0);
	}
	RLPrint(L1);
	printf("============>Size = %d\n", LocalNodesList.Size);
    }

    {
        LocalArrayStruct
	    *a = LocalNodesList.Phead;
	int i, j;

	printf("============>Size = %d\n", LocalNodesList.Size);
	for (i = 0; i < LocalNodesList.Size; i++) {
	    for (j = 0; j < LOCAL_ARRAY_SIZE; j++)
	        printf("Array%d[%02d] = %f\n", i, j, a->Data[j].Left);
	    a = a -> Pnext;
	}
    }
}

#endif /* DEBUG_MAIN_MIN_COVER */


#ifdef MISC_MVAR_COVER_OLD_UNIFYMR_INT

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function creates a new Range List, which is supposed to behave like *
*   a set; especially to support the RLUnifyWithNode() function.             *
*   The size of this list is determined according to the ranges needed       *
*   to express all the unifications performed.                               *
*                                                                            *
*   This is used mainly to decide whether a unification of intervals,        *
*   (multi range) is a cover, that is contains the range [0,1].              *
*                                                                            *
* PARAMETERS:                                                                *
*   None		                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   RLStruct *: A new RangeList with IsOrdered member set to 1.              *
*****************************************************************************/
static RLStruct *RLNewSet(void)
{
    RLStruct
	*l = (RLStruct *) IritMalloc(sizeof(RLStruct));

    l -> Size = 0;
    l -> IsOrdered = 1;
    l -> IsMultiRange = 1;
    
    l -> Plist = NULL;
	
    return l;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function receievs a List and a Mask of Atrributes of ranges         *
*   and creates a Set which is the unifications of all relevant ranges.      *
*                                                                            *
* PARAMETERS:                                                                *
*    List: A List of ranges, multi ranges (same Attr) are expected.          *
*    Hash: Pointers to the beginning of each unique Attribute.               *
*          We assume that all ranges with the same Attr are consecutive      *
*    Mask: A mask of the Hash (relevant subset of ranges) to be unified.     *
*    G:    The size of the mask (number of gaurdians).                       *
*                                                                            *
* RETURN VALUE:                                                              *
*    int: 0 or 1. Failure or Success.                                        *
*****************************************************************************/
static int UnifyMRIntervals(RLStruct *List, 
			    RLNodeStruct **Hash, 
			    int *Mask,
			    int G,
			    IrtRType Tol)
{
    int i, Ret;
    RLNodeStruct *p;
    RLStruct
	*Set = RLNewSet();

    /* For every attribute in Mask. */
    for (i = 0; i < G; i++) { 
        int index = Mask[i];

	/* For every range with this attribute. */
	for (p = Hash[index];
	     p -> Attr == Hash[index] -> Attr;
	     p = p -> Pnext) {
            if (p -> Left <= 0.0) {
	        Ret = RLUnifyWithRange(Set, 0.0, p->Right, Tol);
		Ret = RLUnifyWithRange(Set, 1 + p -> Left, 1.0, Tol);
	    }
	    else if (p -> Right >= 1.0) {
	        Ret = RLUnifyWithRange(Set, 0.0, p -> Right - 1.0, Tol);
		Ret = RLUnifyWithRange(Set, p -> Left, 1.0, Tol);
	    }
	    else {
	        Ret = RLUnifyWithRange(Set, p -> Left, p -> Right, Tol);
	    }

	    if (Ret)
	        break;
	}
    }

    RLDelete(Set); 

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This function unifyies a list of ranges with a new range.                *
*   if the nwe range may be unified with existing ranges a suitable new      *
*   range will replace the unified ranges; otherwise the range is merely     *
*   inserted into the list in its proper place.                              *
*                                                                            *
*  !!This function has no knowledge about cyclic ranges, it merely tries     *
*    to cover the range [0,1], so the calling function should consider this. *
*                                                                            *
* PARAMETERS:                                                                *
*    List: An ascending-ordered list of range nodes.                         *
*    L: Left value of the range.                                             *
*    R: Right value of the range.                                            *
*                                                                            *
* RETURN VALUE:                                                              *
*    int:  1 or 0, specifying whether the list contains the range [0,1].     *
*****************************************************************************/
static int RLUnifyWithRange(RLStruct *Set, 
			    IrtRType L, IrtRType R, 
			    IrtRType Tol)
{
    int i;
    IrtRType lastR;
    RLNodeStruct *l, *r, *n;

    /* If the list is still empty. */
    if (Set -> Size == 0) {
	RLAdd(Set, L, R, -1);
	if ((L - Tol) <= 0.0 && (R + Tol) >= 1.0)
	    return 1;
	return FALSE;
    }

    /* First anchor L within the ranges of Set, or just add new node [L,R]. */
    l = LIST_HEAD(Set);
    for (i = 0; i < Set -> Size; i++, l = l -> Pnext)
	if (L <= l -> Right)	
	    break;

    if (i == Set -> Size) { /* Reached the end of list without intersection. */
	RLAdd(Set, L, R, -1);
	return FALSE;
    }

    if ((R+Tol) < l -> Left) {
	RLAdd(Set, L, R, -1);
        
	/* Node should not necessarily remain at the tail. */
        n = LIST_TAIL(Set);
        RLRemove(Set, n);
	RLInsertBefore(Set, l, n);
	return FALSE;
    }

    l -> Left = IRIT_MIN(l -> Left, L);

    
    /* Second anchor R ... remove all redundant ranges in between. */
    lastR = l -> Right;
    r = l -> Pnext;
    while (r != LIST_HEAD(Set)) { 
	if ((R + Tol) < r -> Left)
	    break;

	n = r -> Pnext;
	lastR = r -> Right;
	RLRemove(Set,r);
	IritFree(r);
	r = n;
    } 

    l -> Right = IRIT_MAX(lastR, R);

    /* Now check whether the new Set covers the range [0,1]. */
    if (Set->Size == 1) {
	if ((Set -> Plist -> Left - Tol) <= 0.0 &&
	    (Set -> Plist -> Right + Tol) >= 1.0)
	    return TRUE;
    }
    return FALSE;
}

#endif /* MISC_MVAR_COVER_OLD_UNIFYMR_INT */
