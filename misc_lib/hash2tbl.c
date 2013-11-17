/*****************************************************************************
* hash2tbl.c - yet another implementation of a hash table.                   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by Eyal Guthmann, December, 2009                                   *
*****************************************************************************/

#include <stdlib.h>
#include "misc_loc.h"

typedef struct MiscHashTableStruct {
    void **Table;
    long *ElementAuxData;
    unsigned long Size;
    unsigned long NumHashed;
    int KeySizeBits;
    MiscHashFuncType HashFunc;
    MiscHashCopyFuncType CopyFunc;
    MiscHashFreeFuncType FreeFunc;
    MiscHashCompFuncType CompFunc;

    MiscListPtrType ElementList;      /* Used for freeing elements quickly. */

} MiscHashTableStruct;

static void *MiscHashDummyCopy(void *ToCopy, unsigned long Dummy);
static long MiscHashFindElementAux(MiscHashPtrType Hash, 
                                   void *Elem, 
                                   unsigned long SizeInByte);

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Does nothing.                                                            *
*                                                                            *
*                                                                            *
* PARAMETERS:                                                                *
*   ToCopy: Will be returned.                                                *
*   Dummy:  does nothing.                                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void *: ToCopy                                                           *
*****************************************************************************/
static void *MiscHashDummyCopy(void *ToCopy, unsigned long Dummy)
{
    return ToCopy;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*    Creates a new Hash Table.                                               M
*                                                                            *
* PARAMETERS:                                                                M
*   Size:       The size of the hash table (the range of the hash function   M
*               should be compatible).                                       M
*   HashFunc:   The function used for hashing (the range should be compatibleM
*               with HashSize). Receives 2 parameters to make it easy to hashM
*               arrays.                                                      M
*   CopyFunc:   The function that will be used to copy elements (for example M
*               when hashing a new element).                                 M
*   FreeFunc:   The function that will be used to free elements (for example M
*               when freeing the hash table).                                M
*   CompFunc:   The function that will be used to compare elements (for      M
*               example when looking for an element in the hash table).      M
*                                                                            *
* RETURN VALUE:                                                              M
*   MiscHashPtrType:   The new hash table if successful, of NULL if failed.  M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscHashNewHash                                                          M
*****************************************************************************/
MiscHashPtrType MiscHashNewHash(unsigned long Size,
				MiscHashFuncType HashFunc,
				MiscHashCopyFuncType CopyFunc,
				MiscHashFreeFuncType FreeFunc,
				MiscHashCompFuncType CompFunc)
{
    unsigned long i, Temp, KeySizeBits;
    MiscHashPtrType Ret;

    if (Size == 0)
        return NULL;

    /* Calculates the number of bits in the key. */
    Temp = Size;
    KeySizeBits = -1;

    while (Temp != 0UL) {
        Temp >>= 1;
        ++KeySizeBits;
    }

    Ret = IritMalloc(sizeof(struct MiscHashTableStruct));
    
    /* Elements are copied when hashed and freed via this list. */
    Ret -> ElementList = MiscListNewEmptyList(MiscHashDummyCopy, FreeFunc,
					      CompFunc);
    
    Ret -> Size = Size;
    Ret -> NumHashed = 0;
    Ret -> KeySizeBits = KeySizeBits;
    Ret -> HashFunc = HashFunc;
    Ret -> CopyFunc = CopyFunc;
    Ret -> FreeFunc = FreeFunc;
    Ret -> CompFunc = CompFunc;
    
    Ret -> Table = IritMalloc(sizeof(void*) * Ret -> Size);
    Ret -> ElementAuxData  = IritMalloc(sizeof(long) * Ret -> Size);

    for (i = 0; i < Ret -> Size; i++) {
        Ret -> Table[i] = NULL;
    }

    return Ret;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Hash the given element and insert it into the table.                     M
*   The added element is the returned value from the CopyFunc given to       M
* MiscHashNewHash. More than one identical element can exist in the table.   M
* The element's auxilary data is initialized to 0.                           M
*                                                                            *
* PARAMETERS:                                                                M
*   Hash:       The hash table.                                              M
*   Elem:       The element to hash.                                         M
*   SizeInByte: The second parameter to pass to the hash function.           M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        -1 if unable to add, 0 otherwise.                            M
*                                                                            *
* KEYWORDS:                                                                  M
*    MiscHashAddElement                                                      M
*****************************************************************************/
int MiscHashAddElement(MiscHashPtrType Hash, 
                       void *Elem, 
                       unsigned long SizeInByte)
{
    unsigned long HashEntry;

    if (Hash == NULL || Elem == NULL || Hash -> NumHashed >= (Hash -> Size-1))
        return -1;

    HashEntry = Hash -> HashFunc(Elem, SizeInByte, Hash -> KeySizeBits);
    if (HashEntry >= Hash -> Size)
        return -1;


    while (Hash -> Table[HashEntry] != NULL) {
        HashEntry = (HashEntry + 1) % Hash -> Size;
    }

    Hash -> Table[HashEntry] = Hash -> CopyFunc(Elem, SizeInByte);
    if (Hash -> Table[HashEntry] == NULL)
        return -1;
    
    if (MiscListAddElement(Hash -> ElementList, Hash -> Table[HashEntry],
			   SizeInByte) == -1) {
        Hash -> Table[HashEntry] = NULL;
        return -1;
    }

    Hash -> ElementAuxData[HashEntry] = 0;

    Hash -> NumHashed++;
    return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Looks for the given element in the hash table (using the CompFunc        *
*   given to MiscHashNewHash).                                               *
*   The Element and SizeInByte will be passed to the hash function.          *
*                                                                            *
* PARAMETERS:                                                                *
*   Hash:       The hash table.                                              *
*   Elem:       The element to hash.                                         *
*   SizeInByte: The second parameter to pass to the hash function. If the    *
*               second parameter is not equal to the one used to hash the    *
*               element, the element may not be found even if it is in the   *
*               hash table.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   long:       Entry in the table if found, -1 otherwise.                   *
*                                                                            *
* KEYWORDS:                                                                  *
*   MiscHashFindElementAux, MiscHashFindElement                              *
*****************************************************************************/
static long MiscHashFindElementAux(MiscHashPtrType Hash, 
                                   void *Elem, 
                                   unsigned long SizeInByte)
{
    unsigned long HashEntry;

    if (Hash == NULL || Elem == NULL)
        return -1;

    HashEntry = Hash -> HashFunc(Elem, SizeInByte, Hash -> KeySizeBits);
    if (HashEntry >= Hash -> Size)
        return -1;

    while(Hash -> Table[HashEntry] != NULL) {
        if (Hash -> CompFunc(Hash -> Table[HashEntry], Elem, SizeInByte) == 0)
            return HashEntry;

        HashEntry = (HashEntry + 1) % Hash -> Size;
    }

    return -1;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Looks for the given element in the hash table (using the CompFunc        M
*   given to MiscHashNewHash).                                               M
*   The Element and SizeInByte will be passed to the hash function.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Hash:       The hash table.                                              M
*   Elem:       The element to hash.                                         M
*   SizeInByte: The second parameter to pass to the hash function. If the    M
*               second parameter is not equal to the one used to hash the    M
*               element, the element may not be found even if it is in the   M
*               hash table.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:        0 if found, -1 otherwise.                                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscHashFindElement                                                      M
*****************************************************************************/
int MiscHashFindElement(MiscHashPtrType Hash, 
                        void *Elem, 
                        unsigned long SizeInByte)
{
    if (MiscHashFindElementAux(Hash, Elem, SizeInByte) == -1)
        return -1;
    else
        return 0;
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Find the first occurence of Elem in the hash table (using the CompFunc   M
*   given to MiscHashNewHash) and return a pointer to its auxilary data      M
*   (The auxilary data may be changed without influencing the hash table).   M
*   The Element and SizeInByte will be passed to the hash function.          M
*                                                                            *
* PARAMETERS:                                                                M
*   Hash:       The hash table.                                              M
*   Elem:       The element to retrieve its auxilary data.                   M
*   SizeInByte: The second parameter to pass to the hash function. If the    M
*               second parameter is not equal to the one used to hash the    M
*               element, the element may not be found even if it is in the   M
*               hash table.                                                  M
*                                                                            *
* RETURN VALUE:                                                              M
*   long *:     Pointer to Elem's auxilary data. NULL if Elem doesn't exist  M
*               in the hash table.                                           M
*                                                                            *
* KEYWORDS:                                                                  M
*   MiscHashGetElementAuxData                                                M
*****************************************************************************/
long *MiscHashGetElementAuxData(MiscHashPtrType Hash, 
				void *Elem, 
				unsigned long SizeInByte)
{
    long HashEntry = MiscHashFindElementAux(Hash, Elem, SizeInByte);

    if (HashEntry == -1)
        return NULL;
    return &Hash -> ElementAuxData[HashEntry];
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Frees the given hash table.                                              M
*                                                                            M
*                                                                            *
* PARAMETERS:                                                                M
*   Hash: The hash table to free.                                            M
*                                                                            *
* RETURN VALUE:                                                              M
*   void                                                                     M
*                                                                            *
* KEYWORDS:                                                                  M
*    MiscHashFreeHash                                                        M
*****************************************************************************/
void MiscHashFreeHash(MiscHashPtrType Hash)
{
    IritFree(Hash -> Table);
    IritFree(Hash -> ElementAuxData);
    MiscListFreeList(Hash -> ElementList);
    IritFree(Hash);
}
