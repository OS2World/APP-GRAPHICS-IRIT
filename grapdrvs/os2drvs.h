/****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				    *
*****************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology               *
*****************************************************************************
* Written by:  Gershon Elber				Ver 1.0, Jan. 1992  *
*****************************************************************************
* Global definitions of	os2drvs interface.			            *
****************************************************************************/

#ifndef	OS2DRVS_H	/* Define only once */
#define	OS2DRVS_H

#define ID_OS2DRVS		1984

#define IDM_FILE		10
#define IDM_FILE_SAVE		11
#define IDM_FILE_SAVE_AS	12
#define IDM_FILE_CLEAR_VIEW	13
#define IDM_FILE_QUIT		14

#define IDM_MOUSE		20
#define IDM_MOUSE_MORE		21
#define IDM_MOUSE_LESS		22

#define IDM_STATE		30
#define IDM_STATE_MORE_ISO	31
#define IDM_STATE_LESS_ISO	32
#define IDM_STATE_FINER_APPROX	33
#define IDM_STATE_COARSER_APPROX 34
#define IDM_STATE_SHORTER_VEC	35
#define IDM_STATE_LONGER_VEC	36
#define IDM_STATE_WIDE_LINES	37
#define IDM_STATE_THIN_LINES	38
#define IDM_STATE_WIDE_POINTS	39
#define IDM_STATE_THIN_POINTS	40

#define IDM_TOGGLE		50
#define IDM_TGLS_SCREEN		51
#define IDM_TGLS_PERSP		52
#define IDM_TGLS_DEPTH_CUE	53
#define IDM_TGLS_DOUBLE_BUFFER	54
#define IDM_TGLS_DRAW_SOLID	55
#define IDM_TGLS_BFACE_CULL	56
#define IDM_TGLS_SHADING_MODES	57
#define IDM_TGLS_INTERNAL	58
#define IDM_TGLS_VRTX_NRML	59
#define IDM_TGLS_POLY_NRML	60
#define IDM_TGLS_CTL_MESH	61
#define IDM_TGLS_SRF_POLYS	62
#define IDM_TGLS_SRF_ISOS	63
#define IDM_TGLS_SRF_SKETCH	64
#define IDM_TGLS_4_PER_FLAT	65

#define	IDM_VIEWS		70
#define	IDM_VIEW_FRONT		71
#define IDM_VIEW_SIDE		72
#define IDM_VIEW_TOP		73
#define IDM_VIEW_ISOMETRY	74

#define IDM_EXTENSIONS		100
#define IDM_EXTN_ANIMATION	101
#define IDM_EXTN_SHADE_PARAM	102

/* Animation's dialog box constants. */
#define ID_ANIM_FORM		150
#define ID_ANIM_SLIDER		151
#define ID_ANIM_SAVE_FILE	152
#define ID_ANIM_MIN_TIME	153
#define ID_ANIM_MAX_TIME	154
#define ID_ANIM_TIME_STEP	155
#define ID_ANIM_BEGIN		156
#define ID_ANIM_STOP		157
#define ID_ANIM_DISMISS		158

#define ID_ANIM_GET_TIME_FORM	170
#define ID_ANIM_GOT_TIME	171
#define ID_ANIM_GET_TIME_ENT	172
#define ID_ANIM_GET_TIME_CAN	173

#define ID_ANIM_SLIDER_FONT	"6.Courier"

/* Shade Param's dialog box constants. */
#define ID_SHADE_PARAM_FORM		200
#define ID_SHADE_RESET			201
#define ID_SHADE_DISMISS		202
#define ID_SHADE_SKETCH_SILS		210
#define ID_SHADE_SKETCH_CRVTR		211
#define ID_SHADE_SKETCH_ISOCRVS		212
#define ID_SHADE_STYLE_NONE		220
#define ID_SHADE_STYLE_BACKGROUND	221
#define ID_SHADE_STYLE_FLAT		222
#define ID_SHADE_STYLE_GOURAUD		223
#define ID_SHADE_STYLE_PHONG		224
#define ID_SHADE_AMBIENT_SLIDER		230
#define ID_SHADE_DIFFUSE_SLIDER		231
#define ID_SHADE_SPECULAR_SLIDER	232
#define ID_SHADE_SHININESS_SLIDER	233
#define ID_SKETCH_SIL_POWER_SLIDER	234
#define ID_SKETCH_SHD_POWER_SLIDER	235

#endif /* OS2DRVS_H */
