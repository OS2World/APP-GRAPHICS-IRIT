/*****************************************************************************
*   "Irit" - the 3d polygonal solid modeller.				     *
*									     *
* Written by:  Gershon Elber				Ver 1.0, Jan. 1992   *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Global definitions of	wntdrvs interface.			             *
*****************************************************************************/

#ifndef	WNTDRVS_H	/* Define only once */
#define	WNTDRVS_H

#ifndef WINVER
#define WINVER 0x0400
#endif

#define APP_CLASS		"wntdrvs"
#define APP_TITLE		"Irit Display Device"
#define APP_MENU		999
#define APP_VIEW_CLASS		"IritViewClass"
#define APP_SUB_VIEW_CLASS	"IritSubViewClass"
#define APP_TRANS_CLASS		"IritTransClass"

#define IDM_FILE		1
#define IDM_FILE_SAVE_MAT	11
#define IDM_FILE_SAVE_MAT_AS	12
#define IDM_FILE_SAVE_IMAGE_AS	13
#define IDM_FILE_SUBMIT		14
#define IDM_FILE_CLEAR_VIEW     15
#define IDM_FILE_BG_COLOR       16
#define IDM_FILE_DISCONNECT	17
#define IDM_FILE_QUIT		18

#define IDM_MOUSE		2
#define IDM_MOUSE_MORE		21
#define IDM_MOUSE_LESS		22

#define IDM_STATE		3
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

#define IDM_TOGGLE		5
#define IDM_TGLS_SCREEN		51
#define IDM_TGLS_CONT_MOTION    52
#define IDM_TGLS_NRML_ORIENT    53
#define IDM_TGLS_PERSP		54
#define IDM_TGLS_DEPTH_CUE	55
#define IDM_TGLS_DOUBLE_BUFFER  56
#define IDM_TGLS_ANTI_ALIASING  57
#define IDM_TGLS_DRAW_STYLE	58
#define IDM_TGLS_BFACE_CULL	59
#define IDM_TGLS_SHADING_MODES  60
#define IDM_TGLS_INTERNAL	61
#define IDM_TGLS_VRTX_NRML	62
#define IDM_TGLS_POLY_NRML	63
#define IDM_TGLS_CTL_MESH	64
#define IDM_TGLS_SRF_POLYS	65
#define IDM_TGLS_SRF_BNDRY	66
#define IDM_TGLS_SRF_SILH	67
#define IDM_TGLS_SRF_ISOS	68
#define IDM_TGLS_SRF_SKTCH      69
#define IDM_TGLS_POLYGONS	70
#define IDM_TGLS_4_PER_FLAT	71
#define IDM_TGLS_NUM_POLY_COUNT 72
#define IDM_TGLS_FRAME_PER_SEC  73

#define	IDM_VIEWS		8
#define	IDM_VIEW_FRONT		80
#define IDM_VIEW_SIDE		81
#define IDM_VIEW_TOP		82
#define IDM_VIEW_ISOMETRY	83
#define IDM_VIEW_4VIEWS		84

#define IDM_EXTENSIONS		9
#define IDM_EXTN_ENVIRONMENT	90
#define IDM_EXTN_ANIMATION	91
#define IDM_EXTN_SHADE_PARAM    92
#define IDM_EXTN_CRV_EDIT       93
#define IDM_EXTN_SRF_EDIT       94
#define IDM_EXTN_PICK_OBJS	95
#define IDM_EXTN_OBJ_MANIP	96
#define IDM_EXTN_TRANSFORMATIONS 97

#define IDEN_ENV_PARAM_FORM	9500

