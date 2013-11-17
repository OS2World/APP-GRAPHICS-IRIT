/*****************************************************************************
* Adaptive Iso Shader program to render freeform models.		     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Gershon Elber			     Ver 0.1, October 1994.  *
*****************************************************************************/

#ifndef ADAP_ISO_SHADER_H
#define ADAP_ISO_SHADER_H

IRIT_GLOBAL_DATA_HEADER SymbCrvApproxMethodType
    GlblCrvApproxMethod;
IRIT_GLOBAL_DATA_HEADER int
    GlblTalkative,
    GlblSamplesPerCurve,
    GlblPolyOptimalMethod;
IRIT_GLOBAL_DATA_HEADER IrtRType
    GlblPolyFineNess;

/* Global function in aisoshad.c/pts_shad.c. */
void ShaderExit(int ExitCode);

/* Global function in zbufcrvs.c. */
void ScanConvertPolySrfs(IPObjectStruct *PObjs,
			 int ZBufferSize,
			 IrtRType Depth,
			 int ImageOperator);
void DumpZBufferAsSrf(char *FileName);
IPObjectStruct *TestCurveVisibility(CagdCrvStruct *Crvs, IrtRType WidthScale);
int TestPointVisibility(IrtRType X, IrtRType Y, IrtRType Z);
IrtRType GetPointDepth(int x, int y);

#endif /* ADAP_ISO_SHADER_H */
