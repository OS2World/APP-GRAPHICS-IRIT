/*****************************************************************************
*   "Irit" - the 3d (not only polygonal) solid modeller.		     *
*									     *
* Written by:  Gershon Elber				Ver 0.2, Mar. 1990   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
*  Module to handle adjacancies between polygons. Each edge has exactly two  *
* polygons which share it. An edge is implicitly defined by the VList - each *
* IPVertexStruct defines an edge with its succesor, and has a pointer to the *
* other polygons using that edge. Those pointers are our target in this      *
* module.								     *
*****************************************************************************/

#include <stdio.h>
#include <math.h>
#include "irit_sm.h"
#include "allocate.h"
#include "geom_lib.h"
#include "bool_loc.h"

#ifdef DEBUG
/* If the adjacencies should be printed to stdout. */
IRIT_SET_DEBUG_PARAMETER(_DebugAdj1, FALSE);
/* If the hash table content should be printed to stdout. */
IRIT_SET_DEBUG_PARAMETER(_DebugAdj2, FALSE);
/* If second attempt of adjs should be printed to stdout. */
IRIT_SET_DEBUG_PARAMETER(_DebugAdj3, FALSE);
IRIT_SET_DEBUG_PARAMETER(_DebugPrintAdj, FALSE);
#endif/* DEBUG */

#define BOOL_ALLOCATE_NUM     1000 /* Number of objects to allocate at once. */
#define BOOL_DIR_EPS	      0.99999	     /* Dot prod of same directions. */
#define BOOL_LINE_LINE_EPS    0.00001	      /* Distance between two edges. */

typedef struct HashTableEntry {
    int Key;
    IrtVecType Dir;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;
    struct HashTableEntry *Pnext;
} HashTableEntry;

typedef struct HashTableStruct {
    HashTableEntry **Entry;
} HashTableStruct;

IRIT_STATIC_DATA int
    GlblSrfUVBndryValid = FALSE,
    GlblHashTableSize = 10000,
    GlblHashTableSize2 = 5000,			       /* Half of the above. */
    GlblHashTableSize1 = 10001;			      /* One plus the above. */

IRIT_STATIC_DATA IrtRType
    GlblSrfUMin = 0.0,
    GlblSrfUMax = 0.0,
    GlblSrfVMin = 0.0,
    GlblSrfVMax = 0.0;

IRIT_STATIC_DATA HashTableEntry
    *HashTableEntryFreedList = NULL;

/* Prototypes of local function of adjacecies module: */
static HashTableEntry *BoolAllocHashTableEntry(void);
static void BoolFreeHashTableEntry(HashTableEntry *H);
static void InsertHashTable(HashTableStruct *HashTbl,
			    IPPolygonStruct *Pl,
			    IPVertexStruct *V);
static int EdgeKey(IPVertexStruct *V);
static int IsEdgesSameUVVals(IPVertexStruct *E1V1,
			     IPVertexStruct *E1V2,
			     IPVertexStruct *E2V1,
			     IPVertexStruct *E2V2);
static int IsEdgeOnSrfBndry(IPVertexStruct *V1, IPVertexStruct *V2);
static HashTableEntry *FindMatchEdge(HashTableStruct *HashTbl,
				     int EntryNum,
				     HashTableEntry *PHash);
static int SameEdges(IPVertexStruct *V1E1,
		     IPVertexStruct *V2E1,
		     IPVertexStruct *V1E2,
		     IPVertexStruct *V2E2);
static void InsertSecondHashTable(HashTableStruct *SecondHashTbl,
				  HashTableEntry *PHash);
static int SecondEdgeKey(IPVertexStruct *V, IrtVecType Dir);
static HashTableEntry *FindSecondMatchEdge(HashTableStruct *SecondHashTbl,
					   int EntryNum,
					   HashTableEntry *PHash,
					   int *HashInMatch);
static void BoolDisjPropagateAdjacency(IPPolygonStruct *Pl, int Index);