#define IDEN_BUTTON_SCREEN_OBJ		9000
#define IDEN_BUTTON_PERSP_ORTHO		9001
#define IDEN_BUTTON_CONT_MOTION		9002
#define IDEN_BUTTON_NRML_ORIENT		9003
#define IDEN_BUTTON_LIGHT_ONE_SIDE	9004
#define IDEN_BUTTON_Z_BUFFER		9005
#define IDEN_BUTTON_DEPTH_CUE		9006
#define IDEN_BUTTON_CULL_BFACE		9007
#define IDEN_BUTTON_INT_EDGES		9008
#define IDEN_BUTTON_DOUBLE_BUF		9010
#define IDEN_BUTTON_ANTI_ALIAS		9011
#define IDEN_BUTTON_DRAW_VNRMLS		9012
#define IDEN_BUTTON_DRAW_PNRMLS		9013
#define IDEN_BUTTON_DRAW_SMESH		9014
#define IDEN_BUTTON_DRAW_SPOLY		9015
#define IDEN_BUTTON_DRAW_BNDRY  	9016
#define IDEN_BUTTON_DRAW_SILH   	9017
#define IDEN_BUTTON_DRAW_SISO		9018
#define IDEN_BUTTON_DRAW_SSKTCH		9020
#define IDEN_BUTTON_DRAW_POLYGONS	9021
#define IDEN_BUTTON_DRAW_RFLCT_LNS	9022
#define IDEN_BUTTON_DRAW_4_FLAT		9023
#define IDEN_BUTTON_CLEAR_VIEW		9024
#define IDEN_BUTTON_DISMISS		9025
#define IDEN_BUTTON_NUM_POLY_COUNT	9026
#define IDEN_BUTTON_POLYGON_OPTI	9027
#define IDEN_BUTTON_POLYLINE_OPTI	9028
#define IDEN_BUTTON_DRAW_3D_GLASSES	9030
#define IDEN_BUTTON_FRAME_PER_SEC	9031

#define IDEN_NORMAL_LEN_TRACKBAR	9040
#define IDEN_LINE_WIDTH_TRACKBAR	9041
#define IDEN_SENSITIVITY_TRACKBAR	9042
#define IDEN_LOWRES_RATIO_TRACKBAR	9043
#define IDEN_ISOLINES_TRACKBAR		9044
#define IDEN_POLYLINE_FINENESS_TRACKBAR	9045
#define IDEN_POLYGON_FINENESS_TRACKBAR	9046
#define IDEN_NORMAL_LEN_TITLE		9050
#define IDEN_LINE_WIDTH_TITLE		9051
#define IDEN_SENSITIVITY_TITLE		9052
#define IDEN_LOWRES_RATIO_TITLE		9053
#define IDEN_ISOLINES_TITLE		9054
#define IDEN_POLYLINE_FINENESS_TITLE	9055
#define IDEN_POLYGON_FINENESS_TITLE	9056

#define IDEN_SHADE_STYLE		9070
#define IDEN_SHADE_STYLE_NONE		9071
#define IDEN_SHADE_STYLE_BACKGROUND	9072
#define IDEN_SHADE_STYLE_FLAT		9073
#define IDEN_SHADE_STYLE_GOURAUD	9074
#define IDEN_SHADE_STYLE_PHONG		9075

#define IDEN_DRAW_STYLE			9080

#define IDEN_DRAW_STYLE_WIREFRAME	9081
#define IDEN_DRAW_STYLE_SOLID		9082
#define IDEN_DRAW_STYLE_POINTS		9083

#define IDEN_ANTI_ALIAS			9090
#define IDEN_ANTI_ALIAS_OFF		9091
#define IDEN_ANTI_ALIAS_ON		9092
#define IDEN_ANTI_ALIAS_BLEND		9093

#define IDC_BUTTON_RESTART              1000
#define IDC_STATIC_PICTURE              1002
#define IDC_BUTTON_SAVE_GEOM		1003
#define IDC_BUTTON_SAVE_IMAGE		1004
#define IDC_BUTTON_REWIND               1005
#define IDC_BUTTON_PLAYBACK             1006
#define IDC_BUTTON_PLAYBACKWARD         1007
#define IDC_BUTTON_STOP                 1008
#define IDC_BUTTON_PLAY                 1009
#define IDC_BUTTON_FORWARD              1010
#define IDC_BUTTON_MINTIME              1011
#define IDC_BUTTON_MAXTIME              1012
#define IDC_BUTTON_INTERVALDT           1013
#define IDC_BUTTON_FASTINTERVALDT       1014
#define IDC_BUTTON_SSTEP                1015
#define IDC_TRACKBAR_SCALE              1016
#define IDC_STATIC_SCALENAME            1017
#define IDC_BUTTON_SINGLESTEP           1019
#define IDC_EDIT_SELECTION              1020
#define IDC_BUTTON6                     1022
#define IDC_SCROLLBAR1                  1022
#define IDC_BUTTON7                     1023
#define IDC_BUTTON8                     1024
#define IDC_BUTTON9                     1025
#define IDC_BUTTON10                    1026
#define IDC_STATIC_SELECTIONLABEL	1027
#define IDC_STATIC_LABEL1               1028
#define IDC_STATIC_LABEL2               1029
#define IDC_STATIC_LABEL3               1030
#define IDC_STATIC_TAG1                 1031
#define IDC_CHECK1      	        1032
#define IDC_CHECK2	                1033
#define IDC_EDIT1	                1034
#define IDC_STYLE			1035
#define IDC_TWO_WAYS			1036
#define IDC_REPEAT			1037

