/*****************************************************************************
* Module to read OBJ files into IRIT data.                                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Nadav Shragai                             Ver 1.0, July 2009  *
*****************************************************************************/
#ifndef	OBJ_IRIT_LIST_H
#define	OBJ_IRIT_LIST_H

#include "irit_sm.h"
#include "iritprsr.h"

extern const char IP_O2I_WHITE_SPACE[];

typedef int IPO2IInt3Type[3];
typedef IrtRType IPO2IReal3Type[3];
typedef IrtRType IPO2IReal4Type[4];
typedef struct IPO2IMtlStruct {
    char *Name;
    IPAttributeStruct *Attr;
} IPO2IMtlStruct;

typedef enum IPO2IListTypesType {
    IP_O2I_I,
    IP_O2I_IVEC_3,
    IP_O2I_R,
    IP_O2I_RPT_3,
    IP_O2I_RPT_4,
    IP_O2I_GROUPS,                           
    IP_O2I_CURV2,
    IP_O2I_STRING,
    IP_O2I_MTL
} IPO2IListTypesType;

typedef struct IPO2IListStruct {

    union {
        void             **p;
        /* Allocating memory to the elements themself and copy their value   */
        /* in _AddElement.                                                   */
        int              *i;
        IPO2IInt3Type    *IVec;
        IrtRType         *r; 
        IPO2IReal3Type   *RPts3;
        IPO2IReal4Type   *RPts4;
        IPO2IMtlStruct   *Mtl;
        /* Allocating memory to the pointers to the elements and copy only   */
        /* the pointers in _AddElement.                                      */
        IPObjectStruct   **Groups;
        CagdCrvStruct    **Curv2s;
        char **Strings;
    } U;
    int Len;                              /* Number of elements in the list. */
    int Size;                  /* Number of allocated elements in the array. */
    int Curr;/* A pointer to location in the list. For free use of the user. */
    IPO2IListTypesType Type;                        /* The type of the list. */
} IPO2IListStruct;

char *_IPO2IStrcat(char **Str1, const char* Str2);
char *_IPO2ITrimWhiteSpaces(const char *Str);
void _IPO2IAddElement(IPO2IListStruct *List, void *Element);
int _IPO2IPutElement(IPO2IListStruct *List, 
                      void *Element, 
                      int Index, 
                      int FreeElement);
void _IPO2IFreeElement(IPO2IListStruct *List, int Index);
void _IPO2IFreeList(IPO2IListStruct *List, int FreeElements);
void _IPO2IClearList(IPO2IListStruct *List, int FreeElements);
void _IPO2IInitList(IPO2IListStruct *List, IPO2IListTypesType Type);

#endif /* OBJ_IRIT_LIST_H */
