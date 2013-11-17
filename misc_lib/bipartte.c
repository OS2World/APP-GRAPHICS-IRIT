/*****************************************************************************
* bipartte.c - minimal-cost complete matching in a bi-partite graph.         *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by Tomer Vromen, August 2010                                       *
*****************************************************************************/

#ifdef DEBUG_COMPILE_ALONE
#include <stdlib.h>
#define IrtRType double
#define IritMalloc malloc
#define IritFree free
#define IRIT_SWAP(type, x, y)	{ type _temp = (x); x = (y); y = _temp; }

typedef struct IritBiPrWeightedMatchStruct {
    int m1, m2, m3;
} IritBiPrWeightedMatchStruct;
#else
#include "irit_sm.h"
#include "misc_lib.h"
#endif /* DEBUG_COMPILE_ALONE */

/*#define BIPARTITE_NO_SHUFFLE        Uncomment for "deterministic" version */
/*#define BIPARTITE_LESS_SHUFFLE            uncomment for "less" randomness */


static int MiscBiPrBasicPreprocessing(const IrtRType **Weight,
				      IritBiPrWeightedMatchStruct *Match,
				      IrtRType *u,
				      IrtRType *v,
				      int *MatchReverse,
				      int *PermutationArray,
				      int n);
static int MiscBiPrAllocate(IrtRType **u,
			    IrtRType **v,
			    int **MatchReverse,
			    int **PathIsShortestU,
			    int **PathIsShortestV,
			    IrtRType **PathCost,
			    int **PathPred,
			    int **PermutationArray,
			    int n);
static void MiscBiPrFree(IrtRType *u,
			 IrtRType *v,
			 int *MatchReverse,
			 int *PathIsShortestU,
			 int *PathIsShortestV,
			 IrtRType *PathCost,
			 int *PathPred,
			 int *PermutationArray);
static void MiscBiPrPermutationArray(int *PermutationArray, int n);