#define IDD_ANIMATION_FORM              1501
#define IDD_PROMPT                      1503
#define IDB_BITMAP_PLAYBACK             1504
#define IDB_BITMAP_REWIND               1505
#define IDB_BITMAP_PLAYBACKWARD         1506
#define IDB_BITMAP_STOP                 1507
#define IDB_BITMAP_PLAY                 1508
#define IDB_BITMAP_FORWARD              1509

#define IDD_PROJNAME			1700
#define IDD_IRITGDISPLAYPROPPAGE        1701
#define IDR_IRITGDISPLAYPROPPAGE        1702
#define IDD_IRITGDISPLAY                1703
#define IDR_IRITGDISPLAY                1704
#define IDD_TITLEIritGDisplayPropPage   1705
#define IDD_HELPFILEIritGDisplayPropPage 1706
#define IDD_DOCSTRINGIritGDisplayPropPage 1707
#define IDD_TITLEIritGDisplayPropPage1   1708
#define IDD_HELPFILEIritGDisplayPropPage1 1709
#define IDD_DOCSTRINGIritGDisplayPropPage1 1710

#define IDS_RESET			2000
#define IDS_DISMISS			2001
#define IDS_SKETCH_SIL			2010
#define IDS_SKETCH_SIL_ISOCRVS		2011
#define IDS_SKETCH_SIL_CRVTR		2012
#define IDS_SKETCH_SIL_ISOCLINES	2013
#define IDS_SKETCH_SIL_ORTHOCLINES	2014
#define IDS_SKETCH_SHD			2015
#define IDS_SKETCH_SHD_ISOCRVS		2016
#define IDS_SKETCH_SHD_CRVTR		2017
#define IDS_SKETCH_SHD_ISOCLINES	2018
#define IDS_SKETCH_SHD_ORTHOCLINES	2019
#define IDS_SKETCH_IMP			2020
#define IDS_SKETCH_IMP_ISOCRVS		2021
#define IDS_SKETCH_IMP_CRVTR		2022
#define IDS_SKETCH_IMP_ISOCLINES	2023
#define IDS_SKETCH_IMP_ORTHOCLINES	2024
#define IDS_SKETCH_IMPORTANCE		2025
#define IDS_SKETCH_FRONTAL_SUPPORT	2026
#define IDS_SKETCH_SHD_INV		2027
#define IDS_STYLE			2030
#define IDS_STYLE_NONE			2031
#define IDS_STYLE_BACKGROUND		2032
#define IDS_STYLE_FLAT			2033
#define IDS_STYLE_GOURAUD		2034
#define IDS_STYLE_PHONG			2035
#define IDS_TRACKBAR1_SCALE		2040
#define IDS_TRACKBAR2_SCALE		2041
#define IDS_TRACKBAR3_SCALE		2042
#define IDS_TRACKBAR4_SCALE		2043
#define IDS_TRACKBAR5_SCALE		2044
#define IDS_TRACKBAR6_SCALE		2045
#define IDS_TRACKBAR7_SCALE		2046
#define IDS_TRACKBAR8_SCALE		2047
#define IDS_TRACKBAR9_SCALE		2048
#define IDS_TRACKBAR10_SCALE		2049
#define IDS_LIGHT_NUM			2060
#define IDS_LIGHT_X			2061
#define IDS_LIGHT_Y			2062
#define IDS_LIGHT_Z			2063
#define IDS_LIGHT_W			2064

#define IDS_SHADE_PARAM_FORM            2501

