/*****************************************************************************
* Module to read OBJ files into IRIT data.                                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Nadav Shragai                             Ver 1.0, July 2009  *
*****************************************************************************/
#include "objirlst.h"

IRIT_STATIC_DATA const int
    IP_O2I_LIST_INIT_SIZE = 10;
IRIT_GLOBAL_DATA const char
    IP_O2I_WHITE_SPACE[] = "\t ";

static int ElementsSize(IPO2IListStruct *List);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Return the required size for one elements in the given list.             * 
*                                                                            *
* PARAMETERS:                                                                *
*   List: A IPO2IListStruct to calculate the required size for one of its    *
*         elements.                                                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: The required size for one element in the given list.                *
*****************************************************************************/
static int ElementsSize(IPO2IListStruct *List)
{
    int Size;

    switch (List -> Type) {
        case IP_O2I_I:
            Size = sizeof(int);
            break;
        case IP_O2I_IVEC_3: 
            Size = sizeof(IPO2IInt3Type);
            break;
        case IP_O2I_R:
            Size = sizeof(IrtRType);
            break;
        case IP_O2I_RPT_3:
            Size = sizeof(IPO2IReal3Type);
            break;
        case IP_O2I_RPT_4:
            Size = sizeof(IPO2IReal4Type);
            break;
        case IP_O2I_GROUPS:
        case IP_O2I_CURV2:
        case IP_O2I_STRING:
            Size = sizeof(void *);
            break;
        case IP_O2I_MTL:
            Size = sizeof(IPO2IMtlStruct);
            break;
        default:
	    Size = -1;
	    assert(0);
	    break;
    }

    return Size;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Adding Str2 to the end of Str1, puting both of them in a new allocated   *
*   memory. Setting Str1 to the new string and freeing the previous memory   *
*   of Str1.                                                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   Str1:  The first string. If it's null or *Str1 is NULL, just duplicate   *
*          Str2.                                                             *
*   Str2:  The second string. Added after the first string.                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   The combined string.                                                     *
*****************************************************************************/
char *_IPO2IStrcat(char **Str1, const char* Str2)
{
    char *Temp;

    if ((*Str1 == NULL) || (Str1 == NULL))
        Temp = IritStrdup(Str2);
    else {
        Temp = IritMalloc((unsigned int) ((strlen(*Str1) + strlen(Str2) + 1) * 
					                      sizeof(char)));
        sprintf(Temp, "%s%s", *Str1, Str2);
        IritFree(*Str1);
    }
    *Str1 = Temp;
    return *Str1;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Trim white spaces from both sides of the string.                         *
*                                                                            *
* PARAMETERS:                                                                *
*   Str:   The string to trim.                                               *
*                                                                            *
* RETURN VALUE:                                                              *
*   The trimmed string (in new allocated memory) or NULL if Str is NULL.     *
*****************************************************************************/
char *_IPO2ITrimWhiteSpaces(const char *Str)
{
    char *Temp;

    if (Str == NULL)
        return NULL;

    /* Trimming from the left. */
    for (; strchr(IP_O2I_WHITE_SPACE,*Str) != NULL && *Str != '\0'; Str++);
    Temp = IritStrdup(Str);

    /* Trimming from the right. */
    if (Temp[0] != '\0') {
        char *End;

        for (End = Temp + strlen(Temp) - 1; strchr(IP_O2I_WHITE_SPACE, *End) 
            != NULL && End != Temp; End--);
        End[1] = '\0';
    }
    return Temp;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Add the given element to the end of the list. Reallocate the list memory *
*   if needed.                                                               *
*                                                                            *
* PARAMETERS:                                                                *
*   List: A IPO2IListStruct to add the element to.                           *
*   Element: An element to put into the list.                                *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void _IPO2IAddElement(IPO2IListStruct *List, void *Element)
{
    if (List -> Len == List -> Size) {
        if (List -> U.p == NULL) {
            List -> U.p = 
                IritMalloc(IP_O2I_LIST_INIT_SIZE * ElementsSize(List));
            List -> Size = IP_O2I_LIST_INIT_SIZE;
        }
        else {
            List -> U.p = IritRealloc(List -> U.p, 
                List -> Size * ElementsSize(List), 
                List -> Size * 2 * ElementsSize(List));
            List -> Size = List -> Size * 2;
        }
    }
    List -> Len++;
    _IPO2IPutElement(List, Element, List -> Len -1, FALSE);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Put the given element at the given location in the list.                 *
*                                                                            *
* PARAMETERS:                                                                *
*   List: A IPO2IListStruct to add the element to.                           *
*   Element: An element to put into the list.                                *
*   Index: The location in the list to put the element.                      *
*   FreeElementMemory: TRUE - Free the memory associated with the element    *
*                             which is currently in the given Index.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   int: FALSE - The given index is larger than the size of the list.        *
*        TRUE  - Otherwise.                                                  *
*****************************************************************************/
int _IPO2IPutElement(IPO2IListStruct *List, 
		     void *Element, 
		     int Index, 
		     int FreeElementMemory)
{
    if (List -> Len < Index)
        return FALSE;

    if (FreeElementMemory)
        _IPO2IFreeElement(List, Index);

    switch (List -> Type) {
        case IP_O2I_I:
            List -> U.i[Index] = (*(int*) Element);
            break;
        case IP_O2I_IVEC_3:
            IRIT_GEN_COPY((List -> U.IVec)[Index], Element, 
                sizeof(IPO2IInt3Type));  
            break;
        case IP_O2I_R:
            List -> U.r[Index] = (*(IrtRType*) Element);
            break;
        case IP_O2I_RPT_3:
            IRIT_GEN_COPY((List -> U.RPts3)[Index], Element, 
                sizeof(IPO2IReal3Type));  
            break;
        case IP_O2I_RPT_4:
            IRIT_GEN_COPY((List -> U.RPts4)[Index], Element, 
                sizeof(IPO2IReal4Type));  
            break;
        case IP_O2I_GROUPS:
        case IP_O2I_CURV2:
            (List -> U.p)[Index] = Element;
            break;
        case IP_O2I_STRING:
            (List -> U.p)[Index] = _IPO2ITrimWhiteSpaces(Element);
            break;
        case IP_O2I_MTL:
            (List -> U.Mtl)[Index] = (*(IPO2IMtlStruct*) Element);
            break;
    }
    return TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free the memory associated with the element at the given Index.          *
*                                                                            *
* PARAMETERS:                                                                *
*   List:    A List to free its element (If needed. Depends on the type).    *
*   Element: An element to free its memory. If it's NULL do nothing.         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void _IPO2IFreeElement(IPO2IListStruct *List, int Index)
{
    switch (List -> Type) {
        case IP_O2I_I:
        case IP_O2I_IVEC_3: 
        case IP_O2I_R:
        case IP_O2I_RPT_3:
        case IP_O2I_RPT_4:
            break;
        case IP_O2I_GROUPS:
        case IP_O2I_CURV2:
        case IP_O2I_STRING:
            IritFree((List -> U.p)[Index]);
            break;
        case IP_O2I_MTL:
            AttrFreeAttributes(&(List -> U.Mtl)[Index].Attr);
            IritFree((List -> U.Mtl)[Index].Name);
            break;
    }    
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Free the memory used by this List.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   List:         A list to free its memory.                                 *
*   FreeElements: TRUE - Free the memory associated with the elements in the *
*                        list before freeing the list's memory.              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void _IPO2IFreeList(IPO2IListStruct *List, int FreeElements)
{
    if (FreeElements)
        _IPO2IClearList(List, TRUE);
    IritFree(List -> U.p);
    _IPO2IInitList(List, List -> Type);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Clear the list without freeing its memory (Setting its elements number   *
*   to 0).                                                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   List:         A list to clear.                                           *
*   FreeElements: TRUE - Free the memory associated with the elements in the *
*                        list before clearing list.                          *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void _IPO2IClearList(IPO2IListStruct *List, int FreeElements) 
{
    if (FreeElements) {
        switch (List -> Type) {
            int i;

            case IP_O2I_I:
            case IP_O2I_IVEC_3: 
            case IP_O2I_R:
            case IP_O2I_RPT_3:
            case IP_O2I_RPT_4:
                break;
            case IP_O2I_GROUPS:
            case IP_O2I_CURV2:
            case IP_O2I_STRING:
            case IP_O2I_MTL:
                for(i =0; i <= List -> Len - 1; i++)
                    _IPO2IFreeElement(List, i);
        }
    }
    List -> Len = 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initialise the list to empty list.                                       *
*                                                                            *
* PARAMETERS:                                                                *
*   List: A list to initialise.                                              *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
void _IPO2IInitList(IPO2IListStruct *List, IPO2IListTypesType Type)
{
    List -> U.p = NULL;
    List -> Size = 0;
    List -> Len = 0;
    List -> Curr = -1;
    List -> Type = Type;
}