#ifdef DEBUG
static void BoolDebugPrintAdjacencies(IPObjectStruct *PObj);
#endif /* DEBUG */

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Routine to generate adjacencies to the given object.                     M
*   Note an edge might be only partially adjacent to another edge, and a     M
* second attempt is made to find (again only part of - see below) them. Any  M
* case, FALSE will be returned as there is no way we can say the object is   M
* perfectly closed!							     M
*   This is the only routine to generate the adjacencies of a geometric      M
* object. These adjacencies are needed for the Boolean operations on them.   M
*   Algorithm: for each edge, for each polygon in the object, the edges are  M
* sorted according to the key defined by EdgeKey routine (sort in hash tbl). M
* A second path on the table is made to match common keys edges and set the  M
* pointers from one to another. Note that each edge is common to exactly 2   M
* faces if it is internal, or exactly 1 face if it is on the border (if the  M
* object is open).							     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj: The polygonal object to compute the adjacency information for.     M
*                                                                            *
* RETURN VALUE:                                                              M
*   int: TRUE if all adjacencies were resolved, or the object is completely  M
*        closed.                                                             M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolGenAdjSetSrfBoundaries, BoolGetAdjEdge, BoolClnAdjacencies           M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolGenAdjacencies, adjacency, topology                                  M
*****************************************************************************/
int BoolGenAdjacencies(IPObjectStruct *PObj)
{
    int i, IsOpenObject;
    HashTableStruct *HashTbl, *SecondHashTbl;
    HashTableEntry *PHash, *PHashMatch;
    IPPolygonStruct *Pl;
    IPVertexStruct *V;

    if (!IP_IS_POLY_OBJ(PObj))
	BOOL_FATAL_ERROR(BOOL_ERR_NO_POLY_OBJ);
    if (IP_IS_POLYLINE_OBJ(PObj))
	return TRUE;				 /* No adj. in polyline obj. */

    IsOpenObject = FALSE;		    /* "Default" is closed object... */

    /* Esimate num of vertices as 3 time num of polygons + some slack. */
    GlblHashTableSize = IRIT_MAX(1000, 5 * IPPolyListLen(PObj -> U.Pl));
    GlblHashTableSize1 = GlblHashTableSize + 1;
    GlblHashTableSize2 = (GlblHashTableSize >> 1);

    /* Prepare hash tables (for first and second attempts) and clear them.   */
    HashTbl = (HashTableStruct *) IritMalloc(sizeof(HashTableStruct));
    HashTbl -> Entry = (HashTableEntry **)
		     IritMalloc(GlblHashTableSize1 * sizeof(HashTableEntry *));
    for (i = 0; i < GlblHashTableSize1; i++)
	HashTbl -> Entry[i] = NULL;

    SecondHashTbl = (HashTableStruct *) IritMalloc(sizeof(HashTableStruct));
    SecondHashTbl -> Entry = (HashTableEntry **)
		     IritMalloc(GlblHashTableSize1 * sizeof(HashTableEntry *));
    for (i = 0; i < GlblHashTableSize1; i++)
	SecondHashTbl -> Entry[i] = NULL;

    /* Step one - enter all the edges into the hash table: */
    Pl = PObj -> U.Pl;
    while (Pl) {
	V = Pl -> PVertex;
	do {
	    V -> PAdj = NULL;
	    InsertHashTable(HashTbl, Pl, V); /* Insert the edge V..V->Pnext. */
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
	if (V == NULL)
	    BOOL_FATAL_ERROR(BOOL_ERR_CIRC_VRTX_LST);

	Pl = Pl -> Pnext;
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugAdj2) {
        int j = 0;

        IRIT_INFO_MSG_PRINTF("Hash Table content (Size = %d):\n",
			     GlblHashTableSize);
	for (i = 0; i < GlblHashTableSize; i++) {
	    PHash = HashTbl -> Entry[i];

	    while (PHash) {
	        j++;

		IRIT_INFO_MSG_PRINTF("Edge (%d): %7.4lg %7.4lg %7.4lg :: %7.4lg %7.4lg %7.4lg (%d)\n", i,
		       PHash -> V -> Coord[0],
		       PHash -> V -> Coord[1],
		       PHash -> V -> Coord[2],
		       PHash -> V -> Pnext -> Coord[0],
		       PHash -> V -> Pnext -> Coord[1],
		       PHash -> V -> Pnext -> Coord[2],
		       PHash -> Key);
 
		PHash = PHash -> Pnext;
	    }
	}

	IRIT_INFO_MSG_PRINTF("Hash table has %d edges\n", j);
    }
#endif /* DEBUG */

    /* Step two - scans all the entries and look for the matching edge.      */
    for (i = 0; i < GlblHashTableSize; i++) {
	while (HashTbl -> Entry[i] != NULL) {
	    PHash = HashTbl -> Entry[i]; /* Remove one edge from hash table. */
	    HashTbl -> Entry[i] = HashTbl -> Entry[i] -> Pnext;

	    /* Find matching edge (if perfect match - exactly the same edge) */
	    /* Otherwise put the edge in SecondHashTbl.			     */
	    if ((PHashMatch = FindMatchEdge(HashTbl, i, PHash)) == NULL) {
		InsertSecondHashTable(SecondHashTbl, PHash);
		IsOpenObject = TRUE;
	    }
	    else {
#		ifdef DEBUG
	        IRIT_IF_DEBUG_ON_PARAMETER(_DebugAdj1) {
		    /* Switch the pointers of the edges themselves.*/
		    PHash -> V -> PAdj = (IPPolygonStruct *) PHashMatch -> V;
		    PHashMatch -> V -> PAdj = (IPPolygonStruct *) PHash -> V;
		}
		else {
		    /* Otherwise switch pointers of the edges polygons */
		    PHash -> V -> PAdj = PHashMatch -> Pl;
		    PHashMatch -> V -> PAdj = PHash -> Pl;
		}
#		else
		    /* Otherwise switch pointers of the edges polygons */
		    PHash -> V -> PAdj = PHashMatch -> Pl;
		    PHashMatch -> V -> PAdj = PHash -> Pl;
#		endif /* DEBUG */

		BoolFreeHashTableEntry(PHash);
		BoolFreeHashTableEntry(PHashMatch);
	    }
	}
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugAdj1) {
        IRIT_INFO_MSG_PRINTF("Adjacencies for object %s (found to be open = %d)\n",
			     IP_GET_OBJ_NAME(PObj), IsOpenObject);
	Pl = PObj -> U.Pl;
	/* Note adjacency in DEBUG_ADJ1 is other edge, not other polygon. */
	i = 0;
	while (Pl) {
	    V = Pl -> PVertex;
	    do {
	        if (V -> PAdj == NULL) {
		    IRIT_INFO_MSG_PRINTF("Edge  %10lf %10lf %10lf :: %10lf %10lf %10lf\n",
			   V -> Coord[0], V -> Coord[1], V -> Coord[2],
			   V -> Pnext -> Coord[0], V -> Pnext -> Coord[1],
			   V -> Pnext -> Coord[2]);
		    i++;
		}

		V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);
	    Pl = Pl -> Pnext;
	}
	IRIT_INFO_MSG_PRINTF("\nNo Match on %d edges\n", i);
    }
#endif /* DEBUG */

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugAdj1) {
        IRIT_INFO_MSG("Hash Table content left after all full matches were deleted:\n");
	for (i = 0; i < GlblHashTableSize; i++) {
	    PHash = SecondHashTbl -> Entry[i];
	    if (PHash)
	        IRIT_INFO_MSG_PRINTF("\nHashTable entry %d:\n", i);
	    while (PHash) {
	        IRIT_INFO_MSG_PRINTF("Edge  %10lf %10lf %10lf :: %10lf %10lf %10lf\n",
		       PHash -> V -> Coord[0],
		       PHash -> V -> Coord[1],
		       PHash -> V -> Coord[2],
		       PHash -> V -> Pnext -> Coord[0],
		       PHash -> V -> Pnext -> Coord[1],
		       PHash -> V -> Pnext -> Coord[2]);
		PHash = PHash -> Pnext;
	    }
	}
    }
#endif /* DEBUG */

    /* Time to activate the second attempt - scan SecondHashTable for edges  */
    /* partially adjacent to each other.				     */
    for (i = 0; i < GlblHashTableSize; i++) {
	while (SecondHashTbl -> Entry[i] != NULL) {
	    int HashInMatch;

	    /* Remove one edge from table. */
	    IRIT_LIST_POP(PHash, SecondHashTbl -> Entry[i]);

	    /* Find matching edge (collinear and share some portion). */
	    if ((PHashMatch = FindSecondMatchEdge(SecondHashTbl, i, PHash,
						  &HashInMatch)) == NULL) {
		BoolFreeHashTableEntry(PHash);
	    }
	    else {
		/* Exchange adjacency pointers of the edges polygons. */
		PHash -> V -> PAdj = PHashMatch -> Pl;
		PHashMatch -> V -> PAdj = PHash -> Pl;

		if (HashInMatch) {
		    BoolFreeHashTableEntry(PHash);
		    InsertSecondHashTable(SecondHashTbl, PHashMatch);
		}
		else {
		    BoolFreeHashTableEntry(PHashMatch);
		    InsertSecondHashTable(SecondHashTbl, PHash);
		}
	    }
	}
    }

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugAdj3) {
        IRIT_INFO_MSG_PRINTF("Adjacencies for object %s - second attempt (found to be open = %d)\n",
			     IP_GET_OBJ_NAME(PObj), IsOpenObject);
	Pl = PObj -> U.Pl;
	/* Note adjacency in DEBUG_ADJ1 is other edge, not other polygon. */
	i = 0;
	while (Pl) {
	    V = Pl -> PVertex;
	    do {
	        if (V -> PAdj == NULL) {
		    IrtVecType Dir;
		    int Key = SecondEdgeKey(V, Dir);

		    IRIT_INFO_MSG_PRINTF("Edge (%d) %6.4lf %6.4lf %6.4lf : %6.4lf %6.4lf %6.4lf | %6.4lf %6.4lf %6.4lf\n",
			   Key, Dir[0], Dir[1], Dir[2],
			   V -> Coord[0], V -> Coord[1], V -> Coord[2],
			   V -> Pnext -> Coord[0], V -> Pnext -> Coord[1],
			   V -> Pnext -> Coord[2]);
		    i++;
		}

		V = V -> Pnext;
	    }
	    while (V != NULL && V != Pl -> PVertex);
	    Pl = Pl -> Pnext;
	}
	IRIT_INFO_MSG_PRINTF("\nNo Match on %d edges\n", i);
    }