#define IDCE_CLEAR			3000
#define IDCE_NAME			3001
#define IDCE_DRAW_MESH			3002
#define IDCE_DRAW_ORIG			3003
#define IDCE_UNDO			3004
#define IDCE_REDO			3005
#define IDCE_CONSTRAINTS		3006
#define IDCE_SAVE_CURVE			3010
#define IDCE_SUBMIT_CURVE		3011
#define IDCE_DISMISS			3012
#define IDCE_STATE			3013
#define IDCE_ORDER			3014
#define IDCE_END_COND			3015
#define IDCE_RATIONAL			3016
#define IDCE_REGION			3017
#define IDCE_REFINE			3018
#define IDCE_SUBDIV			3019
#define IDCE_SUBDIV1			3020
#define IDCE_SUBDIV2			3021
#define IDCE_CMERGE			3022
#define IDCE_MOVE_CTLPTS		3023
#define IDCE_DELETE_CTLPTS		3024
#define IDCE_MODIFY_CURVE		3025
#define IDCE_MESSAGE			3026
#define IDCE_PARAM_TYPE			3027
#define IDCE_CURVE_TYPE			3028
#define IDCE_DRAISE			3029
#define IDCE_LST_SQR_PERCENT		3030
#define IDCE_MR_STATUS			3040
#define IDCE_MR_TRACKBAR_SCALE		3041
#define IDCE_REVERSE_CURVE		3042
#define IDCE_EVALUATE_CURVE		3043
#define IDCE_PRIMITIVE_CURVE		3044

#define IDCE_CRV_EDIT_PARAM_FORM        3501

#define IDSE_CLEAR			4000
#define IDSE_NAME			4001
#define IDSE_DRAW_MESH			4002
#define IDSE_DRAW_ORIG			4003
#define IDSE_UNDO			4004
#define IDSE_REDO			4005
#define IDSE_SAVE_SRF			4010
#define IDSE_SUBMIT_SRF			4011
#define IDSE_DISMISS			4012
#define IDSE_STATE			4013
#define IDSE_U_ORDER			4014
#define IDSE_V_ORDER			4015
#define IDSE_U_END_COND			4016
#define IDSE_V_END_COND			4017
#define IDSE_RATIONAL			4018
#define IDSE_REGION			4019
#define IDSE_REFINE			4020
#define IDSE_SUBDIV			4021
#define IDSE_SMERGE			4022
#define IDSE_MOVE_CTLPTS		4023
#define IDSE_MODIFY_SRF			4024
#define IDSE_MODIFY_NORMAL_DIR		4025
#define IDSE_MESSAGE			4026
#define IDSE_U_PARAM_TYPE		4027
#define IDSE_V_PARAM_TYPE		4028
#define IDSE_U_DRAISE			4029
#define IDSE_V_DRAISE			4030
#define IDSE_SRF_TYPE			4031
#define IDSE_MRU_STATUS			4040
#define IDSE_MRV_STATUS			4041
#define IDSE_MRU_TRACKBAR_SCALE		4042
#define IDSE_MRV_TRACKBAR_SCALE		4043
#define IDSE_REVERSE_SRF		4044
#define IDSE_TRIM_SRF			4045
#define IDSE_EVALUATE_SRF		4046
#define IDSE_PRIMITIVE_SRF		4047

#define IDSE_SRF_EDIT_PARAM_FORM        4501

#define IDPK_PICK_OBJS		 5000
#define IDPK_PICK_OBJS_POLY	 5001
#define IDPK_PICK_OBJS_NUMERIC	 5002
#define IDPK_PICK_OBJS_POINT	 5003
#define IDPK_PICK_OBJS_VECTOR	 5004
#define IDPK_PICK_OBJS_PLANE	 5005
#define IDPK_PICK_OBJS_MATRIX	 5006
#define IDPK_PICK_OBJS_CURVE	 5007
#define IDPK_PICK_OBJS_SURFACE	 5008
#define IDPK_PICK_OBJS_STRING	 5009
#define IDPK_PICK_OBJS_LIST_OBJ	 5010
#define IDPK_PICK_OBJS_CTLPT	 5011
#define IDPK_PICK_OBJS_TRIMSRF	 5012
#define IDPK_PICK_OBJS_TRIVAR	 5013
#define IDPK_PICK_OBJS_INSTANCE	 5014
#define IDPK_PICK_OBJS_TRISRF	 5015
#define IDPK_PICK_OBJS_MODEL	 5016
#define IDPK_PICK_OBJS_MULTIVAR	 5017
#define IDPK_PICK_SAVE_POLY	 5018

#define IDPK_PICK_DISMISS	 5099

#define IDPK_PICK_OBJS_FORM      5501