/******************************************************************************
* DESCRIPTION:                                                                M
* Implementation of Shortest-Path variation of the Hungarian algorithm, from  M
* "Assignment Problems" by R. Burkard, M. Dell'Amico, S. Martello             M
* Algorithm 4.10 Hungarian_SP, page 97.    Complexity: O(n^3)                 M
* The sacnning order of the graph's vertices is chosen at random.             M
*                                                                             *
* PARAMETERS:                                                                 M
*   Weight:   Cost matrix of size n*n, passed as an array of size n holding   M
*             pointers to arrays of size n.                                   M
*             Weight[i][j] is the cost of the edge between vertices i and j.  M
*             Negative entry denotes that there is no edge (== edge cost is   M
*             infinite).                                                      M
*   Match:    Array of size n allocated in advance.                           M
*             Returns with the optimal matching found.  For every 0 <= i < n  M
*                                 Match[i].m1 = i                             V
*                                 Match[i].m2 = vertex that is matched to i   V
*                                 Match[i].m3 = unused                        V
*   n:        Number of vertices in each side of the bi-partite graph.        M
*                                                                             M
* RETURN VALUE:	                                                              M
*   int: 0 if an optimal match was found - in such case the array Match is    M
*        filled with the optimal matching found. Negative value if there is   M
*        no match in the graph - in such case Match may be undefined.         M
*                                                                             *
* KEYWORDS:                                                                   M
*   MiscBiPrWeightedMatchBipartite                                            M
******************************************************************************/
int MiscBiPrWeightedMatchBipartite(const IrtRType **Weight,
				   IritBiPrWeightedMatchStruct *Match,
				   int n)
{
    /* For U vertices, we use Match to indicate the current matching         */
    /* (negative if unmatched).						     */
    int *MatchReverse,             /* For V vertices. negative if unmatched. */
        root,                                        /* Current vertex in U. */
        i, j,           /* These refer to vertices in U and V respectively.  */
        /* Flag if shortest path was found for vertices in U,V resp: */
        *PathIsShortestU, *PathIsShortestV,
        /* Cost for shortest path found so far, for vertices in V:	     */
        /* (negative if no path yet).					     */
        *PathPred,         /* Predecessor in the shortest path found so far. */
        *PermutationArray,	/* Generates a more "random-looking" result. */
        sink, JMin;
    IrtRType *u, *v, *PathCost, delta;

    /* Initialize */
    if (MiscBiPrAllocate(&u, &v, &MatchReverse,
			 &PathIsShortestU, &PathIsShortestV,
			 &PathCost, &PathPred, &PermutationArray, n) < 0)
        return -1;	/* Allocation error. */

    if (MiscBiPrBasicPreprocessing(Weight, Match, u, v, MatchReverse,
				   PermutationArray, n) < 0) {
        MiscBiPrFree(u, v, MatchReverse, PathIsShortestU, PathIsShortestV,
		     PathCost, PathPred, PermutationArray);
        return -2;	/* No match. */
    }

    /* Main loop. continue while there is an unmatched vertex in U. */
    root = 0;
    while (root < n && Match[root].m2 >= 0) {
        root++;
    }

    while (root < n) {
        /* Find shortest alternating path in the incremental graph that      */
        /* starts in root and ends with an unmatched vertex in V.            */
        for (i = 0; i < n; i++) {
            PathIsShortestU[i] = 0;
        }
        for (j = 0; j < n; j++) {
            PathIsShortestV[j] = 0;
            PathCost[j] = -1;
        }
        sink = -1;
        delta = 0;
        i = root;

        while (sink < 0) {    /* Sink not found yet. */
            PathIsShortestU[i] = 1;
            /* Dijkstra update. */
            for (j = 0; j < n; j++) {
                if (Weight[i][j] >= 0 && !PathIsShortestV[j]) {
                    if (PathCost[j] < 0 ||
			delta + Weight[i][j] - u[i] - v[j] < PathCost[j]) {
                        PathPred[j] = i;
                        PathCost[j] = delta + Weight[i][j] - u[i] - v[j];
                    }
                }
            }
            /* Find a reachable j in V such that		             */
	    /* PathIsShortestV[j] == 0 and PathCost[j] is minimal.           */
	    /*   To get a random scanning order, generate a new permutation  */
	    /* array and filter the iterator through it.		     */
#ifndef BIPARTITE_NO_SHUFFLE
#ifndef BIPARTITE_LESS_SHUFFLE
            MiscBiPrPermutationArray(PermutationArray, n);
#endif /* BIPARTITE_LESS_SHUFFLE */
#endif /* BIPARTITE_NO_SHUFFLE */
            JMin = PermutationArray[0];
            for (j = 1; j < n; j++) {
                if (PathCost[PermutationArray[j]] >= 0
			&& !PathIsShortestV[PermutationArray[j]]) {
                    if (PathIsShortestV[JMin] ||
			PathCost[JMin] < 0 ||
			PathCost[PermutationArray[j]] < PathCost[JMin]) {
                        JMin = PermutationArray[j];
                    }
                }
            }
            if (PathIsShortestV[JMin] || PathCost[JMin] < 0) {
	        MiscBiPrFree(u, v, MatchReverse,
			     PathIsShortestU, PathIsShortestV,
			     PathCost, PathPred, PermutationArray);
                return -3;	/* No match. */
            }
            j = JMin;
            PathIsShortestV[j] = 1;
            delta = PathCost[j];
            if (MatchReverse[j] < 0) {    /* Unmatched. */
                sink = j;
            }
	    else {
                i = MatchReverse[j];
            }
        }

        /* Update feasible solution of the dual problem. */
        u[root] += delta;
        for (i = 0; i < n; i++) {
            if (PathIsShortestU[i] && i != root) {
                u[i] += delta - PathCost[Match[i].m2];
            }
        }
        for (j = 0; j < n; j++) {
            if (PathIsShortestV[j]) {
                v[j] -= delta - PathCost[j];
            }
        }

        /* Augment the found path (switch matchings along the path). */
        j = sink;
        do {
            i = PathPred[j];
            MatchReverse[j] = i;
            IRIT_SWAP(int, Match[i].m2, j);
        }
	while (i != root);

        /* Find next unmatched vertex in U. */
        while (root < n && Match[root].m2 >= 0) {
            root++;
        }
    }

    MiscBiPrFree(u, v, MatchReverse, PathIsShortestU, PathIsShortestV,
		 PathCost, PathPred, PermutationArray);

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Implementation of initializtion algorithm from                            *
* "Assignment Problems", R. Burkard, M. Dell'Amico, S. Martello               *
* Algorithm 4.1, page 76.                                                     *
*   Initializes the parameters to create a feasible labeling (u,v) and a      *
* partial feasible matching. This is done to speed-up the matching            *
* algorithm, and is generally unnecessary (u and v could have been            *
* initialized to 0's and Match to an empty matching).                         *
*   Scans vertices in the order defined by PermutationArray.                  *
*                                                                             *
* PARAMETERS:                                                                 *
*   Weight:  Cost matrix of size n*n, passed as an array of size n holding    *
*            pointers to arrays of size n.                                    *
*              Weight[i][j] is the cost of the edge between vertices i and j. *
*            Negative entry denotes that there is no edge (== edge cost is    *
*            infinite).                                                       *
*   Match:   Array of size n allocated in advance.                            *
*            Returns with a partial feasible matching found.                  *
*            For every 0 <= i < n:                                            *
*                          Match[i].m1 = i                                    *
*                          Match[i].m2 = vertex that is matched to i          *
*                          Match[i].m3 = unused                               *
*   v:       Pointer to an array of size n. returns with a feasible           *
*            labeling for vertices in U.                                      *
*   v:       Pointer to an array of size n. returns with a feasible           *
*            labeling for vertices in V.                                      *
*   MatchReverse: Pointer to array of size n. returns with the reverse        *
*                 matching according to Match: if Match[i]==j then            *
*                 MatchReverse[j]==i.                                         *
*   PermutationArray: Pointer to array of size n. Returns with a permutation  *
*                     array (each i, 0 <= i < n, appears once in the array.)  *
*   n:       Number of vertices in each side of the bi-partite graph.         *
*                                                                             *
* RETURN VALUE:                                                               *
*   int: 0 on success, in which case Match is filled with a partial feasible  *
*        matching and u,v are filled with feasible labels. negative value if  *
*        there is no matching in the graph.                                   *
******************************************************************************/
static int MiscBiPrBasicPreprocessing(const IrtRType **Weight,
				      IritBiPrWeightedMatchStruct *Match,
				      IrtRType *u,
				      IrtRType *v,
				      int *MatchReverse,
				      int *PermutationArray,
				      int n)
{
    int i, j;
    IrtRType min;

    /* Initial feasible u. */
    for (i = 0; i < n; i++) {
        min = Weight[i][0];
        for (j = 1; j < n; j++) {
            if (Weight[i][j] >= 0) {
                if (min < 0 || Weight[i][j] < min) {
                    min = Weight[i][j];
                }
            }
        }
        if (min < 0) {    /* Negative values represent infinity. */
            return -1;
        }
        u[i] = min;
    }

    /* Initial feasible v. */
    for (j = 0; j < n; j++) {
        min = Weight[0][j] - u[0];
        for (i = 1; i < n; i++) {
            if (Weight[i][j] >= 0) {
                if (min < 0 || Weight[i][j] - u[i] < min) {
                    min = Weight[i][j] - u[i];
                }
            }
        }
        if (min < 0) {    /* Negative values represent infinity. */
            return -1;
        }
        v[j] = min;
    }

#ifdef BIPARTITE_NO_SHUFFLE
    for (i = 0; i < n; i++) {
    	PermutationArray[i] = i;
    }
#endif /* BIPARTITE_NO_SHUFFLE */

#ifdef BIPARTITE_LESS_SHUFFLE
    MiscBiPrPermutationArray(PermutationArray, n);
#endif /* BIPARTITE_LESS_SHUFFLE */

    /* Partial feasible matching.  */
    for (j = 0; j < n; j++) {
        MatchReverse[j] = -1;    /* Unmatched. */
    }
    for (i = 0; i < n; i++) {
	Match[i].m1 = i;
	Match[i].m2 = -1;    /* Unmatched. */
	Match[i].m3 = 0;     /* Unused. */
#ifndef BIPARTITE_NO_SHUFFLE
#ifndef BIPARTITE_LESS_SHUFFLE
	MiscBiPrPermutationArray(PermutationArray, n);
#endif /* BIPARTITE_LESS_SHUFFLE */
#endif /* BIPARTITE_NO_SHUFFLE */

        for (j = 0; j < n; j++) {
            /* scan vertices in the order defined by PermutationArray */
            if (MatchReverse[PermutationArray[j]] < 0 &&
		Weight[i][PermutationArray[j]]
			               - u[i] - v[PermutationArray[j]] == 0) {
                Match[i].m2 = PermutationArray[j];
                MatchReverse[PermutationArray[j]] = i;
                break;
            }
        }
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Allocates arrays of size n for all the pointers.                          *
*                                                                             *
* PARAMETERS:                                                                 *
*   u,                                                                        *
*   v,                                                                        *
*   MatchReverse,                                                             *
*   PathIsShortestU,                                                          *
*   PathIsShortestV,                                                          *
*   PathCost,                                                                 *
*   PathPred,                                                                 *
*   PermutationArray:                                                         *
*          All of the above are (uninitialized) pointers that will be         *
*          initialized to point to arrays of size n.                          *
*   n:     The size of the arrays to be allocated.                            *
*                                                                             *
* RETURN VALUE:                                                               *
*   int: 0 on success, in which case all the pointers are filled with the     *
*        addresses of the allocated arrays. Negative value in case of         *
*        failure.                                                             *
******************************************************************************/
static int MiscBiPrAllocate(IrtRType **u,
			    IrtRType **v,
			    int **MatchReverse,
			    int **PathIsShortestU,
			    int **PathIsShortestV,
			    IrtRType **PathCost,
			    int **PathPred,
			    int **PermutationArray,
			    int n)
{
    int flag = 0;

    *u = *v = NULL;
    *MatchReverse = NULL;
    *PathIsShortestU = *PathIsShortestV = NULL;
    *PathCost = NULL;
    *PathPred = NULL;
    *PermutationArray = NULL;

    if ((*u = (IrtRType *) IritMalloc(n * sizeof(IrtRType))) == NULL ||
        (*v = (IrtRType *) IritMalloc(n * sizeof(IrtRType))) == NULL ||
        (*MatchReverse = (int *) IritMalloc(n * sizeof(int))) == NULL ||
        (*PathIsShortestU = (int *) IritMalloc(n * sizeof(int))) == NULL ||
        (*PathIsShortestV = (int *) IritMalloc(n * sizeof(int))) == NULL ||
        (*PathCost = (IrtRType *) IritMalloc(n * sizeof(IrtRType))) == NULL ||
        (*PathPred = (int *) IritMalloc(n * sizeof(int))) == NULL ||
	(*PermutationArray = (int *) IritMalloc(n * sizeof(int))) == NULL)
        flag = 1;

    if (flag) {
        MiscBiPrFree(*u, *v, *MatchReverse, *PathIsShortestU, *PathIsShortestV,
		     *PathCost, *PathPred, *PermutationArray);
        return -1;
    }

    return 0;
}

/******************************************************************************
* DESCRIPTION:                                                                *
*   Frees arrays of size n of all the pointers.                               *
*                                                                             *
* PARAMETERS:                                                                 *
*   u,                                                                        *
*   v,                                                                        *
*   MatchReverse,                                                             *
*   PathIsShortestU,                                                          *
*   PathIsShortestV,                                                          *
*   PathCost,                                                                 *
*   PathPred,                                                                 *
*   PermutationArray:                                                         *
*           All of the above are pointers to be freed.                        *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscBiPrFree(IrtRType *u,
			 IrtRType *v,
			 int *MatchReverse,
			 int *PathIsShortestU,
			 int *PathIsShortestV,
			 IrtRType *PathCost,
			 int *PathPred,
			 int *PermutationArray)
{
    if (u != NULL)
        IritFree(u);
    if (v != NULL)
        IritFree(v);
    if (MatchReverse != NULL)
        IritFree(MatchReverse);
    if (PathIsShortestU != NULL)
        IritFree(PathIsShortestU);
    if (PathIsShortestV != NULL)
        IritFree(PathIsShortestV);
    if (PathCost != NULL)
        IritFree(PathCost);
    if (PathPred != NULL)
        IritFree(PathPred);
    if (PermutationArray != NULL)
	IritFree(PermutationArray);
}


/******************************************************************************
* DESCRIPTION:                                                                *
*   Generates a permutation array of size n.                                  *
*   Implements the Fisher-Yates algorithm.                                    *
*                                                                             *
* PARAMETERS:                                                                 *
*   PermutationArray: pointer to array                                        *
*   n: size of array                                                          *
*                                                                             *
* RETURN VALUE:                                                               *
*   void                                                                      *
******************************************************************************/
static void MiscBiPrPermutationArray(int *PermutationArray, int n)
{
    int i, j;

    PermutationArray[0] = 0;
    IritRandomInit(30101960);

    for (i = 1; i < n; i++) {
        j = (int) (IritRandom(0, 30101960)) % (i+1);
	PermutationArray[i] = PermutationArray[j];
	PermutationArray[j] = i;
    }
}