#endif /* DEBUG */

    IritFree(HashTbl -> Entry);
    IritFree(HashTbl);
    IritFree(SecondHashTbl -> Entry);
    IritFree(SecondHashTbl);

#ifdef DEBUG
    IRIT_IF_DEBUG_ON_PARAMETER(_DebugPrintAdj)
        BoolDebugPrintAdjacencies(PObj);
#endif /* DEBUG */

    return !IsOpenObject;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Sets boundary definition using UV coordinates of original surface.       M
*   If the original surface was closed, polygons along the seam are usually  M
* not detected as boundary since the other side of the seam shares the edge. M
*   If the prescribed domain is valid (non zero) edges found on a surface    M
* boundary are always considered boundary, even if the surface is closed at  M
* the (shared) seam.							     M
*                                                                            *
* PARAMETERS:                                                                M
*   UMin, VMin:   Minimal UV value of the domain, zeros to disable.          M
*   UMax, VMax:   Maximal UV value of the domain, zeros to disable.          M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:    Old values of surface domain validation option.		     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolGenAdjacencies                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolGenAdjSetSrfBoundaries                                               M
*****************************************************************************/
int BoolGenAdjSetSrfBoundaries(IrtRType UMin,
			       IrtRType VMin,
			       IrtRType UMax,
			       IrtRType VMax)
{
    int OldSrfUVBndryValid = GlblSrfUVBndryValid;

    GlblSrfUVBndryValid = !(IRIT_APX_EQ(UMin, 0.0) &&
			    IRIT_APX_EQ(UMax, 0.0) &&
			    IRIT_APX_EQ(VMin, 0.0) &&
			    IRIT_APX_EQ(VMax, 0.0));

    if (GlblSrfUVBndryValid) {
	GlblSrfUMin = UMin;
	GlblSrfVMin = VMin;
	GlblSrfUMax = UMax;
	GlblSrfVMax = VMax;
    }
    else {
	GlblSrfUMin = GlblSrfVMin = GlblSrfUMax = GlblSrfVMax = IRIT_INFNTY;
    }

    return OldSrfUVBndryValid;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Allocates one HashTableEntry Structure. 			             *
*                                                                            *
* PARAMETERS:                                                                *
*   None								     *
*                                                                            *
* RETURN VALUE:                                                              *
*   HashTableEntry *:  A new allocated HashTableEntry structure.             *
*****************************************************************************/
static HashTableEntry *BoolAllocHashTableEntry(void)
{
    HashTableEntry *p;

    if (HashTableEntryFreedList != NULL) {
	p = HashTableEntryFreedList;
	HashTableEntryFreedList = HashTableEntryFreedList -> Pnext;
    }
    else {
	IRIT_STATIC_DATA int 
	    ComputedAllocateNumObj,
	    AllocateNumObj = BOOL_ALLOCATE_NUM;
	HashTableEntry *H;

#ifdef DEBUG_IP_MALLOC
	H = (HashTableEntry *) IritMalloc(sizeof(HashTableEntry));
#else
	int i;

	/* Allocate AllocateNumObj objects, returns first one as new   */
	/* and chain together the rest of them into the free list.     */
	if (!ComputedAllocateNumObj)
	    AllocateNumObj = getenv("IRIT_MALLOC") ? 1 : BOOL_ALLOCATE_NUM;

	if ((H = (HashTableEntry *) IritMalloc(sizeof(HashTableEntry)
					       * AllocateNumObj)) != NULL) {
	    for (i = 1; i < AllocateNumObj - 1; i++)
		H[i].Pnext = &H[i + 1];
	    H[AllocateNumObj - 1].Pnext = NULL;
	    if (AllocateNumObj > 1)
		HashTableEntryFreedList = &H[1];
	}
#endif /* DEBUG_IP_MALLOC */
	p = H;
    }

    IRIT_ZAP_MEM(p, sizeof(HashTableEntry));

    return p;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
* Free one HashTableEntry Structure.                                         *
*                                                                            *
* PARAMETERS:                                                                *
*   H:   To free.                            	                             *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolFreeHashTableEntry(HashTableEntry *H)
{
    if (H != NULL) {
#ifdef DEBUG_IP_MALLOC
	IritFree(H);
#else
	/* Chain this new object to the global freed HashTableEntry list: */
	H -> Pnext = HashTableEntryFreedList;
	HashTableEntryFreedList = H;
#endif /* DEBUG_IP_MALLOC */
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates a key (integer!) from the given vertex V (in polygon Pl) and   *
* insert that in the hash table:					     *
*                                                                            *
* PARAMETERS:                                                                *
*   HashTbl:  To be used.                                                    *
*   Pl:       Polygon containing vertex.                                     *
*   V:        Vertex to insert into hash table.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertHashTable(HashTableStruct *HashTbl,
			    IPPolygonStruct *Pl,
			    IPVertexStruct *V)
{
    int Key;
    HashTableEntry
        *PHash = BoolAllocHashTableEntry();

    PHash -> Pl = Pl;
    PHash -> V = V;
    PHash -> Key = Key = EdgeKey(V);
    PHash -> Pnext = HashTbl -> Entry[Key];
    HashTbl -> Entry[Key] = PHash;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This routine evaluates a key for a given edge. In order the try to make  *
* them unique as possible, the point is projected on a "random" vector.      *
*   The key itself is the average of the two vertices keys.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:   To computed key for.                                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: The resulting key.                                                  *
*****************************************************************************/
static int EdgeKey(IPVertexStruct *V)
{
    int Key;
    IrtRType RKey1, RKey2;

    RKey1 = 61137 * V -> Coord[0] +
            31171 * V -> Coord[1] +
	    47219 * V -> Coord[2];

    V = V -> Pnext;

    RKey2 = 61137 * V -> Coord[0] +
            31171 * V -> Coord[1] +
	    47219 * V -> Coord[2];

    Key = ((int) (RKey1 + RKey2)) % GlblHashTableSize2 + GlblHashTableSize2;

    return IRIT_BOUND(Key, 0, GlblHashTableSize - 1);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Searches the hash table for matching with a given edge pointed by PHash. *
* PHash was extracted from the hash table in entry EntryNum, so the match    *
* is probably in the same entry. If it is not, it must be (if there is one!) *
* in EntryNum+1 as we scans the entries in order and (EntryNum-1) is empty.  *
*   Note that idealy the match was in EntryNum, but because of real number   *
* errors there is a small chance it will be in its neibours: EntryNum +/- 1. *
*                                                                            *
* PARAMETERS:                                                                *
*   HashTbl:   To search.                                                    *
*   EntryNum:  The probably contains the matching to PHash.                  *
*   PHash:     To find a match for.                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   HashTableEntry *:  The matched entry.                                    *
*****************************************************************************/
static HashTableEntry *FindMatchEdge(HashTableStruct *HashTbl,
				     int EntryNum,
				     HashTableEntry *PHash)
{
    int i, j;
    HashTableEntry *PMatch,
	*PLast = NULL;

    for (j = EntryNum; j <= EntryNum + 1; j++) {
	i = IRIT_BOUND(j, 0, GlblHashTableSize - 1);
	PMatch = HashTbl -> Entry[i];
	while (PMatch) {
	    if (SameEdges(PHash -> V,
			  PHash -> V -> Pnext,
			  PMatch -> V,
			  PMatch -> V -> Pnext)) {
		/* Delete the matched edge from hash table, and return it: */
		if (PMatch == HashTbl -> Entry[i])
		    HashTbl -> Entry[i] = HashTbl -> Entry[i] -> Pnext;
		else
		    PLast -> Pnext = PLast -> Pnext -> Pnext;
		return PMatch;
	    }
	    PLast = PMatch;
	    PMatch = PMatch -> Pnext;
	}
    }

    return NULL;				  /* No match for this one ! */
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if the given two edges share the same UV coordinates.               *
*                                                                            *
* PARAMETERS:                                                                *
*   E1V1, E1V2:   Vertices of first edges.				     *
*   E2V1, E2V2:   Vertices of second edges.				     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if sharing the same UV vals, FALSE otherwise.               *
*****************************************************************************/
static int IsEdgesSameUVVals(IPVertexStruct *E1V1,
			     IPVertexStruct *E1V2,
			     IPVertexStruct *E2V1,
			     IPVertexStruct *E2V2)
{
    float 
	*E1UV1 = AttrGetUVAttrib(E1V1 -> Attr, "uvvals"),
	*E1UV2 = AttrGetUVAttrib(E1V2 -> Attr, "uvvals"),
	*E2UV1 = AttrGetUVAttrib(E2V1 -> Attr, "uvvals"),
	*E2UV2 = AttrGetUVAttrib(E2V2 -> Attr, "uvvals");

    return (IRIT_APX_EQ(E1UV1[0], E2UV1[0]) &&
	    IRIT_APX_EQ(E1UV1[1], E2UV1[1]) &&
	    IRIT_APX_EQ(E1UV2[0], E2UV2[0]) &&
	    IRIT_APX_EQ(E1UV2[1], E2UV2[1])) ||
	   (IRIT_APX_EQ(E1UV1[0], E2UV2[0]) &&
	    IRIT_APX_EQ(E1UV1[1], E2UV2[1]) &&
	    IRIT_APX_EQ(E1UV2[0], E2UV1[0]) &&
	    IRIT_APX_EQ(E1UV2[1], E2UV1[1]));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Test if the given edge is on the boundary of the original surface using  *
* the edge's UV coordinates.                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   V1,V2:   The two vertices forming the edge.                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:    TRUE if a surface boundary, FALSE otherwise.                     *
*****************************************************************************/
static int IsEdgeOnSrfBndry(IPVertexStruct *V1, IPVertexStruct *V2)
{
    float 
        *UV1 = AttrGetUVAttrib(V1 -> Attr, "uvvals"),
	*UV2 = AttrGetUVAttrib(V2 -> Attr, "uvvals");

    return (UV1 != NULL &&
	    UV2 != NULL &&
	    ((IRIT_APX_EQ(UV1[0], GlblSrfUMin) &&
	      IRIT_APX_EQ(UV2[0], GlblSrfUMin)) ||
	     (IRIT_APX_EQ(UV1[0], GlblSrfUMax) &&
	      IRIT_APX_EQ(UV2[0], GlblSrfUMax)) ||
	     (IRIT_APX_EQ(UV1[1], GlblSrfVMin) &&
	      IRIT_APX_EQ(UV2[1], GlblSrfVMin)) ||
	     (IRIT_APX_EQ(UV1[1], GlblSrfVMax) &&
	      IRIT_APX_EQ(UV2[1], GlblSrfVMax))));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Compere 2 edges - if same up to BOOL_IRIT_EPS (see BOOL_IRIT_PT_APX_EQ). *
* The two vetrices of each edge are given, but no order on them is assumed   *
*                                                                            *
* PARAMETERS:                                                                *
*   V1E1, V2E1: Two vertices of first edge.                                  *
*   V1E2, V2E2: Two vertices of second edge.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   inr: TRUE is found the same, FALSE otherwise.                            *
*****************************************************************************/
static int SameEdges(IPVertexStruct *V1E1,
		     IPVertexStruct *V2E1,
		     IPVertexStruct *V1E2,
		     IPVertexStruct *V2E2)
{
    IrtRType
	*V1E1Pt = V1E1 -> Coord,
	*V2E1Pt = V2E1 -> Coord,
	*V1E2Pt = V1E2 -> Coord,
	*V2E2Pt = V2E2 -> Coord;

    if (GlblSrfUVBndryValid && !IsEdgesSameUVVals(V1E1, V1E2, V2E1, V2E2))
        return FALSE;

    return (BOOL_IRIT_PT_APX_EQ(V1E1Pt, V1E2Pt) &&
	    BOOL_IRIT_PT_APX_EQ(V2E1Pt, V2E2Pt)) ||
	   (BOOL_IRIT_PT_APX_EQ(V1E1Pt, V2E2Pt) &&
	    BOOL_IRIT_PT_APX_EQ(V2E1Pt, V1E2Pt));
}

/*****************************************************************************
* AUXILIARY:								     *
*   Everything from this point handles the second attempt - match edges      *
* which are not complete match - cases which one edge is only part of its    *
* adjacent one. We trap only cases which the two edges has common vertex. If *
* the two edges has no common vertex (i.e. one is totally in the other) we   *
* still misses that. You are invited to improve that. Any case this one will *
* have influence in extremely rare cases (The booleans will usually          *
* propagate the information using the common vertex edges).	             *
*   Note, the obvious, that if one edge is adjacent to few edges, only one   *
* (arbitrarily) will result in the match, and the other will result as NULL. *
*****************************************************************************/

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Evaluates two keys (integer!) from a given edge in HashTableEntry.       *
*   This time keys are of the vertices themselves (see SecondEdgeKet rtn).   *
*   Note each HashTableEntry hold the key of the other entry this time...    *
*                                                                            *
* PARAMETERS:                                                                *
*   SecondHashTbl:   See above.                                              *
*   PHash:           See above.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InsertSecondHashTable(HashTableStruct *SecondHashTbl,
				  HashTableEntry *PHash)
{
    int Key;

    /* And insert the edge as at Key (using given HashTableEntry PHash): */
    PHash -> Key = Key = SecondEdgeKey(PHash -> V, PHash -> Dir);
    PHash -> Pnext = SecondHashTbl -> Entry[Key];
    SecondHashTbl -> Entry[Key] = PHash;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   This routine evaluates key for the given edge - the edge's direction and *
* project on a vector.							     *
*                                                                            *
* PARAMETERS:                                                                *
*   V:        To compute keys for it and its next vertex, forming the edge.  *
*   Dir:      The normalized direction of the edge is returned here.	     *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: The resulting key.                                                  *
*****************************************************************************/
static int SecondEdgeKey(IPVertexStruct *V, IrtVecType Dir)
{
    int Key;
    IrtRType Size;

    IRIT_PT_SUB(Dir, V -> Pnext -> Coord, V -> Coord);

    Size = IRIT_VEC_LENGTH(Dir);
    if (Size < IRIT_PT_NORMALIZE_ZERO)
        Size = 1.0;
    else
        Size = 1.0 / Size;
    IRIT_VEC_SCALE(Dir, Size);

    Key = (int) (IRIT_FABS(0.244 * Dir[0] + 0.468 * Dir[1] + 0.288 * Dir[2])
							 * GlblHashTableSize);

    Key = IRIT_BOUND(Key, 0, GlblHashTableSize - 1);

    return Key;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Search The hash table for matching with the given edge pointed by PHash, *
* as in the second attempt: the keys used here are of the edges direction.   *
*   We search for collinearity of the two edges, which if both match,        *
* confirm at list partial adjacency between the two edges.		     *
*                                                                            *
* PARAMETERS:                                                                *
*   SecondHashTbl:    Containing the edge database.                          *
*   EntryNum:         Where to look at the database.                         *
*   PHash:            To search for a match                                  *
*   HashInMatch:      TRUE if PHash entry is subset of returned PMatch,      *
*		      FALSE otherwise.					     *
*                                                                            *
* RETURN VALUE:                                                              *
*   HashTableEntry:   Found match, if any.				     *
*                     Found match is removed from SecondHashTbl, if found.   *
*****************************************************************************/
static HashTableEntry *FindSecondMatchEdge(HashTableStruct *SecondHashTbl,
					   int EntryNum,
					   HashTableEntry *PHash,
					   int *HashInMatch)
{
    int i;
    HashTableEntry *PMatch,
	*PLast = NULL;

    *HashInMatch = FALSE;

    if (GlblSrfUVBndryValid &&
	IsEdgeOnSrfBndry(PHash -> V, PHash -> V -> Pnext))
	return NULL;		 /* This match is on the boundary - ignore. */

    for (i = EntryNum; i <= EntryNum + 1; i++) {
	PMatch = SecondHashTbl -> Entry[i];   /* It must be here if exists. */
	while (PMatch) {
	    IrtRType
		DotProd = IRIT_DOT_PROD(PHash -> Dir, PMatch -> Dir);

	    if (GlblSrfUVBndryValid &&
		IsEdgeOnSrfBndry(PMatch -> V, PMatch -> V -> Pnext)) {
		/* This match is on the boundary - ignore. */
	    }
	    else if ((DotProd > BOOL_DIR_EPS || DotProd < -BOOL_DIR_EPS) &&
		     GMDistPointLine(PHash -> V -> Coord, PMatch -> V -> Coord,
				     PMatch -> Dir) < BOOL_LINE_LINE_EPS) {
		/* The two edges are collinear - make sure there is overlap */
		IrtRType Len1, Len2, Len3;
		IrtVecType Dir1, Dir2, Dir3;

		IRIT_PT_SUB(Dir1,
		            PHash -> V -> Pnext -> Coord, PHash -> V -> Coord);
		IRIT_PT_SUB(Dir2,
		            PMatch -> V -> Coord, PHash -> V -> Coord);
		IRIT_PT_SUB(Dir3,
		            PMatch -> V -> Pnext -> Coord, PHash -> V -> Coord);
		Len1 = IRIT_DOT_PROD(Dir1, PHash -> Dir);
		assert(Len1 > 0.0);
		Len2 = IRIT_DOT_PROD(Dir2, PHash -> Dir);
		Len3 = IRIT_DOT_PROD(Dir3, PHash -> Dir);
		if (Len3 < Len2)
		    IRIT_SWAP(IrtRType, Len2, Len3);

		if (Len1 <= Len2 || IRIT_APX_EQ(Len1, Len2) ||
		    Len3 < 0.0 || IRIT_APX_EQ(Len3, 0.0)) {
		    /* The two segments do not overlap. */
		}
		else {
		    /* Delete matched edge from hash table, its complement  */
		    /* with the second key and return a pointer to it:      */
		    if (PMatch == SecondHashTbl -> Entry[i])
		        SecondHashTbl -> Entry[i] =
			    SecondHashTbl -> Entry[i] -> Pnext;
		    else
		        PLast -> Pnext = PLast -> Pnext -> Pnext;

		    *HashInMatch = (Len2 < 0.0 || IRIT_APX_EQ(Len2, 0.0)) &&
				   (Len3 > Len1 || IRIT_APX_EQ(Len3, Len1));

		    return PMatch;
		}
	    }
	    PLast = PMatch;
	    PMatch = PMatch -> Pnext;
	}
    }

    return NULL; /* No match for this one ! */
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Clean the adjacency pointers in the given polygonal model.               M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:   Polygon object to clean adjacency information.                   M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolGenAdjacencies                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolClnAdjacencies                                                       M
*****************************************************************************/
void BoolClnAdjacencies(IPObjectStruct *PObj)
{
    IPPolygonStruct *Pl;

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *V = Pl -> PVertex;

	BOOL_DISJ_RESET(Pl);

	do {
	    V -> PAdj = NULL;

	    V = V -> Pnext;
	}
	while (V != Pl -> PVertex && V != NULL);
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Mark polygons in the given object based on their association with        M
* different disjoint object parts.                                           M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    To anaylize for different disjoint parts.                       M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:     Number of disjoint parts.                                       M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolGetDisjointPart                                                      M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolMarkDisjointParts                                                    M
*****************************************************************************/
int BoolMarkDisjointParts(IPObjectStruct *PObj)
{
    int DisjIndex = 0;
    IPPolygonStruct *Pl;

    /* Reset the disjoint parts' indices. */
    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	BOOL_DISJ_RESET(Pl);
    }

    /* Traverse the object and look for unvisited parts to look for adj. */
    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
	if (BOOL_DISJ_GET_INDEX(Pl) == 0) {
	    BoolDisjPropagateAdjacency(Pl, ++DisjIndex);
	}
    }

    return DisjIndex;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Recursively visit and propagate this parts' index.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Pl:      Seed to propagate the part index.				     *
*   Index:   Part index.						     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void BoolDisjPropagateAdjacency(IPPolygonStruct *Pl, int Index)
{
    int StackPointer = 0;
    IPVertexStruct *V;
    IPPolygonStruct
	**AdjStack = (IPPolygonStruct **) IritMalloc(sizeof(IPPolygonStruct *)
					                    * ADJ_STACK_SIZE);

    AdjStack[StackPointer++] = Pl;

    while (StackPointer > 0) {
        /* Pop top of stack and mark it as belong to this part. */
        Pl = AdjStack[--StackPointer];
	BOOL_DISJ_SET_INDEX(Pl, Index);

	/* Push the adjacent polygons. */
	V = Pl -> PVertex;
	do {
	    if (V -> PAdj != NULL) {
	        if (BOOL_DISJ_GET_INDEX(V -> PAdj) == 0)
		    AdjStack[StackPointer++] = V -> PAdj;
		else if (BOOL_DISJ_GET_INDEX(V -> PAdj) != Index)
		    BOOL_FATAL_ERROR(BOOL_ERR_DISJ_PROP_ERR);
	    }

	    if (StackPointer >= ADJ_STACK_SIZE)
	        BOOL_FATAL_ERROR(BOOL_ERR_ADJ_STACK_OF);

	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }

    IritFree(AdjStack);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Get the disjoint part number Index from object PObj.                     M
*                                                                            *
* PARAMETERS:                                                                M
*   PObj:    Object to extract part number Index from.			     M
*   Index:   Index of part to fetch from PObj.				     M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPPolygonStruct *:   Extract polygonal list with disjoint number Index.  M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolMarkDisjointParts                                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolGetDisjointPart                                                      M
*****************************************************************************/
IPPolygonStruct *BoolGetDisjointPart(IPObjectStruct *PObj, int Index)
{
    IPPolygonStruct *Pl,
	*PlList = NULL;

    for (Pl = PObj -> U.Pl; Pl != NULL; Pl = Pl -> Pnext) {
        IPPolygonStruct *PlNew;

	if (BOOL_DISJ_GET_INDEX(Pl) == Index) {
	    PlNew = IPCopyPolygon(Pl);
	    IRIT_LIST_PUSH(PlNew, PlList);
	}
    }

    return PlList;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Given a polygonal mesh with adjacent information, find the adjacent edge M
* (VAdj, VAdj->Pnext) that is adjacent to edge (V, V->Pnext), if any.        M
*                                                                            *
* PARAMETERS:                                                                M
*   V:   Vertex of a mesh defining edgee (V, V->Pnext) to extract its        M
*        adjacent edge, if any.                                              M
*                                                                            *
* RETURN VALUE:                                                              M
*   IPVertexStruct *: First vertex of adjacent edge to edge (V, V->Pnext) or M
*		      NULL if none.                                          M
*                                                                            *
* SEE ALSO:                                                                  M
*   BoolGenAdjacencies                                                       M
*                                                                            *
* KEYWORDS:                                                                  M
*   BoolGetAdjEdge                                                           M
*****************************************************************************/
IPVertexStruct *BoolGetAdjEdge(IPVertexStruct *V)
{
    IrtRType Len1, Len2, Len;
    IrtVecType Dir, Vec;
    IPPolygonStruct
	*PAdj = V -> PAdj;
    IPVertexStruct *VAdj;

    if (PAdj == NULL || V -> Pnext == NULL)
	return NULL;

    IRIT_VEC_SUB(Dir, V -> Pnext -> Coord, V -> Coord);
    if ((Len = IRIT_VEC_LENGTH(Dir)) < IRIT_UEPS)
        return FALSE;
    Len1 = 1.0 / Len;
    IRIT_VEC_SCALE(Dir, Len1);

    VAdj = PAdj -> PVertex;
    do {
        if (GMDistPointLine(VAdj -> Coord, V -> Coord, Dir) < IRIT_EPS &&
	    VAdj -> Pnext != NULL &&
	    GMDistPointLine(VAdj -> Pnext -> Coord,
			    V -> Coord, Dir) < IRIT_EPS) {
	    /* Edge (VAdj, VAdj->Pnext) is on the infinite line through     */
	    /* edge (V, V->Pnext) - examine if they overlap as well.        */
	    IRIT_VEC_SUB(Vec, VAdj -> Coord, V -> Coord);
	    Len1 = IRIT_DOT_PROD(Vec, Dir);
	    IRIT_VEC_SUB(Vec, VAdj -> Pnext -> Coord, V -> Coord);
	    Len2 = IRIT_DOT_PROD(Vec, Dir);
	    if (IRIT_MIN(Len1, Len2) < Len - IRIT_EPS &&
		IRIT_MAX(Len1, Len2) > IRIT_EPS)
		return VAdj;                              /* Found overlap. */
	}

        VAdj = VAdj -> Pnext;
    }
    while (VAdj != NULL && VAdj != PAdj -> PVertex);

    BOOL_FATAL_ERROR(BOOL_ERR_DISJ_PROP_ERR);
    
    return NULL;
}

#ifdef DEBUG

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Prints the adjacency information of an object.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   PObj:      To debug print its adjacency information.                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*                                                                            *
* KEYWORDS:                                                                  *
*   BoolDebugPrintAdjacencies                                                *
*****************************************************************************/
static void BoolDebugPrintAdjacencies(IPObjectStruct *PObj)
{
    IPPolygonStruct
	*Pl = PObj -> U.Pl;

    IRIT_INFO_MSG_PRINTF("Object %s\n", IP_GET_OBJ_NAME(PObj));

    for ( ; Pl != NULL; Pl = Pl -> Pnext) {
	IPVertexStruct
	    *V = Pl -> PVertex;

	IRIT_INFO_MSG_PRINTF("Polygon %08x\n", Pl);
	do {
	    IRIT_INFO_MSG_PRINTF("\tVertex [PAdj %08x] %f %f %f\n",
		    V -> PAdj, V -> Coord[0], V -> Coord[1], V -> Coord[2]);
	    V = V -> Pnext;
	}
	while (V != NULL && V != Pl -> PVertex);
    }
}

#endif /* DEBUG */