#define IDOT_ROT_X_TRACKBAR	 6001
#define IDOT_ROT_Y_TRACKBAR	 6002
#define IDOT_ROT_Z_TRACKBAR	 6003
#define IDOT_TRANS_X_TRACKBAR	 6004
#define IDOT_TRANS_Y_TRACKBAR	 6005
#define IDOT_TRANS_Z_TRACKBAR	 6006
#define IDOT_SCALE_TRACKBAR	 6007
#define IDOT_ROT_X_TITLE	 6010
#define IDOT_ROT_Y_TITLE	 6011
#define IDOT_ROT_Z_TITLE	 6012
#define IDOT_TRANS_X_TITLE	 6013
#define IDOT_TRANS_Y_TITLE	 6014
#define IDOT_TRANS_Z_TITLE	 6015
#define IDOT_SCALE_TITLE	 6016
#define IDOT_STATE	         6020
#define IDOT_SCREEN_SPACE        6021
#define IDOT_SNAP		 6022
#define IDOT_SNAP_DEGREES	 6023
#define IDOT_SNAP_DISTANCE	 6024
#define IDOT_SAVE		 6025
#define IDOT_SUBMIT		 6026
#define IDOT_DELETE		 6027
#define IDOT_MESSAGE		 6028
#define IDOT_DISMISS		 6029
#define IDOT_RESET		 6030
#define IDOT_NAME		 6031
#define IDOT_REVERSE		 6032
#define IDOT_COLOR		 6033
#define IDOT_STATIC_LABEL1       6040
#define IDOT_STATIC_LABEL2       6041
#define IDOT_STATIC_LABEL3       6042

#define IDOT_OBJ_MANIP_FORM	 6500

#define IDCC_CRV_CNST_TEXT		7000
#define IDCC_ADD_CNST			7001
#define IDCC_DELETE_CNST		7002
#define IDCC_DISMISS			7003
#define IDCC_STATIC_LABEL1		7010
#define IDCC_UNDULATE			7011
#define IDCC_SATISFY_ALL		7012
#define IDCC_CONSTRAINTS		7013
#define IDCC_X_SYMMETRY			7020
#define IDCC_Y_SYMMETRY			7021
#define IDCC_C_SYMMETRY			7022
#define IDCC_AREA			7023

#define IDCC_CRV_CNST_PARAM_FORM	7500

#define ID_STATIC_SCALETITLE1           9900
#define ID_STATIC_SCALETITLE2           9901
#define ID_STATIC_SCALETITLE3           9902
#define ID_STATIC_SCALETITLE4           9903
#define ID_STATIC_SCALETITLE5           9904
#define ID_STATIC_SCALETITLE6           9905
#define ID_STATIC_SCALETITLE7           9906
#define ID_STATIC_SCALETITLE8           9907
#define ID_STATIC_SCALETITLE9           9908
#define ID_STATIC_SCALETITLE10          9909
#define ID_STATIC_SCALETITLE11          9910
#define ID_STATIC_SCALETITLE12          9911
#define ID_STATIC_SCALETITLE13          9912
#define ID_STATIC_SCALETITLE14          9913
#define ID_STATIC_SCALETITLE15          9914
#define ID_STATIC_SCALETITLE16          9915

#define IG_NUM_OF_SUB_VIEWS    		4

#define DEFAULT_TRANS_WIDTH	200
#define DEFAULT_TRANS_HEIGHT	500
#define DEFAULT_VIEW_WIDTH	400
#define DEFAULT_VIEW_HEIGHT	400
                                   
#define ARGCV_LINE_LEN	1000
#define ARGCV_MAX_WORDS	100

#define WNT_MAP_X_COORD(x) ((int) (IGViewWidth2 + x * IGViewWidth2))
#define WNT_MAP_Y_COORD(y) ((int) (IGViewHeight2 - y * IGViewHeight2))

#define SET_MENU_FLAGS(Bool)  (MF_STRING | (Bool ? MF_CHECKED : 0))

typedef struct SlideBar {
    HWND  ValueText;
    SHORT DecimalPoints;
    SHORT Scale;
} SlideBar;

typedef struct ActiveXCreateInfoStruct {
    BOOL hasTransWnd;
    HWND controlWnd;
} ActiveXCreateInfoStruct;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

IRIT_GLOBAL_DATA_HEADER short GlblDrvsColors[IG_MAX_COLOR + 1][3];
IRIT_GLOBAL_DATA_HEADER void
    *IGGlblWinBGImage;
