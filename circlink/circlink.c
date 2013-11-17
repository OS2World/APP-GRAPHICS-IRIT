/*****************************************************************************
*   A dummy function to force linkage of circularities in libraries.	     *
* This module will be linked as an object file and hence always loaded.      *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
*****************************************************************************/

#include "irit_sm.h"
#include "cagd_lib.h"
#include "symb_lib.h"
#include "iritprsr.h"
#include "ip_cnvrt.h"
#include "user_lib.h"
#include "grap_lib.h"
#include "mdl_lib.h"

#ifdef __WINCE__
int __GSHandlerCheck; /* Dummy missing variable in WIn CE/mobile linking... */
#endif /* __WINCE__ */

/*****************************************************************************
* AUXILIARY:								     *
* Dummy function that is never called. Just force linkage of functions.      *
*****************************************************************************/
void _DummyCircularLinkResolver(void)
{
    CagdVType Center;
    IrtPlnType Plane;
    IrtHmgnMatType Mat;

    CagdExtrudeSrf(NULL, NULL);
    CagdPrimTorusSrf(Center, 0.0, 0.0, TRUE);

    CagdOneBoolSumSrf(NULL);

    BspCrvMult(NULL, NULL);
    BzrCrvMult(NULL, NULL);
    BspSrfMult(NULL, NULL);
    BzrSrfMult(NULL, NULL);

    BspSrfDeriveRational(NULL, CAGD_CONST_U_DIR);
    BzrSrfDeriveRational(NULL, CAGD_CONST_U_DIR);

    SymbSrf2OptPolysCurvatureErrorPrep(NULL);
    SymbComposeSrfCrv(NULL, NULL);

    IPCoerceObjectTo(NULL, 0);

    UserPolylines2LinBsplineCrvs(NULL, TRUE);
    UserCntrSrfWithPlane(NULL, Plane, 10.0);

    MdlModelListBBox(NULL, NULL);

    IPMapObjectInPlace(NULL, Mat);

    IGGlblViewMode = IG_VIEW_ORTHOGRAPHIC;
}
