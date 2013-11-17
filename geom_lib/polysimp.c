/******************************************************************************
* PolySimp.c - An implementation of multiresolution polygon decimation alg.   *
*******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                 *
*******************************************************************************
* Written by:  Uri Danan and Yankel Pariente           Ver 1.0, Jan. 2000     *
******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "allocate.h"   
#include "iritprsr.h"
#include "allocate.h"
#include "geom_loc.h"   

#define HDS_BOUNDARY		0
#define HDS_ACTIVE		1
#define HDS_INACTIVE		2
#define HDS_THRESHOLD_MODE	0
#define HDS_TRI_BUDGET_MODE	1
#define HDS_TBQ_INIT_MODE	2
#define HDS_BBOX_GAP		1

typedef struct HDSTribaseStruct {
    struct HDSTribaseStruct *Pnext, *Pprev;
    IrtPtType Vert[3];
    IrtNrmlType Norm[3];
    struct HDSNodeStruct *Corners[3];
    struct HDSNodeStruct *Proxies[3];
    IrtRType Id;
} HDSTribaseStruct;

typedef struct HDSTribaseListStruct {
    HDSTribaseStruct *Head;
    HDSTribaseStruct *Tail;
    IrtRType Count;
} HDSTribaseListStruct;

typedef struct HDSTriangleStruct {
    struct HDSTriangleStruct *Pnext, *Pprev;
    HDSTribaseStruct* PBase;
    IrtRType Id;
} HDSTriangleStruct;

typedef struct HDSTriListStruct {
    HDSTriangleStruct *Head, *Tail;
    IrtRType Count;
} HDSTriListStruct;

typedef struct HDSNodeStruct {
    struct HDSNodeStruct *Child0, *Child1, *Child2, *Child3,
		         *Child4, *Child5, *Child6, *Child7, *Parent;
    int Status, WeightRep, Depth;
    IrtPtType RepVert;
    IrtNrmlType RepNorm;
    IrtRType Xmin, Xmax, Ymin, Ymax, Zmin, Zmax, Radius;
    HDSTriListStruct *Tris, *SubTris;
} HDSNodeStruct;

typedef struct HDSTBNodeStruct {
    struct HDSTBNodeStruct *Pnext, *Pprev;
    struct HDSNodeStruct *Node;
} HDSTBNodeStruct;

typedef struct TBQueueStruct {
    struct HDSTBNodeStruct *Head;
    struct HDSTBNodeStruct *Tail;
    struct HDSTBNodeStruct *Current;
    IrtRType Count;
    IrtRType NumActive;
} TBQueueStruct;

typedef struct HDSOctreeStruct {
    HDSNodeStruct *Root;
    int MaxDepth;
    HDSTriListStruct *ActiveList;
    HDSTribaseListStruct *TriangleList;
    struct TBQueueStruct *Tbq;
} HDSOctreeStruct;

static HDSTriangleStruct *HDSCreateTri(struct HDSTribaseStruct *PBase);
static HDSTriListStruct *HDSCreateTriList(void);
static void HDSFreeList(HDSTriListStruct* List);
static void HDSEnqueue(HDSTriangleStruct* Node, HDSTriListStruct **PList);
static void HDSTriInsert(HDSTriangleStruct* Node, HDSTriListStruct **PList);
static void HDSTriRemove(HDSTriangleStruct* Node, HDSTriListStruct *List);
static HDSTriangleStruct *HDSFindTri(HDSTriangleStruct *Node,
				     HDSTriListStruct *List);
static void HDSCollapseNode(HDSNodeStruct* n, HDSOctreeStruct* Qt);
static void HDSExpandNode(HDSNodeStruct *n,
			  HDSOctreeStruct *Qt,
			  IrtRType Limit,
			  int Mode);
static void HDSAdjustTree(HDSNodeStruct *n,
			  HDSOctreeStruct *Qt,
			  IrtRType Threshold);
static HDSNodeStruct *HDSCreateNode(void);
static HDSNodeStruct *HDSGetChild(int i, HDSNodeStruct *n);
static HDSOctreeStruct* HDSCreateOctree(int MaxDepth, 
					IrtRType Xmin, 
					IrtRType Xmax, 
					IrtRType Ymin, 
					IrtRType Ymax, 
					IrtRType Zmin, 
					IrtRType Zmax);
static void HDSOctreeRemove(HDSNodeStruct* n, HDSOctreeStruct * Qt);
static HDSNodeStruct* HDSOctreeInsertVertex(int Id,
					    IrtRType x,
					    IrtRType y,
					    IrtRType z,
					    IrtNrmlType norm,
					    HDSTribaseStruct* Tri,
					    HDSNodeStruct* n,
					    HDSOctreeStruct *Qt);
static HDSNodeStruct* HDSOctreeUpdateNodes(int Id,
					    IrtRType x,
					    IrtRType y,
					    IrtRType z,
					    HDSTribaseStruct* Tri,
					    HDSNodeStruct* n,
					    HDSOctreeStruct *Qt);
static void HDSOctreeCalculateRadius(IrtRType x,
				     IrtRType y,
				     IrtRType z,
				     HDSNodeStruct *n,
				     HDSOctreeStruct *Qt);
static HDSOctreeStruct *HDSInitOctree(HDSTribaseListStruct *TriangleList,
				      IrtRType Xmin,
				      IrtRType Xmax,
				      IrtRType Ymin,
				      IrtRType Ymax,
				      IrtRType Zmin,
				      IrtRType Zmax,
				      int Depth);
static void HDSSetChild(HDSNodeStruct *n, int i, HDSNodeStruct *Value);
static int HDSPointInNode(IrtPtType Pt, HDSNodeStruct *n);
static IrtRType HDSNodeSize(HDSNodeStruct *n);
static IPObjectStruct *HDSConVertActiveList2PObj(HDSTriListStruct *AList);
static TBQueueStruct* HDSCreateTBQ(void);
static HDSTBNodeStruct* HDSCreateTBNode(void);
static void HDSInsertToTBQ(HDSTBNodeStruct *n, TBQueueStruct *q);
static void HDSFreeTBQ(TBQueueStruct *q);
static void HDSInitTBQ(HDSOctreeStruct *Qt, HDSNodeStruct *n);
static void HDSTBQInitActiveList(HDSNodeStruct *n, HDSOctreeStruct *Qt);
static HDSTribaseStruct *HDSCreateTribase(IrtRType x1,
					  IrtRType y1,
					  IrtRType z1,
					  IrtRType x2,
					  IrtRType y2,
					  IrtRType z2,
					  IrtRType x3,
					  IrtRType y3,
					  IrtRType z3,
					  IrtNrmlType n1,
					  IrtNrmlType n2,
					  IrtNrmlType n3);
static HDSTribaseListStruct *HDSCreateTribaseList(void);
static void HDSFreeListbase(HDSTribaseListStruct *List);
static void HDSTribaseInsert(HDSTribaseStruct *Node,
			     HDSTribaseListStruct **PList);
static void HDSTribaseRemove(HDSTribaseStruct *Node,
			     HDSTribaseListStruct *List);
static HDSTribaseStruct *HDSFindTribase(HDSTribaseStruct *Node,
					HDSTribaseListStruct *List);
static int HDSGetChildIndex(HDSNodeStruct *n,double x,double y,double z);
static HDSNodeStruct* HDSCreateChild(int index, HDSNodeStruct* n);
static void HDSSetChildAttributes(HDSNodeStruct* n,int index);
static int HDSIsLeaf(HDSNodeStruct *n);
int HDSGetActiveListCount(VoidPtr Qt);
int HDSGetTriangleListCount(VoidPtr Qt);
int HDSGetDismissedTrianglesCount(VoidPtr Qt);

/*****************************************************************************
* DESCRIPTION:							             *
*    Returns the number of triangles in the current ActiveList.		     *
*									     *
* PARAMETERS:								     *
*    Qt:    A pointer to the Vertex Tree.				     *
*									     *
* RETURN VALUE:							             *
*    int: The number of triangles.					     *
*****************************************************************************/
int HDSGetActiveListCount(VoidPtr Qt)
{
	return (int)((HDSOctreeStruct *)Qt)->ActiveList->Count;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Returns the number of triangles in the original object.		     *
*									     *
* PARAMETERS:								     *
*    Qt:    A pointer to the Vertex Tree.				     *
*									     *
* RETURN VALUE:							             *
*    int: The number of triangles.					     *
*****************************************************************************/
int HDSGetTriangleListCount(VoidPtr Qt)
{
	return (int)((HDSOctreeStruct *)Qt)->TriangleList->Count;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Returns the number of triangles from original object dismissed in the   *
*    Vertex Tree.							     *
*									     *
* PARAMETERS:								     *
*    Qt:    A pointer to the Vertex Tree.				     *
*									     *
* RETURN VALUE:							             *
*    int: The number of triangles.					     *
*****************************************************************************/
int HDSGetDismissedTrianglesCount(VoidPtr Qt)
{
	HDSThreshold(Qt,0);
	return (int)(((HDSOctreeStruct *)Qt)->TriangleList->Count - 
		((HDSOctreeStruct *)Qt)->ActiveList->Count);
}


/*****************************************************************************
* DESCRIPTION:							             *
*    Returns true if the node is a leaf in the vertex tree		     *
*    (all children NULL), false otherwise.				     *
*									     *
* PARAMETERS:								     *
*    n:    A pointer to a node inside the vertex tree.			     *
*									     *
* RETURN VALUE:							             *
*    int: A boolean value.						     *
*****************************************************************************/
static int HDSIsLeaf(HDSNodeStruct *n)
{
	if(n->Child0==NULL && n->Child1==NULL && n->Child2==NULL &&
		n->Child3==NULL && n->Child4==NULL && n->Child5==NULL &&
		n->Child6==NULL && n->Child7==NULL) return TRUE;
	else return FALSE;
}




/*****************************************************************************
* DESCRIPTION:							             *
*    Creates an element for the active, Tris or SubTris lists.		     *
*									     *
* PARAMETERS:								     *
*    PBase: A pointer to the original triangle from the global triangle list. *
*									     *
* RETURN VALUE:							             *
*    HDSTribaseStruct*: A pointer to the new triangle.			     *
*****************************************************************************/
static HDSTriangleStruct *HDSCreateTri(struct HDSTribaseStruct *PBase)
{
    HDSTriangleStruct
	*Node = IritMalloc(sizeof(HDSTriangleStruct));

    Node -> Pnext = NULL;
    Node -> Pprev = NULL;
    Node -> PBase = PBase;

    return Node;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Creates a triangle list (active list, Tris or SubTris).		     *
*									     *
* PARAMETERS:								     *
*    None								     *
*									     *
* RETURN VALUE:							             *
*    HDSTribaseListStruct*: A pointer to the new list.			     *
*****************************************************************************/
static HDSTriListStruct *HDSCreateTriList(void)
{
    HDSTriListStruct
	*List = IritMalloc(sizeof(HDSTriListStruct));

    List -> Head = NULL;
    List -> Tail = NULL;
    List -> Count = 0;

    return List;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Free the memory allocated to the List.				     *
*									     *
* PARAMETERS:								     *
*    List:  The List to be freed.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSFreeList(HDSTriListStruct *List)
{
    HDSTriangleStruct *HeadNext;

    if (List == NULL)
	return;
    
    while (List -> Head != NULL ) {
	HeadNext = List -> Head -> Pnext;
	HDSTriRemove(List -> Head,List);
	List -> Head = HeadNext;
	if (HeadNext != NULL)
	    HeadNext = HeadNext -> Pnext;
    }
    IritFree(List);
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Insert a triangle at the Tail of the List.				     *
*									     *
* PARAMETERS:								     *
*    Node:  The new triangle.						     *
*    PList: A pointer to the triangle List.				     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSEnqueue(HDSTriangleStruct* Node, HDSTriListStruct **PList)
{
    if ((*PList) == NULL)
	(*PList) = HDSCreateTriList();
    
    /* If the *PList is empty. */
    if ((*PList) -> Head == NULL) {
	(*PList) -> Head = (*PList) -> Tail = Node;

	(*PList) -> Count++;
    }
    else {
	if ((*PList) -> Tail -> PBase != Node -> PBase) {
	    (*PList) -> Tail -> Pnext = Node;
	    Node -> Pprev = (*PList) -> Tail;
	    (*PList) -> Tail = Node;

	    (*PList) -> Count++;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Insert a triangle in the List without repetitions.			     *
*									     *
* PARAMETERS:								     *
*    Node:   The new triangle						     *
*    PList:  A pointer to the triangle List				     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSTriInsert(HDSTriangleStruct *Node, HDSTriListStruct **PList)
{
    HDSTriangleStruct *Found;

    if (Node == NULL)
	return;

    if ((*PList) == NULL)
	(*PList) = HDSCreateTriList();
    
    /* If the *PList is empty. */
    if ((*PList) -> Head == NULL) {
	(*PList) -> Head = (*PList) -> Tail = Node;
	(*PList) -> Count++;
    }
    else {
	if ((Found = HDSFindTri(Node,*PList)) != NULL ) {
	    if (Found -> PBase != Node -> PBase) {
		if (Node -> Id > Found -> Id) {
		    Node -> Pnext = Found -> Pnext;
		    if (Found -> Pnext != NULL)
		        Found -> Pnext -> Pprev = Node;
		    Node -> Pprev = Found;
		    Found -> Pnext = Node;
		    if (Found == (*PList) -> Tail)
		        (*PList) -> Tail = Node;
		}
		else {
		    Node -> Pprev = Found -> Pprev;
		    if (Found -> Pprev != NULL)
		        Found -> Pprev -> Pnext = Node;
		    Node -> Pnext = Found;
		    Found -> Pprev = Node;
		    if (Found == (*PList) -> Head)
		        (*PList) -> Head = Node;
		}

		(*PList) -> Count++;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Remove a triangle from the List.					     *
*									     *
* PARAMETERS:								     *
*    Node:   The triangle to be removed.				     *
*    List:   The triangle List.						     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSTriRemove(HDSTriangleStruct *Node, HDSTriListStruct *List)
{
    HDSTriangleStruct*Tmp;

    if (Node == NULL || List == NULL)
        return;

    if ((Tmp = HDSFindTri(Node,List)) && Tmp -> PBase == Node -> PBase) {
	if (Tmp -> Pprev == NULL && Tmp -> Pnext == NULL) {
	    IritFree(Tmp);
	    List -> Head = List -> Tail = NULL;
	}

	/* If Tmp is Head. */
	else if (Tmp -> Pprev == NULL) {
	    List -> Head = List -> Head -> Pnext;
	    List -> Head -> Pprev = NULL;
	    IritFree(Tmp);
	}

	/* If Tmp is Tail. */
	else if (Tmp -> Pnext == NULL) {
	    List -> Tail = Tmp -> Pprev;
	    List -> Tail -> Pnext = NULL;
	    IritFree(Tmp);
	}

	/* Any other case. */
	else {
	    Tmp -> Pprev -> Pnext = Tmp -> Pnext;
	    Tmp -> Pnext -> Pprev = Tmp -> Pprev;
	    IritFree(Tmp);
	}

	List -> Count--;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Find a triangle in the List.					     *
*									     *
* PARAMETERS:								     *
*    Node:   The triangle to be Found.				             *
*    List:   The List.						             *
*									     *
* RETURN VALUE:							             *
*    HDSTribaseStruct *: A pointer to the triangle if Found, or else NULL.   *
*****************************************************************************/
static HDSTriangleStruct *HDSFindTri(HDSTriangleStruct *Node,
				     HDSTriListStruct *List)
{
    HDSTriangleStruct *Tmp;

    if (Node == NULL || List == NULL)
	return NULL;
    Tmp = List -> Head;
    
    while (Tmp != NULL) {
	if (Tmp -> Id > Node -> Id || Tmp -> PBase == Node -> PBase)
	    return Tmp;
	else
	    Tmp = Tmp -> Pnext;
    }
    return List -> Tail;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Collapse a Node in the Vertex tree according to the HDS algorithm.      *
*									     *
* PARAMETERS:								     *
*    n:  The Node to collapse.						     *
*    Qt: A pointer to the Vertex tree.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSCollapseNode(HDSNodeStruct *n, HDSOctreeStruct *Qt)
{
    int i;
    HDSTriangleStruct* Tmp;

    if (n != NULL) {
	n -> Status = HDS_BOUNDARY;
    
	for (i = 0;i<8;i++) {
	    HDSNodeStruct* Child = HDSGetChild(i,n);
	    if (Child != NULL) {
		if (Child -> Status == HDS_ACTIVE) HDSCollapseNode(Child,Qt);
		Child -> Status = HDS_INACTIVE;
	    }
	}

	if (n -> Tris != NULL) {
	    Tmp = n -> Tris -> Head;
	    while (Tmp != NULL) {
		/* Verify if the point is in the Node. */
		for (i = 0; i<3; i++) {
		    if (HDSPointInNode(Tmp -> PBase -> Vert[i], n))
			 /* Now the Proxies point to the right Node. */
			 Tmp -> PBase -> Proxies[i] = n;   
		} 

		Tmp = Tmp -> Pnext;
	    }
	}

	if (n -> SubTris != NULL) {
	    Tmp = n -> SubTris -> Head;
	    while (Tmp != NULL) {
		HDSTriRemove(Tmp,Qt -> ActiveList);
		Tmp = Tmp -> Pnext;
	    }
	}
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Expand a Node in the Vertex tree according to the HDS algorithm.	     *
*									     *
* PARAMETERS:								     *
*    n:     The Node to expand.						     *
*    Qt:    A pointer to the Vertex tree.				     *
*    Limit: Error threshold or triangle budget.				     *
*    Mode:  A flag indicating if the criterium is error threshold or         *
*           triangle budget.						     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSExpandNode(HDSNodeStruct *n,
			  HDSOctreeStruct *Qt,
			  IrtRType Limit,
			  int Mode)
{
    HDSNodeStruct *Child;
    int i, j;
    HDSTriangleStruct *Tmp, *Tri;

    if (n != NULL)    {
	for (i = 0; i < 8; i++) {
	    Child = HDSGetChild(i, n);
	    if (Child != NULL)
		Child -> Status = HDS_BOUNDARY;
	} 
	n -> Status = HDS_ACTIVE;

	if (n -> Tris != NULL) {
	    Tmp = n -> Tris -> Head;
	    while (Tmp != NULL) {
		/* If the point is in the Node then correct it. */
		for (i = 0; i < 3; i++) {
		    if (HDSPointInNode(Tmp -> PBase -> Vert[i], n)) {
			for (j = 0;j < 8; j++) {
			    Child = HDSGetChild(j, n);
			    if (Child != NULL &&
			        HDSPointInNode(Tmp -> PBase -> Vert[i], Child))
			        /* Now the proxie point to the right Node. */
				Tmp -> PBase -> Proxies[i] = Child;   
			}
		    }
		}

		Tmp = Tmp -> Pnext;
	    }
	}

	if (n -> SubTris != NULL) {
	    Tmp = n -> SubTris -> Head;
	    while (Tmp != NULL) {
		Tri = HDSCreateTri(Tmp -> PBase);
		Tri -> Id = Tmp -> Id;
		HDSTriInsert(Tri, &Qt -> ActiveList);
		Tmp = Tmp -> Pnext;
	    }
	}

	for (i = 0; i < 8; i++) {
	    Child = HDSGetChild(i, n);
	    if (Child != NULL) {
		if (Child -> WeightRep > 1) {
		    if (Mode == HDS_THRESHOLD_MODE) 
			HDSAdjustTree(Child, Qt, Limit);
		    if (Mode == HDS_TBQ_INIT_MODE) 
			HDSExpandNode(Child, Qt, 0, HDS_TBQ_INIT_MODE);
		}
	    }
	}
    } 
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Traverse the Vertex tree, and expand or collapse the Nodes according    *
*    to the error threshold.						     *
*									     *
* PARAMETERS:								     *
*    n:    The the top Node, necessary for the recursive call.		     *
*    Qt:   A pointer to the Vertex tree.				     *
*    Threshold: The error threshold.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSAdjustTree(HDSNodeStruct *n,
			  HDSOctreeStruct *Qt,
			  IrtRType Threshold)
{
    int i;
    IrtRType Size;

    if (n == NULL)
        return;
    Size = HDSNodeSize(n);
    if (Size >= Threshold) {
	if (n -> Status == HDS_ACTIVE) {
	    for (i = 0; i < 8; i++) {
		HDSNodeStruct
		    *Child = HDSGetChild(i, n);

		HDSAdjustTree(Child, Qt, Threshold);
	    }
	}
	else /*if (n -> WeightRep > 1)*/
	    HDSExpandNode(n, Qt, Threshold, HDS_THRESHOLD_MODE);
	   /* If the Node has less than 2 Vertices, no reason to expand it. */
    }
    else {
	if (n -> Status == HDS_ACTIVE)
	    HDSCollapseNode(n,Qt);
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Create and initialize a Node of the Vertex tree.			     *
*									     *
* PARAMETERS:								     *
*    None								     *
*									     *
* RETURN VALUE:							             *
*   HDSNodeStruct*:   A pointer to new Node.				     *
*****************************************************************************/
static HDSNodeStruct *HDSCreateNode(void)
{
    HDSNodeStruct *n = IritMalloc(sizeof(HDSNodeStruct));
    
    n -> Child0 = NULL;
    n -> Child1 = NULL;
    n -> Child2 = NULL;
    n -> Child3 = NULL;
    n -> Child4 = NULL;
    n -> Child5 = NULL;
    n -> Child6 = NULL;
    n -> Child7 = NULL;
    n -> Parent = NULL;
    n -> WeightRep = 0;
    n -> RepVert[0] = 0;
    n -> RepVert[1] = 0;
    n -> RepVert[2] = 0;
    
    n -> Radius = 0;
    n -> Status = HDS_INACTIVE;

    n -> Tris = NULL;
    n -> SubTris = NULL;

    return n;
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Creates an empty octree of given Depth covering a given portion of space.*
*									     *
* PARAMETERS:								     *
*   MaxDepth:   The maximum Depth of the tree.				     *
*   Xmin, Xmax, Ymin, Ymax, Zmin, Zmax:   The extreme coordinates of the     *
*		volume covered by the octree.				     *
*									     *
* RETURN VALUE:							             *
*    HDSOctreeStruct*: a pointer to the new octree (Vertex tree).   	     *
*****************************************************************************/
static HDSOctreeStruct* HDSCreateOctree(int MaxDepth, 
					IrtRType Xmin, 
					IrtRType Xmax, 
					IrtRType Ymin, 
					IrtRType Ymax, 
					IrtRType Zmin, 
					IrtRType Zmax)
{
    HDSOctreeStruct
	*Qt = IritMalloc(sizeof(HDSOctreeStruct));
    
    Qt -> MaxDepth = MaxDepth;
    Qt -> Root = HDSCreateNode();
    Qt -> Root -> Depth = 0;
    Qt -> Root -> Xmin = Xmin;
    Qt -> Root -> Xmax = Xmax;
    Qt -> Root -> Ymin = Ymin;
    Qt -> Root -> Ymax = Ymax;
    Qt -> Root -> Zmin = Zmin;
    Qt -> Root -> Zmax = Zmax;
    Qt -> ActiveList = NULL;
    Qt -> TriangleList = HDSCreateTribaseList();
    Qt -> Tbq = HDSCreateTBQ();
    
    return Qt;
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Remove a Node and all its Children from a given octree.		     *
*									     *
* PARAMETERS:								     *
*   n:    A pointer to the Node that is to be removed.			     *
*   Qt    pointer to the octree.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSOctreeRemove(HDSNodeStruct *n, HDSOctreeStruct *Qt)
{
    if (n == NULL)
        return;		/* End condition. */

    /* Recursive calls. */
    HDSOctreeRemove(n -> Child0, Qt);
    HDSOctreeRemove(n -> Child1, Qt);
    HDSOctreeRemove(n -> Child2, Qt);
    HDSOctreeRemove(n -> Child3, Qt);
    HDSOctreeRemove(n -> Child4, Qt);    
    HDSOctreeRemove(n -> Child5, Qt);
    HDSOctreeRemove(n -> Child6, Qt);
    HDSOctreeRemove(n -> Child7, Qt);

    HDSFreeList(n -> SubTris);
    HDSFreeList(n -> Tris);

    IritFree(n);
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Initialize a given childinside a given node.			     * 
*									     *
* PARAMETERS:								     *
*   n:        A pointer to the father Node.			 	     *
*   index:    The index of the child.					     *
*									     *
* RETURN VALUE:							             *
*    void								     *
*****************************************************************************/
static void HDSSetChildAttributes(HDSNodeStruct *n, int index)
{
    int d = n -> Depth + 1;

    switch(index) {
	case 0:
	    n -> Child0 -> Parent = n;
	    n -> Child0 -> Depth = d;
	    n -> Child0 -> Xmin = n -> Xmin;
	    n -> Child0 -> Xmax = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child0 -> Ymin = n -> Ymin;
	    n -> Child0 -> Ymax = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child0 -> Zmin = n -> Zmin;
	    n -> Child0 -> Zmax = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    break;

	case 1:
	    n -> Child1 -> Parent = n;
	    n -> Child1 -> Depth = d;
	    n -> Child1 -> Xmin = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child1 -> Xmax = n -> Xmax;
	    n -> Child1 -> Ymin = n -> Ymin;
	    n -> Child1 -> Ymax = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child1 -> Zmin = n -> Zmin;
	    n -> Child1 -> Zmax = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    break;

	case 2:
	    n -> Child2 -> Parent = n;
	    n -> Child2 -> Depth = d;
	    n -> Child2 -> Xmin = n -> Xmin;
	    n -> Child2 -> Xmax = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child2 -> Ymin = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child2 -> Ymax = n -> Ymax;
	    n -> Child2 -> Zmin = n -> Zmin;
	    n -> Child2 -> Zmax = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    break;
	    
	case 3:
	    n -> Child3 -> Parent = n;
	    n -> Child3 -> Depth = d;
	    n -> Child3 -> Xmin = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child3 -> Xmax = n -> Xmax;
	    n -> Child3 -> Ymin = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child3 -> Ymax = n -> Ymax;
	    n -> Child3 -> Zmin = n -> Zmin;
	    n -> Child3 -> Zmax = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    break;
	    
	case 4:
	    n -> Child4 -> Parent = n;
	    n -> Child4 -> Depth = d;
	    n -> Child4 -> Xmin = n -> Xmin ;
	    n -> Child4 -> Xmax = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child4 -> Ymin = n -> Ymin ;
	    n -> Child4 -> Ymax = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child4 -> Zmin = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    n -> Child4 -> Zmax = n -> Zmax;
	    break;

	case 5:
	    n -> Child5 -> Parent = n;
	    n -> Child5 -> Depth = d;
	    n -> Child5 -> Xmin = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child5 -> Xmax = n -> Xmax;
	    n -> Child5 -> Ymin = n -> Ymin ;
	    n -> Child5 -> Ymax = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child5 -> Zmin = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    n -> Child5 -> Zmax = n -> Zmax;
	    break;
	    
	case 6:
	    n -> Child6 -> Parent = n;
	    n -> Child6 -> Depth = d;
	    n -> Child6 -> Xmin = n -> Xmin ;
	    n -> Child6 -> Xmax = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child6 -> Ymin = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child6 -> Ymax = n -> Ymax;
	    n -> Child6 -> Zmin = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    n -> Child6 -> Zmax = n -> Zmax;
	    break;

	case 7:
	    n -> Child7 -> Parent = n;
	    n -> Child7 -> Depth = d;
	    n -> Child7 -> Xmin = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
	    n -> Child7 -> Xmax = n -> Xmax;
	    n -> Child7 -> Ymin = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
	    n -> Child7 -> Ymax = n -> Ymax;
	    n -> Child7 -> Zmin = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;
	    n -> Child7 -> Zmax = n -> Zmax;
	    break;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Allocate memory for a given child of a given node, and initialize	     *
*   the child.								     *
*									     *
* PARAMETERS:								     *
*   index:   The index of the child.			 		     *
*   n:       A pointer to the father Node.		 		     *
*									     *
* RETURN VALUE:							             *
*   HDSNodeStruct *: A pointer to the new child.			     *
*****************************************************************************/
static HDSNodeStruct *HDSCreateChild(int index, HDSNodeStruct *n)
{
    HDSSetChild(n,index,HDSCreateNode());
    HDSSetChildAttributes(n,index);
    return HDSGetChild(index,n);
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Return the index of the child a given vertex belongs to inside a	     *
*   given node.								     *
*									     *
* PARAMETERS:								     *
*   n:		A pointer to the father Node.			 	     *
*   x, y, z:    The coordinates of the vertex.				     *
*									     *
* RETURN VALUE:							             *
*    int								     *
*****************************************************************************/
static int HDSGetChildIndex(HDSNodeStruct *n, double x, double y, double z)
{
    int Index = 0;
    IrtRType Xmiddle, Ymiddle, Zmiddle;

    Xmiddle = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
    Ymiddle = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
    Zmiddle = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;

    if (x > Xmiddle)
        Index += 1;
    if (y > Ymiddle)
        Index += 2;
    if (z > Zmiddle)
        Index += 4;

    return Index;
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Insert a Vertex into the Vertex tree after updating the representative   *
*   Vertex of the appropriate Nodes, as well as their Lists of Tris and      *
*   SubTris. The insertion is made top to bottom.			     *
*									     *
* PARAMETERS:								     *
*    Id:      The Vertex ID in its Triangle (0, 1 or 2).		     *
*    x, y, z: The coordinates of the Vertex.				     *
*    HDSTribaseStruct *: A pointer to the original Triangle the Vertex       *
*		 belongs to.						     *
*    HDSNodeStruct *:    A pointer to the Node the insertions starts from at *
*		 the top.						     *
*    HDSOctreeStruct *:  A pointer to the Vertex tree.			     *
*									     *
* RETURN VALUE:							             *
*    HDSNodeStruct *:  A pointer to the lowest Node this Vertex was inserted *
*		into.							     *
*****************************************************************************/
static HDSNodeStruct *HDSOctreeInsertVertex(int Id,
					    IrtRType x,
					    IrtRType y,
					    IrtRType z,
					    IrtNrmlType norm,
					    HDSTribaseStruct *Tri,
					    HDSNodeStruct *n,
					    HDSOctreeStruct *Qt)
{
    int RVIndex = 0,
	Index = 0;
    IrtRType Xmiddle, Ymiddle, Zmiddle;
    HDSNodeStruct *Child = NULL;

    if (n == NULL)
	return NULL;	/* End condition. */

    /* Update the representative Vertex of the Node and its weight. */
    if (n->WeightRep == 0) {
	n->RepVert[0]=x;
	n->RepVert[1]=y;
	n->RepVert[2]=z;
	n->RepNorm[0]=norm[0];
	n->RepNorm[1]=norm[1];
	n->RepNorm[2]=norm[2];
	n->WeightRep++;
	return n;
    }

    Xmiddle = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
    Ymiddle = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
    Zmiddle = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;

    if (x > Xmiddle)
        Index += 1;
    if (y > Ymiddle)
        Index += 2;
    if (z > Zmiddle)
        Index += 4;

    /* If n is a leaf, create nodes for the repvert and the new vert 	    */
    /* until they split.						    */
    /* If n is not a leaf,						    */
    /*	   if the appropriate child exists, insert the new vertex into it   */
    /*	   if not, create the child and insert the new vertex into it.      */
    /* Nodes can be created only within the depth boundary.		    */
    if (HDSIsLeaf(n)) {
	/* If the new vertex is not distinct from the repvert return */
	if (x == n->RepVert[0] && 
	    y == n->RepVert[1] && 
	    z == n->RepVert[2])
	    return NULL;

	/* If the max depth is reached return */
	if (n->Depth >= Qt->MaxDepth)
	    return NULL;

	/* Now we know that the two vertices are distinct, we must go 	     */
	/* down the tree until we can put them in two separate leaves.       */
	RVIndex=HDSGetChildIndex(n,n->RepVert[0],n->RepVert[1],n->RepVert[2]);

	/* First create a child for the repvert. In case the repvert is	     */
	/* a multiple vertex, copy the weightrep.			     */
	Child = HDSCreateChild(RVIndex,n);
	HDSOctreeInsertVertex(0,
			      n -> RepVert[0],
			      n -> RepVert[1],
			      n -> RepVert[2],
			      n -> RepNorm,
			      NULL,
			      Child,
			      Qt);

	Child->WeightRep = n->WeightRep;
	/* Now insert the new vertex. If it belongs in the same child        */
	/* with the repvert, just insert it. If it belongs to another	     */
	/* child, create the child then insert the vertex.		     */
	if (RVIndex==Index) {
	    return HDSOctreeInsertVertex(Id, x, y, z, norm, NULL, n, Qt);
	}
	else {
	    Child = HDSCreateChild(Index,n);
	    return HDSOctreeInsertVertex(Id, x, y, z, norm, NULL, n, Qt);
	}
    }
    else { /* n is not a leaf. */
	n -> RepVert[0] = (n -> WeightRep*n -> RepVert[0] + x) / (n -> WeightRep+1);
	n -> RepVert[1] = (n -> WeightRep*n -> RepVert[1] + y) / (n -> WeightRep+1);
	n -> RepVert[2] = (n -> WeightRep*n -> RepVert[2] + z) / (n -> WeightRep+1);
	n -> RepNorm[0] = (n -> WeightRep*n -> RepNorm[0] + norm[0]) / (n -> WeightRep+1);
	n -> RepNorm[1] = (n -> WeightRep*n -> RepNorm[1] + norm[1]) / (n -> WeightRep+1);
	n -> RepNorm[2] = (n -> WeightRep*n -> RepNorm[2] + norm[2]) / (n -> WeightRep+1);
	n -> WeightRep++;
	Child = HDSGetChild(Index, n);
	if (Child != NULL)
	    return HDSOctreeInsertVertex(Id,x,y,z,norm,NULL,Child,Qt);
	else { /* The child does not exist. */
	    if (n->Depth < Qt->MaxDepth) {
		Child=HDSCreateChild(Index,n);
		return HDSOctreeInsertVertex(Id,x,y,z,norm,NULL,Child,Qt);
	    }
	    else
	        return NULL;
	}
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Update the Lists of Tris and SubTris in the vertex tree.		     *
*   The insertion is made top to bottom.				     *
*									     *
* PARAMETERS:								     *
*   Id:      The Vertex ID in its Triangle (0, 1 or 2).			     *
*   x, y, z: The coordinates of the Vertex.				     *
*   HDSTribaseStruct *: A pointer to the original Triangle the Vertex        *
*		 belongs to.						     *
*   HDSNodeStruct *:    A pointer to the Node the insertions starts from at  *
*		 the top.						     *
*   HDSOctreeStruct *:  A pointer to the Vertex tree.			     *
*									     *
* RETURN VALUE:							             *
*    HDSNodeStruct *:  A pointer to the lowest Node this Vertex was inserted *
*		into.							     *
*****************************************************************************/
static HDSNodeStruct *HDSOctreeUpdateNodes(int Id,
					   IrtRType x,
					   IrtRType y,
					   IrtRType z,
					   HDSTribaseStruct* Tri,
					   HDSNodeStruct* n,
					   HDSOctreeStruct *Qt)
{
    int i,
	Index = 0,
	Index1 = 0,
	Index2 = 0,
	Index3 = 0,
	NumVert = 0;    /* Number of Vertices of the Triangle in the Node. */
    IrtRType Xmiddle, Ymiddle, Zmiddle;
    HDSTriangleStruct* TriTemp;
    HDSNodeStruct *Corner, *Child;
    
    if (n == NULL)
	return NULL;	/* End condition. */
 
    Xmiddle = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
    Ymiddle = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
    Zmiddle = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;

    if (x > Xmiddle)
        Index += 1;
    if (y > Ymiddle)
        Index += 2;
    if (z > Zmiddle)
        Index += 4;

    /* Decide what child the vertex belongs to... */
    if (Tri -> Vert[0][0] > Xmiddle)
        Index1 += 1;
    if (Tri -> Vert[0][1] > Ymiddle)
        Index1 += 2;
    if (Tri -> Vert[0][2] > Zmiddle)
        Index1 += 4;
    if (Tri -> Vert[1][0] > Xmiddle)
        Index2 += 1;
    if (Tri -> Vert[1][1] > Ymiddle)
        Index2 += 2;
    if (Tri -> Vert[1][2] > Zmiddle)
        Index2 += 4;
    if (Tri -> Vert[2][0] > Xmiddle)
        Index3 += 1;
    if (Tri -> Vert[2][1] > Ymiddle)
        Index3 += 2;
    if (Tri -> Vert[2][2] > Zmiddle)
        Index3 += 4;

    if (Tri -> Vert[0][0] <= n -> Xmin || Tri -> Vert[0][0] > n -> Xmax)
        Index1 = -1;
    if (Tri -> Vert[0][1] <= n -> Ymin || Tri -> Vert[0][1] > n -> Ymax)
        Index1 = -1;    
    if (Tri -> Vert[0][2] <= n -> Zmin || Tri -> Vert[0][2] > n -> Zmax)
        Index1 = -1;    
    if (Tri -> Vert[1][0] <= n -> Xmin || Tri -> Vert[1][0] > n -> Xmax)
        Index2 = -1;
    if (Tri -> Vert[1][1] <= n -> Ymin || Tri -> Vert[1][1] > n -> Ymax)
        Index2 = -1;
    if (Tri -> Vert[1][2] <= n -> Zmin || Tri -> Vert[1][2] > n -> Zmax)
        Index2 = -1;    
    if (Tri -> Vert[2][0] <= n -> Xmin || Tri -> Vert[2][0] > n -> Xmax)
        Index3 = -1;
    if (Tri -> Vert[2][1] <= n -> Ymin || Tri -> Vert[2][1] > n -> Ymax)
        Index3 = -1;
    if (Tri -> Vert[2][2] <= n -> Zmin || Tri -> Vert[2][2] > n -> Zmax)
        Index3 = -1;    

    for (i = 0; i < 3; i++) {
	if (Tri -> Vert[i][0] > n -> Xmin &&
	    Tri -> Vert[i][0] <= n -> Xmax &&
	    Tri -> Vert[i][1] > n -> Ymin &&
	    Tri -> Vert[i][1] <= n -> Ymax &&
	    Tri -> Vert[i][2] > n -> Zmin &&
	    Tri -> Vert[i][2] <= n -> Zmax)
	    NumVert++;
    }
 
    /* Update the lists Tris and SubTris... */
    TriTemp = HDSCreateTri(Tri);
    TriTemp -> Id = Tri -> Id;
    
    /* Tris is a list of triangles with exactly one corner in the node      */
    /* these are the triangles which must be adjusted when the node is      */
    /* folded or unfolded.						    */
    if (NumVert == 1) {
	HDSEnqueue(TriTemp, &n -> Tris);
	if (Tri -> Proxies[Id] == NULL)
	    Tri -> Proxies[Id] = n;
    }

    /* SubTris is a List of triangles with two or three corners within the   */
    /* node but no more than one corner in each child.			     */
    /* These are the triangles which must be filtered out when the node is   */
    /* folded and reintroduced when the node is unfolded.		     */
    else {
	/* If all of the the Triangle is in one of the deepest Nodes it is   */
	/* irrelevant.							     */
	if (!(NumVert > 1 && HDSIsLeaf(n))) {
	    if (((Index1 != Index2 && Index1 != Index3 && Index2 != Index3) || 
		 HDSIsLeaf(n)) && 
		((Index1 != -1 && Index2 != -1) ||
		 (Index1 != -1 && Index3 != -1)||
		 (Index2 != -1 && Index3 != -1)))
	        HDSEnqueue(TriTemp,&n -> SubTris);
	}
    }

    Child = HDSGetChild(Index, n);
    if (Child != NULL) {
	if ((Corner = HDSOctreeUpdateNodes(Id, x, y, z, Tri,
					   Child ,Qt)) == NULL) { 
	    Tri -> Corners[Id] = Child;
	    if (Tri -> Proxies[Id] == NULL)
	        Tri -> Proxies[Id] = Child;
	    return Child;
	}
	else {
	    Tri -> Corners[Id] = Corner;
	    return Corner;
	}
    }
    else
        return NULL;
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Calculate the radius (error approximation * 0.5) of the nodes	     *
* simulate inserting a vertex but instead compare its distance to the        *
* repvert with the current radius and update to maximum. The radius	     *
* cannot be longer than half the diagonal of the Node.			     *
*   The radius are correct once all of the Vertices have been "inserted".    *
*									     *
* PARAMETERS:								     *
*   x, y, z:     The coordinates of the Vertex Currently being "inserted".   *
*   HDSNodeStruct *:   A pointer to the Node the insertions starts from at   *
*		the top							     *
*   HDSOctreeStruct *: A pointer to the Vertex tree.			     *
*									     *
* RETURN VALUE:							             *
*   HDSNodeStruct*: A pointer to the lowest Node this Vertex was inserted    *
*		into.							     *
*****************************************************************************/
static void HDSOctreeCalculateRadius(IrtRType x,
				     IrtRType y,
				     IrtRType z,
				     HDSNodeStruct *n,
				     HDSOctreeStruct *Qt)
{
    int Index = 0;
    IrtRType Xmiddle, Ymiddle, Zmiddle,
	TmpRadius = 0,
	Diagonal = 0;
    HDSNodeStruct *Child;

    if (n == NULL)
	return;     /* End condition. */
    
    TmpRadius = sqrt(pow(x - n -> RepVert[0], 2)+
		     pow(y - n -> RepVert[1], 2) +
		     pow(z - n -> RepVert[2], 2)); 
    Diagonal = sqrt(pow(n -> Xmax - n -> Xmin, 2)+
		    pow(n -> Ymax - n -> Ymin, 2) +
		    pow(n -> Zmax - n -> Zmin, 2)); 
    if (TmpRadius > n -> Radius)
        n -> Radius = TmpRadius;
    if (n -> Radius > 0.5 * Diagonal)
        n -> Radius = 0.5 * Diagonal;
    
    Xmiddle = n -> Xmin + (n -> Xmax - n -> Xmin) * 0.5;
    Ymiddle = n -> Ymin + (n -> Ymax - n -> Ymin) * 0.5;
    Zmiddle = n -> Zmin + (n -> Zmax - n -> Zmin) * 0.5;

    if (x > Xmiddle)
        Index += 1;
    if (y > Ymiddle)
        Index += 2;
    if (z > Zmiddle)
        Index += 4;

    Child = HDSGetChild(Index, n);
    if (Child != NULL) {
	HDSOctreeCalculateRadius(x, y, z, Child, Qt);
	return;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Creates the Vertex tree, then updates all of the fields in the Nodes    *
* also initializes the Triangle budget queue.				     *
*									     *
* PARAMETERS:								     *
*   TriangleList:        The List of all the Triangles in the scene.	     *
*   Xmin, Xmax, Ymin, Ymax, Zmin, Zmax:   Extreme coordinates of the volume. *
*    covered by the octree						     *
*   Depth: The maximum Depth of the tree				     *
*									     *
* RETURN VALUE:							             *
*   HDSNodeStruct *: A pointer to lowest Node this Vertex was inserted into. *
*****************************************************************************/
static HDSOctreeStruct *HDSInitOctree(HDSTribaseListStruct *TriangleList,
				      IrtRType Xmin,
				      IrtRType Xmax,
				      IrtRType Ymin,
				      IrtRType Ymax,
				      IrtRType Zmin,
				      IrtRType Zmax,
				      int Depth)
{
    HDSOctreeStruct *Qt;
    HDSTribaseStruct *Tri;

    /* Create an empty Vertex tree. */
    Qt = HDSCreateOctree(Depth, Xmin, Xmax, Ymin, Ymax, Zmin, Zmax);

    /* Insert Vertices in the Vertex tree. */
    Qt -> TriangleList = TriangleList;
    Tri = Qt -> TriangleList -> Head;
    while (Tri != NULL) {
        HDSOctreeInsertVertex(0,
			      Tri -> Vert[0][0],
			      Tri -> Vert[0][1],
			      Tri -> Vert[0][2],
			      Tri -> Norm[0],
			      Tri,
			      Qt -> Root,Qt);
	HDSOctreeInsertVertex(1,
			      Tri -> Vert[1][0],
			      Tri -> Vert[1][1],
			      Tri -> Vert[1][2],
			      Tri -> Norm[1],
			      Tri,
			      Qt -> Root,Qt);
	HDSOctreeInsertVertex(2,
			      Tri -> Vert[2][0],
			      Tri -> Vert[2][1],
			      Tri -> Vert[2][2],
			      Tri -> Norm[2],
			      Tri,
			      Qt -> Root,
			      Qt);
	Tri = Tri -> Pnext;
    }

    /* Update tris and subtris of the nodes in the vertex tree */
    Tri = Qt -> TriangleList -> Head;
    while (Tri != NULL) {
        HDSOctreeUpdateNodes(0,
			     Tri -> Vert[0][0],
			     Tri -> Vert[0][1],
			     Tri -> Vert[0][2],
			     Tri,
			     Qt -> Root,Qt);
	HDSOctreeUpdateNodes(1,
			     Tri -> Vert[1][0],
			     Tri -> Vert[1][1],
			     Tri -> Vert[1][2],
			     Tri,
			     Qt -> Root,Qt);
	HDSOctreeUpdateNodes(2,
			     Tri -> Vert[2][0],
			     Tri -> Vert[2][1],
			     Tri -> Vert[2][2],
			     Tri,
			     Qt -> Root,
			     Qt);
	Tri = Tri -> Pnext;
    }

    /* Calculate the radii of the nodes in the vertex tree. */
    Tri = Qt -> TriangleList -> Head;
    while (Tri != NULL) {
        HDSOctreeCalculateRadius(Tri -> Vert[0][0],
				 Tri -> Vert[0][1],
				 Tri -> Vert[0][2],
				 Qt -> Root,
				 Qt);
	HDSOctreeCalculateRadius(Tri -> Vert[1][0],
				 Tri -> Vert[1][1],
				 Tri -> Vert[1][2],
				 Qt -> Root,
				 Qt);
	HDSOctreeCalculateRadius(Tri -> Vert[2][0],
				 Tri -> Vert[2][1],
				 Tri -> Vert[2][2],
				 Qt -> Root,
				 Qt);
	Tri = Tri -> Pnext;
    }

    HDSInitTBQ(Qt,Qt -> Root);

    return Qt;
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Defines a node as a child of another.				     *
*									     *
* PARAMETERS:								     *
*   n:     The parent node.						     *
*   i:     The index of the child (0-7).				     *
*   Value: The child node.						     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSSetChild(HDSNodeStruct *n, int i, HDSNodeStruct *Value)
{
    switch (i) {
	case 0:
	    n -> Child0 = Value;
	    break;
	case 1:
	    n -> Child1 = Value;
	    break;
	case 2:
	    n -> Child2 = Value;
	    break;
	case 3:
	    n -> Child3 = Value;
	    break;
	case 4:
	    n -> Child4 = Value;
	    break;
	case 5:
	    n -> Child5 = Value;
	    break;
	case 6:
	    n -> Child6 = Value;
	    break;
	case 7:
	    n -> Child7 = Value;
	    break;
	default: ;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Fetches child number i of a given node.				     *
*									     *
* PARAMETERS:								     *
*    i: Index of the child (0-7).					     *
*    n: the parent node.						     *
*									     *
* RETURN VALUE:							             *
*    HDSNodeStruct*:   A pointer to the child node.			     *
*****************************************************************************/
static HDSNodeStruct *HDSGetChild(int i, HDSNodeStruct *n)
{
    switch (i) {
	case 0:
	    return n -> Child0;
	case 1:
	    return n -> Child1;
	case 2:
	    return n -> Child2;
	case 3:
	    return n -> Child3;
	case 4:
	    return n -> Child4;
	case 5:
	    return n -> Child5;
	case 6:
	    return n -> Child6;
	case 7:
	    return n -> Child7;
	default:
	    return NULL;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Indicates whether a Vertex is within a given Node or not.		     *
*									     *
* PARAMETERS:								     *
*    Pt:     Coordinates of the vertex.					     *
*    n:      Pointer to the node.					     *
*									     *
* RETURN VALUE:							             *
*    BOOLEAN: TRUE if the vertex is in the node.			     *
*****************************************************************************/
static int HDSPointInNode(IrtPtType Pt, HDSNodeStruct *n)
{

    if (Pt[0] <= n -> Xmax && Pt[0] > n -> Xmin && Pt[1] <= n -> Ymax &&
	Pt[1] > n -> Ymin && Pt[2] <= n -> Zmax && Pt[2] > n -> Zmin)
	return TRUE;
    else
	return FALSE;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Returns the radius of the Node.					     *
*									     *
* PARAMETERS:								     *
*    n:   A pointer to the node.					     *
*									     *
* RETURN VALUE:							             *
*    IrtRType:   The radius.						     *
*****************************************************************************/
static IrtRType HDSNodeSize(HDSNodeStruct *n)
{
    if (n != NULL)
	return n -> Radius;
    else
        return -1;
}

/*****************************************************************************
* DESCRIPTION:							             *
*   Converts the active list into an IPObjectStruct.			     *
*									     *
* PARAMETERS:								     *
*   AList:   A pointer to the active List.				     *
*									     *
* RETURN VALUE:							             *
*   IPObjectStruct *:  An IPObjectStruct made of Triangles.		     *
*****************************************************************************/
static IPObjectStruct *HDSConVertActiveList2PObj(HDSTriListStruct *AList)
{
    int i, j;
    IPVertexStruct
	*Vertex = NULL,
	*TmpVer = NULL,
	*HeadVer = NULL;
    IPPolygonStruct
	*Poly = NULL,
	*TmpPoly = NULL,
	*HeadPoly = NULL;
    IPObjectStruct
	*PObjects = IPGenPolyObject("PolySimp", NULL, NULL);
    struct HDSTriangleStruct
	*Tri = NULL;

    if (AList != NULL) {
	for (Tri = AList -> Head; Tri != NULL; Tri = Tri -> Pnext) {
	    for (j = 0; j < 3; j++) {
		TmpVer = IPAllocVertex2(NULL);
		for (i = 0; i < 3; i++) {
		    TmpVer -> Coord[i] = Tri -> PBase -> Proxies[j] -> RepVert[i];
		    TmpVer -> Normal[i] = Tri -> PBase -> Proxies[j] -> RepNorm[i];
		}
		IP_RST_NORMAL_VRTX(TmpVer);
		if (j == 0)
		    HeadVer = Vertex = TmpVer;
		else { 
		    Vertex -> Pnext = TmpVer;
		    Vertex = Vertex -> Pnext;
		}
	    }
	    
	    TmpPoly = IPAllocPolygon(0, NULL, NULL);
	    TmpPoly -> PVertex = HeadVer;

	    /* Make sure it's a Polygon and not a Polyline. */
	    IP_SET_POLYGON_OBJ(TmpPoly); 
	    if (IRIT_PT_APX_EQ_ZERO_EPS(TmpPoly -> Plane, IRIT_UEPS)) {
		if (!IPUpdatePolyPlane(TmpPoly)) {
		    /* Default to plane Z = 0. */
		    IRIT_PLANE_RESET(TmpPoly -> Plane);
		    TmpPoly -> Plane[2] = 1.0;
		}
	    }
	    if (Tri == AList -> Head)
		HeadPoly = Poly = TmpPoly;
	    else {
		Poly -> Pnext = TmpPoly;
		Poly = Poly -> Pnext;
	    }
	}

	PObjects -> U.Pl = HeadPoly;
    }
    return PObjects;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Create a Triangle budget queue.					     *
*									     *
* PARAMETERS:								     *
*    None								     *
*									     *
* RETURN VALUE:							             *
*    TBQueueStruct *:   A pointer to the new queue			     *
*****************************************************************************/
static TBQueueStruct *HDSCreateTBQ(void)
{
    TBQueueStruct
	*q = IritMalloc(sizeof(TBQueueStruct));

    q -> Head = NULL;
    q -> Tail = NULL;
    q -> Current = NULL;
    q -> Count = 0;
    q -> NumActive = 0;

    return q;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Create a node for the triangle budget queue.			     *
*									     *
* PARAMETERS:								     *
*    None								     *
*									     *
* RETURN VALUE:							             *
*    HDSTBNodeStruct*:   A pointer to the new node.			     *
*****************************************************************************/
static HDSTBNodeStruct* HDSCreateTBNode(void)
{
    HDSTBNodeStruct
	*n = IritMalloc(sizeof(HDSTBNodeStruct));

    n -> Pnext = NULL;
    n -> Pprev = NULL;
    n -> Node = NULL;

    return n;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Insert a node into the triangle budget queue.			     *
*									     *
* PARAMETERS:								     *
*    n:   The Node to insert.						     *
*    q:   The queue.							     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSInsertToTBQ(HDSTBNodeStruct *n, TBQueueStruct *q)
{
    HDSTBNodeStruct *Tmp;

    if (q == NULL || n == NULL)
        return;
    if (q -> Head == NULL && q -> Tail == NULL) {
	q -> Head = q -> Tail = n;
	q -> Count++;
    }
    else {
	for (Tmp = q -> Head; Tmp != NULL; Tmp = Tmp -> Pnext) {
	    if (Tmp -> Node == n -> Node)
	        return;
	    if (Tmp -> Node -> Radius < n -> Node -> Radius)
	        break;
	    if (Tmp -> Node -> Radius == n -> Node -> Radius &&
		Tmp -> Node -> Depth >= n -> Node -> Depth)
	        break;
	}
	if (Tmp == NULL) {
	    q -> Tail -> Pnext = n;
	    n -> Pprev = q -> Tail;
	    q -> Tail = n;
	    n -> Pnext = NULL;
	}
	else {
	    n -> Pnext = Tmp;
	    n -> Pprev = Tmp -> Pprev;
	    if (Tmp -> Pprev != NULL)
	        Tmp -> Pprev -> Pnext = n;
	    else
	        q -> Head = n;
	    Tmp -> Pprev = n;
	
	}
	q -> Count++;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Free the memory alllocated to the triangle budget queue.		     *
*									     *
* PARAMETERS:								     *
*    q: the queue to free.						     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSFreeTBQ(TBQueueStruct *q)
{
    HDSTBNodeStruct *Tmp, *Tmp2;

    if (q == NULL)
        return;

    for (Tmp = q -> Head; Tmp != NULL; ) {
	Tmp2 = Tmp;
	Tmp = Tmp -> Pnext;
	IritFree(Tmp2);
    }
    IritFree(q);
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Initialize the triangle budget queue.				     *
*									     *
* PARAMETERS:								     *
*    Qt:   The vertex tree.						     *
*    n:    The first node of the queue.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSInitTBQ(HDSOctreeStruct *Qt, HDSNodeStruct *n)
{
    int i;
    HDSTBNodeStruct *Tbn;
    HDSNodeStruct *Child;

    if (n == NULL)
	return;

    /* If the node has less than 2 vertices, there is no reason to expand. */
    if (n -> WeightRep < 2)
        return; 

    for (i = 0; i < 8; i++) {
	Child = HDSGetChild(i, n);
	HDSInitTBQ(Qt, Child);
	Tbn = HDSCreateTBNode();
	Tbn -> Node = n;
	HDSInsertToTBQ(Tbn, Qt -> Tbq);
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Initialize the active list before using the triangle budget algorithm   *
*    this consists in expanding all the nodes.				     *
*									     *
* PARAMETERS:								     *
*    n:     The first Node of the queue.				     *
*    Qt:    The Vertex tree.						     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSTBQInitActiveList(HDSNodeStruct *n, HDSOctreeStruct *Qt)
{
    int i;
    if (n == NULL)
	return;

    if (n -> Status == HDS_ACTIVE) {
	for (i = 0; i < 8; i++) {
	    HDSNodeStruct
		*Child = HDSGetChild(i, n);

	    HDSTBQInitActiveList(Child, Qt);
	}
    }
    else if (n -> WeightRep > 1) {
        /* If node has less than 2 Vertices, there is no reason to expand. */
	HDSExpandNode(n, Qt, 0, HDS_TBQ_INIT_MODE);
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Traverse the triangle budget queue and collapse the appropriate nodes.  *
*									     *
* PARAMETERS:								     *
*    Qt:        The vertex tree.					     *
*    TriBudget: The Triangle budget.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSBudgetSimplify(HDSOctreeStruct *Qt, int TriBudget)
{
    HDSTBNodeStruct *Tbn;
    
    if (Qt -> Tbq == NULL)
	return;

    if (Qt -> ActiveList == NULL)
	Qt -> ActiveList = HDSCreateTriList();

    HDSTBQInitActiveList(Qt -> Root, Qt);
    Tbn = Qt -> Tbq -> Tail;
    while (Tbn != NULL) {
	if (Qt -> ActiveList -> Count <= TriBudget)
	    break;
	HDSCollapseNode(Tbn -> Node, Qt);
	Qt -> Tbq -> NumActive--;
	Tbn = Tbn -> Pprev;	
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Creates an element for the triangle list.				     *
*									     *
* PARAMETERS:								     *
*    x1, y1, z1:    The coordinates of the first Vertex.		     *
*    x2, y2, z2:    The coordinates of the second Vertex.		     *
*    x3, y3, z3:    The coordinates of the third Vertex.		     *
*									     *
* RETURN VALUE:							             *
*    HDSTribaseStruct *:   A pointer to the new Triangle.		     *
*****************************************************************************/
static HDSTribaseStruct *HDSCreateTribase(IrtRType x1,
					  IrtRType y1,
					  IrtRType z1,
					  IrtRType x2,
					  IrtRType y2,
					  IrtRType z2,
					  IrtRType x3,
					  IrtRType y3,
					  IrtRType z3,
					  IrtNrmlType n1,
					  IrtNrmlType n2,
					  IrtNrmlType n3)
{
    HDSTribaseStruct
	*Node = IritMalloc(sizeof(struct HDSTribaseStruct));

    Node -> Vert[0][0] = x1;
    Node -> Vert[0][1] = y1;
    Node -> Vert[0][2] = z1;
    Node -> Vert[1][0] = x2;
    Node -> Vert[1][1] = y2;
    Node -> Vert[1][2] = z2;
    Node -> Vert[2][0] = x3;
    Node -> Vert[2][1] = y3;
    Node -> Vert[2][2] = z3;
    Node -> Norm[0][0] = n1[0];
    Node -> Norm[0][1] = n1[1];
    Node -> Norm[0][2] = n1[2];
    Node -> Norm[1][0] = n2[0];
    Node -> Norm[1][1] = n2[1];
    Node -> Norm[1][2] = n2[2];
    Node -> Norm[2][0] = n3[0];
    Node -> Norm[2][1] = n3[1];
    Node -> Norm[2][2] = n3[2];

    Node -> Pnext = NULL;
    Node -> Pprev = NULL;
    Node -> Proxies[0] = NULL;
    Node -> Proxies[1] = NULL;
    Node -> Proxies[2] = NULL;

    return Node;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Creates a triangle list.						     *
*									     *
* PARAMETERS:								     *
*    None								     *
*									     *
* RETURN VALUE:							             *
*    HDSTribaseListStruct*:   A pointer to the new triangle list.	     *
*****************************************************************************/
static HDSTribaseListStruct *HDSCreateTribaseList(void)
{
    HDSTribaseListStruct
	*List  = IritMalloc(sizeof(struct HDSTribaseListStruct));

    List -> Head = NULL;
    List -> Tail = NULL;
    List -> Count = 0;

    return List;
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Free the memory allocated to the triangle list.			     *
*									     *
* PARAMETERS:								     *
*    List:   The list to be freed.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSFreeListbase(HDSTribaseListStruct *List)
{
    HDSTribaseStruct *HeadNext;

    if (List == NULL)
	return;
    
    while (List -> Head != NULL ) {
	HeadNext = List -> Head -> Pnext;
	HDSTribaseRemove(List -> Head, List);
	List -> Head = HeadNext;
	if (HeadNext != NULL)
	    HeadNext = HeadNext -> Pnext;
    }
    IritFree(List);
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Insert a triangle in the triangle list.				     *
*									     *
* PARAMETERS:								     *
*    Node:   The new triangle.						     *
*    PList:  A pointer to the triangle list.				     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSTribaseInsert(HDSTribaseStruct *Node,
			     HDSTribaseListStruct **PList)
{    
    if ((*PList) == NULL)
	(*PList) = HDSCreateTribaseList();
    
    /* If the *PList is empty. */
    if ((*PList) -> Head == NULL) {
	(*PList) -> Count++;
	(*PList) -> Head = (*PList) -> Tail = Node;
	Node -> Id = (*PList) -> Count;
    }
    else {
	(*PList) -> Count++;
	(*PList) -> Tail -> Pnext = Node;
	Node -> Pprev = (*PList) -> Tail;
	(*PList) -> Tail = Node;
	Node -> Id = (*PList) -> Count;
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Remove a triangle from the triangle list.				     *
*									     *
* PARAMETERS:								     *
*    Node:    The triangle to be removed.				     *
*    List:    The triangle list.					     *
*									     *
* RETURN VALUE:							             *
*    None								     *
*****************************************************************************/
static void HDSTribaseRemove(HDSTribaseStruct *Node,
			     HDSTribaseListStruct *List)
{
    HDSTribaseStruct *Tmp;

    if (Node == NULL)
	return;

    if ((Tmp = HDSFindTribase(Node, List)) != NULL) {
	if (Tmp -> Pprev == NULL && Tmp -> Pnext == NULL) {
	    IritFree(Tmp);
	    List -> Head = List -> Tail = NULL;
	}
	else if (Tmp -> Pprev == NULL) {
	    /* If Tmp is Head. */
	    List -> Head = List -> Head -> Pnext;
	    List -> Head -> Pprev = NULL;
	    IritFree(Tmp);
	}
	else if (Tmp -> Pnext == NULL) {
	    /* If Tmp is Tail. */
	    List -> Tail = Tmp -> Pprev;
	    List -> Tail -> Pnext = NULL;
	    IritFree(Tmp);
	}
	else {
	    /* Any other case. */
	    Tmp -> Pprev -> Pnext = Tmp -> Pnext;
	    Tmp -> Pnext -> Pprev = Tmp -> Pprev;
	    IritFree(Tmp);
	}
    }
}

/*****************************************************************************
* DESCRIPTION:							             *
*    Find a triangle in the triangle list.				     *
*									     *
* PARAMETERS:								     *
*    Node:   The triangle to be found.					     *
*    List:   The triangle list.						     *
*									     *
* RETURN VALUE:							             * 
*    HDSTribaseStruct*:    A pointer to the triangle if found, or else NULL. *
*****************************************************************************/
static HDSTribaseStruct *HDSFindTribase(HDSTribaseStruct *Node,
					HDSTribaseListStruct *List)
{
    int i,
    Num = 0;    /* Serves to know how many vertites are similar between     */
                /* our Tri and the one we are now checking.		    */
    HDSTribaseStruct
	*Tmp = List -> Head;

    if (Node == NULL || List == NULL)
	return NULL;
    
    while (Tmp != NULL) {
	Num = 0;
	for (i = 0; i < 3; i++) {
	    if (Node -> Vert[i][0] == Tmp -> Vert[i][0] && 
		Node -> Vert[i][1] == Tmp -> Vert[i][1] &&
		Node -> Vert[i][2] == Tmp -> Vert[i][2])
		Num++;
	    else
		break;
	}
	if (Num == 3)
	    return Tmp;
	else
	    Tmp = Tmp -> Pnext;
    }
    return NULL;
}

/*****************************************************************************
* DESCRIPTION:							             M
*   Creates the vertex tree from the IPObjectStruct.			     M
*									     *
* PARAMETERS:								     M
*   PObjects:    The IRIT object given as input.			     M
*   Depth:       The Depth of the tree (recommended value = 5)		     M
*									     *
* RETURN VALUE:							             M
*   VoidPtr:    A reference to the vertex tree (to be handed on to	     M
*		HDSThreshold and HDSTriBudget.				     M
*                                                                            *
* SEE ALSO:                                                                  M
*   HDSThreshold, HDSTriBudget, HDSFreeQTree				     M
*                                                                            *
* KEYWORDS:								     M
*   HDSCnvrtPObj2QTree							     M
*****************************************************************************/
VoidPtr HDSCnvrtPObj2QTree(IPObjectStruct *PObjects, int Depth)
{ 
    IPObjectStruct *Obj, *ObjTri;
    IPPolygonStruct *Pl;
    IPVertexStruct *Ver;
    HDSTribaseStruct *Tri;
    HDSTribaseListStruct *TriangleList;
    IrtNrmlType Normal[3];
    int Nbelt = 0,
	Nbpt = 0,
	Nbpl = 0;
    IrtRType x[3], y[3], z[3],
	Xmin = IRIT_INFNTY,
	Xmax = -IRIT_INFNTY,
	Ymin = IRIT_INFNTY,
	Ymax = -IRIT_INFNTY,
	Zmin = IRIT_INFNTY,
	Zmax = -IRIT_INFNTY;
  
   TriangleList = HDSCreateTribaseList();
  
   for (Obj = PObjects; Obj != NULL; Obj = Obj -> Pnext) {
      if (IP_IS_POLY_OBJ(Obj) && IP_IS_POLYGON_OBJ(Obj)) {
	  ObjTri = GMConvertPolysToTriangles(Obj);
	  Nbelt++;
	  Nbpl = 0;
	  for (Pl = ObjTri -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	     Nbpl++;
	     Nbpt = 0;
	     /* Vertex List is circular. */

	     for (Ver = Pl -> PVertex; Nbpt < 3 ; Ver = Ver -> Pnext) {
	         /* Remembers the coords of the Vertex*/
	         x[Nbpt] = Ver -> Coord[0];
			 y[Nbpt] = Ver -> Coord[1];	    
			 z[Nbpt] = Ver -> Coord[2];
			 Normal[Nbpt][0]= Ver -> Normal[0];
			 Normal[Nbpt][1]= Ver -> Normal[1];
			 Normal[Nbpt][2]= Ver -> Normal[2];
		 /* Remembers the extremes coords. */
		 if (x[Nbpt] > Xmax)
		     Xmax = x[Nbpt];
		 if (x[Nbpt] < Xmin)
		     Xmin = x[Nbpt];   
		 if (y[Nbpt] > Ymax)
		     Ymax = y[Nbpt];
		 if (y[Nbpt] < Ymin)
		     Ymin = y[Nbpt];
		 if (z[Nbpt] > Zmax)
		     Zmax = z[Nbpt];
		 if (z[Nbpt] < Zmin)
		     Zmin = z[Nbpt];

		 Nbpt++;
	     }
	     /* Creates a new Triangle with our database. */
	     Tri = HDSCreateTribase(x[0], y[0], z[0], x[1], y[1], z[1],
				    x[2], y[2], z[2],
				    Normal[0], Normal[1], Normal[2]);   
	     HDSTribaseInsert(Tri, &TriangleList);
	  }
      }
   }

   /* This for more accuracy. */
   Xmin = Xmin - HDS_BBOX_GAP;
   Ymin = Ymin - HDS_BBOX_GAP;
   Zmin = Zmin - HDS_BBOX_GAP;
   Xmax = Xmax + HDS_BBOX_GAP;
   Ymax = Ymax + HDS_BBOX_GAP;
   Zmax = Zmax + HDS_BBOX_GAP;
   
   return HDSInitOctree(TriangleList, Xmin, Xmax, Ymin,
			Ymax, Zmin, Zmax, Depth);
}

/*****************************************************************************
* DESCRIPTION:							             M
*   Traverses the Vertex tree, updates the active List and conVerts it       M
* into an IPObjectStruct according to the new error threshold.		     M
*									     *
* PARAMETERS:								     M
*   Qt:        A pointer to the Vertex tree.				     M
*   Threshold: The error threshold.					     M
*									     *
* RETURN VALUE:							             M
*   IPObjectStruct *: the active List as an IRIT object.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   HDSCnvrtPObj2QTree, HDSTriBudget, HDSFreeQTree			     M
*									     *
* KEYWORDS:								     M
*    HDSThreshold							     M
*****************************************************************************/
IPObjectStruct *HDSThreshold(VoidPtr Qt, IrtRType Threshold)
{

    HDSOctreeStruct
        *HDSQt = (HDSOctreeStruct *) Qt;

    IrtRType
        MaxError = HDSQt -> Root -> Radius;

    HDSAdjustTree(HDSQt -> Root, HDSQt, Threshold * MaxError);

    return HDSConVertActiveList2PObj(HDSQt -> ActiveList);
}

/*****************************************************************************
* DESCRIPTION:							             M
*    Update the active list according to the new triangle budget, then	     M
* convert it into an IPObjectStruct.					     M
*									     *
* PARAMETERS:								     M
*   Qt:        The Vertex tree.						     M
*   TriBudget: The Triangle budget.					     M
*									     *
* RETURN VALUE:							             M
*   IPObjectStruct *:   The active list as an IRIT object.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   HDSThreshold, HDSCnvrtPObj2QTree, HDSFreeQTree			     M
*									     *
* KEYWORDS:								     M
*    HDSTriBudget							     M
*****************************************************************************/
IPObjectStruct *HDSTriBudget(VoidPtr Qt, int TriBudget)
{
    HDSOctreeStruct
        *HDSQt = (HDSOctreeStruct *) Qt;

    HDSBudgetSimplify(HDSQt, TriBudget);
    return HDSConVertActiveList2PObj(HDSQt -> ActiveList);
}

/*****************************************************************************
* DESCRIPTION:							             M
*   Free all the memory allocated to a particular octree.		     M
*									     *
* PARAMETERS:								     M
*   Qt:    A pointer to the octree.					     M
*									     *
* RETURN VALUE:							             M
*   void								     M
*                                                                            *
* SEE ALSO:                                                                  M
*   HDSThreshold, HDSTriBudget, HDSCnvrtPObj2QTree			     M
*									     *
* KEYWORDS:								     M
*   HDSFreeQTree							     M
*****************************************************************************/
void HDSFreeQTree(VoidPtr Qt)
{
    HDSOctreeStruct
        *HDSQt = (HDSOctreeStruct *) Qt;

    HDSOctreeRemove(HDSQt -> Root, HDSQt);
    HDSFreeList(HDSQt -> ActiveList);
    HDSFreeListbase(HDSQt -> TriangleList);
    HDSFreeTBQ(HDSQt -> Tbq);
    IritFree(HDSQt);
}