IRIT_GLOBAL_DATA_HEADER unsigned int
    IGViewWidth,
    IGViewHeight,
    IGViewWidth2,
    IGViewHeight2,
    IGSubViewWidth,
    IGSubViewHeight;
IRIT_GLOBAL_DATA_HEADER int
    IGLastWasSolidRendering,
    IGGlbl4ViewSeperationColor[3],
    IGGlblWinBGImageX,
    IGGlblWinBGImageY;
IRIT_GLOBAL_DATA_HEADER IrtHmgnMatType
    IGSubViewMat[4];
IRIT_GLOBAL_DATA_HEADER HBRUSH
    IGBackGroundBrush,
    IG4ViewSeperationBrush;
IRIT_GLOBAL_DATA_HEADER COLORREF
    IGBackGroundColor,
    IG4ViewSeperationColor,
    IGCrntColorLowIntensity,
    IGCrntColorHighIntensity,
    IGColorsLowIntensity[IG_MAX_COLOR + 1],
    IGColorsHighIntensity[IG_MAX_COLOR + 1];
IRIT_GLOBAL_DATA_HEADER HPEN
    IGCurrenthPen;
IRIT_GLOBAL_DATA_HEADER HWND
    IGhWndView, IGhWndTrans, IGhTopLevel,
    IGhWndSubView[IG_NUM_OF_SUB_VIEWS];
IRIT_GLOBAL_DATA_HEADER HDC
    IGCurrenthDC;
IRIT_GLOBAL_DATA_HEADER HMENU
    IGGlblStateMenu;
IRIT_GLOBAL_DATA_HEADER HCURSOR
    GlblCurrentCursor;

void GetArgCV(CHAR *Str, int *argc, CHAR ***argv);
LONG APIENTRY WndProc(HWND hWndFrame,
		      UINT wMsg,
		      WPARAM wParam,
		      LONG lParam);
LONG APIENTRY ViewWndProc(HWND hWndFrame,
			  UINT wMsg,
			  WPARAM wParam,
			  LONG lParam);
LONG APIENTRY TransWndProc(HWND hWndFrame,
			   UINT wMsg,
			   WPARAM wParam,
			   LONG lParam);
void IGUpdateWindowPixels(HWND hWnd, HDC MemDC, int x, int y);
void *IGCaptureWindowPixels(HWND hWnd, int *x, int *y, int ReturnImage);
void CreateSubViewWindow(HWND hWnd, int ViewNum);
int RedrawSubViewWindow(HWND hWnd, UINT wMsg, WPARAM wParam);
int GetViewNum(HWND hWnd);
void UpdateSubViewMatrix(int ViewNum);
void ClearBase4Views(HWND hWnd);

int RedrawViewWindow(HWND hWnd, UINT wMsg, WPARAM wParam);
void IGSetColorIndex(int c);
void IGIritBeep(void);

/* wnt_env.c */
void CreateEnvironment(HWND IGTopLevel);
void EnvParamUpdateCb(void);
void EnvironmentCB(void);

/* wnt_pick.c */
void CreatePickObjs(HWND IGTopLevel);
void PickObjsUpdateCb(void);
void PickObjsCB(void);

/* wnt_anim.c */
void CreateAnimation(HWND IGTopLevel); 
void AnimationCB(void);
void ReplaceLabel(HWND, char *NewLabel);
void DisplayErrorMsg(void);
void DisplayWrnMsg(void);
void DisplayErrValue(char *Msg);
int PromptDialogBox(HWND Parent,
		    char *Title,
		    char *SelectionLabel,
		    void (*OkCB)(HWND, PVOID, PVOID),
		    PVOID ClientData);

/* wnt_shad.c */
void ShadeUpdateRadioButtons(void);
void CreateShadeParam(HWND IGTopLevel);
void ShadeParamCB(void);

/* wnt_crvs.c */
void CreateCrvEditParam(HWND IGTopLevel);
void IGCrvEditParamUpdateMRScale(void);
void CrvEditCB(void);

/* wnt_srfs.c */
void CreateSrfEditParam(HWND IGTopLevel);
void IGSrfEditParamUpdateMRScale(void);
void SrfEditCB(void);

/* wnt_manp.c */
void CreateObjectManip(HWND IGTopLevel);
void IGObjManipParamUpdateWidget(void);
void ObjManipCB(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* WNTDRVS_H */
