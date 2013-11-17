/*****************************************************************************
*   A motif interface for animation.					     *
******************************************************************************
* (C) Gershon Elber, Technion, Israel Institute of Technology                *
******************************************************************************
* Written by:  Iris Steinvarts			       Ver 0.1, March 1995.  *
*****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>

#include "irit_sm.h"
#include "grap_loc.h"
#include "xmtdrvs.h"

typedef enum {
    MOVIE_MIN,
    MOVIE_MAX,
    MOVIE_DT,
    MOVIE_FAST_DT
}
MovieScaleType;

#define FRAC		30
#define SCALEMULT	10000.0
#define BUTTONSNUM	20
#define BUFSIZE		100
#define OFFSET		1

#define ICON_WIDTH	70
#define ICON_HEIGHT	60
#define PLAY2_WIDTH	50
#define PLAY2_HEIGHT	30
#define PLAY_WIDTH	50
#define PLAY_HEIGHT	30
#define REW_WIDTH	50
#define REW_HEIGHT	30
#define FF_WIDTH	50
#define FF_HEIGHT	30
#define STOP_WIDTH	50
#define STOP_HEIGHT	30

IRIT_STATIC_DATA unsigned char IconBits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
   0x84, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x04, 0x02, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x1f, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc1,
   0x7f, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe1, 0xff, 0x10, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x80, 0xf0, 0xff, 0x21, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0xf0, 0xff, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf8,
   0xff, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf8, 0xff, 0x23, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0xf8, 0xff, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf8,
   0xff, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xf0, 0xff, 0x21, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x80, 0xf0, 0xff, 0x21, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xe1, 0xff, 0x10, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0xe1,
   0x7f, 0x10, 0x00, 0x80, 0xff, 0x00, 0x00, 0x00, 0x72, 0x1f, 0x08, 0x00,
   0x60, 0x08, 0x03, 0x00, 0x00, 0x3c, 0x04, 0x04, 0x00, 0x10, 0x08, 0x04,
   0x00, 0x00, 0x1c, 0x04, 0x02, 0x00, 0x08, 0x08, 0x08, 0x00, 0x00, 0x2e,
   0x84, 0x01, 0x00, 0x08, 0x08, 0x08, 0x00, 0x00, 0xc7, 0x7f, 0x00, 0x00,
   0x04, 0x7f, 0x10, 0x00, 0x80, 0x07, 0x07, 0x00, 0x00, 0x84, 0xff, 0x10,
   0x00, 0xc0, 0xcf, 0x00, 0x00, 0x00, 0x84, 0xff, 0x10, 0x00, 0xc0, 0x3f,
   0x00, 0x00, 0x00, 0x84, 0xff, 0x10, 0xfe, 0xff, 0xff, 0xff, 0x01, 0x20,
   0xfe, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0xff, 0x01, 0x30, 0x84, 0xff, 0xd0,
   0xff, 0xff, 0xff, 0xff, 0x01, 0x28, 0x84, 0xff, 0x90, 0xff, 0xff, 0xff,
   0xff, 0x39, 0x24, 0x84, 0xff, 0x90, 0xff, 0xff, 0xff, 0xff, 0x39, 0x22,
   0x04, 0x7f, 0xd0, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x21, 0x08, 0x08, 0xc8,
   0xff, 0xff, 0xff, 0x7f, 0xbf, 0x20, 0x08, 0x08, 0xe8, 0xff, 0xff, 0xff,
   0x7f, 0x7f, 0x20, 0x14, 0x08, 0xe4, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x20,
   0x64, 0x08, 0xe3, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x20, 0x8c, 0xff, 0xf0,
   0xff, 0xff, 0xff, 0x7f, 0x3f, 0x20, 0x1c, 0x08, 0xf8, 0xff, 0xdf, 0xbf,
   0x7f, 0x3f, 0x20, 0x3c, 0x00, 0xfc, 0xff, 0x27, 0x4f, 0x7e, 0x7f, 0x20,
   0xfc, 0xff, 0xff, 0xff, 0x77, 0xef, 0xfe, 0xbf, 0x20, 0xfc, 0xff, 0xff,
   0xff, 0xfb, 0xf6, 0xfd, 0x39, 0x21, 0xfc, 0xff, 0xff, 0xff, 0x77, 0xef,
   0xfe, 0x39, 0x22, 0xfc, 0xff, 0xff, 0xff, 0x27, 0x4f, 0xfe, 0x01, 0x24,
   0xfc, 0xff, 0xff, 0xff, 0xdf, 0xbf, 0xff, 0x01, 0x24, 0xfc, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x01, 0x28, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x01, 0x30, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x20,
   0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x07, 0x00, 0x00, 0xfc, 0xff, 0x7f,
   0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x0e, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00,
   0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x70, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
   0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0xf8, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* play2.BitMap */

IRIT_STATIC_DATA unsigned char Play2Bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x30,
   0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0xfc, 0x00, 0x00, 0x00,
   0x00, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xc0, 0x3f, 0x00, 0xfc, 0x00,
   0x00, 0x00, 0xf0, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00,
   0xfc, 0x00, 0x00, 0x00, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0xc0, 0xff,
   0x3f, 0x00, 0xfc, 0x00, 0x00, 0xf0, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00,
   0xfc, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0xff, 0xff, 0x3f, 0x00, 0xfc,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0xf0, 0xff, 0xff, 0x3f,
   0x00, 0xfc, 0x00, 0xfc, 0xff, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0xfc, 0xff,
   0xff, 0x3f, 0x00, 0xfc, 0x00, 0xf0, 0xff, 0xff, 0x3f, 0x00, 0xfc, 0x00,
   0xc0, 0xff, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0xff, 0xff, 0x3f, 0x00,
   0xfc, 0x00, 0x00, 0xfc, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0xf0, 0xff,
   0x3f, 0x00, 0xfc, 0x00, 0x00, 0xc0, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00,
   0x00, 0xff, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0xfc,
   0x00, 0x00, 0x00, 0xf0, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xc0, 0x3f,
   0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0x00,
   0x00, 0x3c, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0xfc, 0x00,
   0x00, 0x00, 0x00, 0x20, 0x00, 0xfc};

/* play.BitMap */

IRIT_STATIC_DATA unsigned char PlayBits[] = {
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x03,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xf8,
   0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, 0x00, 0x00,
   0x00, 0xf8, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x3f,
   0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0xf8, 0xff,
   0xff, 0x7f, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00,
   0xf8, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x03, 0x00,
   0x00, 0x00, 0xf8, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x3f,
   0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8,
   0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x00, 0x00, 0x00, 0x00};

/* rew.BitMap */

IRIT_STATIC_DATA unsigned char RewBits[] = {
   0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xc0, 0x01, 0x00,
   0x80, 0xff, 0x00, 0x00, 0xf0, 0x01, 0x00, 0xe0, 0xff, 0x00, 0x00, 0xf8,
   0x01, 0x00, 0xf8, 0xff, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xfc, 0xff, 0x00,
   0x80, 0xff, 0x01, 0x00, 0xff, 0xff, 0x00, 0xe0, 0xff, 0x01, 0xc0, 0xff,
   0xff, 0x00, 0xf0, 0xff, 0x01, 0xe0, 0xff, 0xff, 0x00, 0xfc, 0xff, 0x01,
   0xf8, 0xff, 0xff, 0x00, 0xff, 0xff, 0x01, 0xfe, 0xff, 0xff, 0x80, 0xff,
   0xff, 0x81, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xc1, 0xff, 0xff, 0xff,
   0xf8, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfd, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xf8,
   0xff, 0xff, 0xf1, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xc1, 0xff, 0xff,
   0xff, 0x80, 0xff, 0xff, 0x01, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x01,
   0xfe, 0xff, 0xff, 0x00, 0xfc, 0xff, 0x01, 0xf8, 0xff, 0xff, 0x00, 0xf0,
   0xff, 0x01, 0xe0, 0xff, 0xff, 0x00, 0xe0, 0xff, 0x01, 0xc0, 0xff, 0xff,
   0x00, 0x80, 0xff, 0x01, 0x00, 0xff, 0xff, 0x00, 0x00, 0xfe, 0x01, 0x00,
   0xfc, 0xff, 0x00, 0x00, 0xf8, 0x01, 0x00, 0xf0, 0xff, 0x00, 0x00, 0xf0,
   0x01, 0x00, 0xe0, 0xff, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x80, 0xff, 0x00,
   0x00, 0x00, 0x01, 0x00, 0x00, 0xfe};

/* ff.BitMap */

IRIT_STATIC_DATA unsigned char FFBits[] = {
   0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x00, 0x1f, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00,
   0xfe, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xfe, 0x03, 0x00, 0x00, 0xff,
   0x03, 0x00, 0xfe, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0xfe, 0x1f, 0x00,
   0x00, 0xff, 0x1f, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0xfe,
   0xff, 0x00, 0x00, 0xff, 0xff, 0x01, 0xfe, 0xff, 0x03, 0x00, 0xff, 0xff,
   0x03, 0xfe, 0xff, 0x0f, 0x00, 0xff, 0xff, 0x0f, 0xfe, 0xff, 0x1f, 0x00,
   0xff, 0xff, 0x3f, 0xfe, 0xff, 0x7f, 0x00, 0xff, 0xff, 0xff, 0xfe, 0xff,
   0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0x01, 0xff,
   0xff, 0x3f, 0xfe, 0xff, 0x7f, 0x00, 0xff, 0xff, 0x0f, 0xfe, 0xff, 0x1f,
   0x00, 0xff, 0xff, 0x03, 0xfe, 0xff, 0x0f, 0x00, 0xff, 0xff, 0x01, 0xfe,
   0xff, 0x03, 0x00, 0xff, 0x7f, 0x00, 0xfe, 0xff, 0x00, 0x00, 0xff, 0x1f,
   0x00, 0xfe, 0x3f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0xfe, 0x1f, 0x00, 0x00,
   0xff, 0x03, 0x00, 0xfe, 0x07, 0x00, 0x00, 0xff, 0x00, 0x00, 0xfe, 0x01,
   0x00, 0x00, 0x3f, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00,
   0x3e, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x01,
   0x00, 0x00, 0x02, 0x00, 0x00, 0x00};

/* sTop.BitMap */

IRIT_STATIC_DATA unsigned char StopBits[] = {
   0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
   0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff,
   0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00,
   0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07,
   0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff,
   0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff,
   0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00,
   0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
   0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff,
   0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00,
   0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07,
   0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff,
   0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff,
   0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00,
   0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
   0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff,
   0xff, 0xff, 0x07, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00,
   0xff, 0xff, 0xff, 0xff, 0x07, 0x00};

IRIT_STATIC_DATA double
    MovieDt = 0.01,
    MovieFastDt = 0.05;
IRIT_STATIC_DATA int
    MovieSingleStep = FALSE;

typedef enum {
    PUSHB,
    IMAGEB
} ButtonKindType;

typedef struct ButtonData {
    Widget  Parent;
    int     Top;
    int     Bot;
    int     Left;
    int     Right;
    char    Name[20];
    ButtonKindType Kind;
    union {
	char Label[20];
	Pixmap BitMap;
    } Data;
} ButtonData;

IRIT_STATIC_DATA Widget AnimationForm, AnimationScale,
    AnimationButtons[BUTTONSNUM], RepeatedButton, TwowaysButton;

static void MovieRewindCb(void);
static void MovieDismissCb(void);
static void MoviePlayBackCb(void);
static void MovieStopCb(void);
static void MovieSaveGeomCb(Widget w);
static void MovieSaveImageCb(Widget w);
static void MoviePlayCb(void);
static void MovieFastForwardCb(void);
static void MovieRestartCb(void);
static void MovieRepeatedCb(void);
static void MovieTwoWaysCb(void);
static void MovieFastDtCb(void);
static void MovieDtCb(void);
static void MovieScaleCb(float ScaleValue);

static void ScaleCB(Widget Scale, XtPointer ClientData, XtPointer CallData);
static void InitButtons(void);
static void CreateButton(Widget w,
			 int index,
			 int Top,
			 int Bot,
			 int Left,
			 int Right,
			 char *Name,
			 ButtonKindType Kind,
			 VoidPtr Data);
static void ButtonsCallBack(Widget w,
			    XtPointer ClientData,
			    XmPushButtonCallbackStruct *cbs);
static void IntervalDtCB(Widget Parent);
static void SingleStepCB(Widget Parent);
static void FastIntervalDtCB(Widget Parent);
static void MinTimeCB(Widget Parent);
static void MaxTimeCB(Widget Parent);
static void ReadValue(Widget widget,
		       XtPointer ClientData,
		       XtPointer CallData);
static void NewValue(Widget widget,
		     XtPointer ClientData,
		     XtPointer CallData);
static void DisplayErrorMsg(void);
static void DisplayWrnMsg(void);
static void DisplayErrValue(char *Msg);
static int IsNumber(char *Buf, double *Value);
static void DoSingleStepAnimation(GMAnimationStruct *Anim,
				  IPObjectStruct *PObj);

/*****************************************************************************
* DESCRIPTION:								     M
*   creates the main animation window				   	     M
*									     *
* PARAMETERS:								     M
*   IGTopLevel: The shell Widget (top level shell) 			     M
*									     *
* RETURN VALUE:								     M
*   void								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   CreateAnimation                                                          M
*****************************************************************************/
void CreateAnimation(Widget IGTopLevel)
{
    Pixmap BitMap;
    XmString Text;
    Arg arg[10];
    Pixel bg;

    GMAnimFindAnimationTime(&IGAnimation, IGGlblDisplayList);
    IGAnimation.TextInterface = FALSE;

    XtSetArg(arg[0], XmNfractionBase, FRAC);

    AnimationForm = XmCreateFormDialog(IGTopLevel, "Animation", arg, 1);
    XtVaSetValues(AnimationForm, XmNautoUnmanage, False, NULL);

    Text = XmStringCreate("Animation", "TAG1");
    XtVaCreateManagedWidget("header",
			    xmLabelWidgetClass,  AnimationForm,
			    XmNlabelString,      Text,
			    XmNbottomAttachment, XmATTACH_POSITION,
			    XmNbottomPosition,   5,
			    XmNtopAttachment,    XmATTACH_POSITION,
			    XmNtopPosition,      0,
			    XmNleftAttachment,   XmATTACH_FORM,
			    XmNleftOffset,       20,
			    XmNrightAttachment,  XmATTACH_POSITION,
			    XmNrightPosition,    FRAC - 12,
			    NULL);

    XtVaGetValues(AnimationForm,
		  XmNbackground, &bg,
		  NULL);

    BitMap = XCreatePixmapFromBitmapData(XtDisplay(IGTopLevel),
				RootWindowOfScreen(XtScreen(IGTopLevel)),
				(char *) IconBits, ICON_WIDTH, ICON_HEIGHT,
				BlackPixelOfScreen(XtScreen(IGTopLevel)),
				bg,
				DefaultDepthOfScreen(XtScreen(IGTopLevel)));

    XtVaCreateManagedWidget("picture",
			    xmLabelWidgetClass,  AnimationForm,
			    XmNlabelType,        XmPIXMAP,
			    XmNlabelPixmap,      BitMap,	
			    XmNlabelString,      Text,
			    XmNbottomAttachment, XmATTACH_POSITION,
			    XmNbottomPosition,   5,
			    XmNtopAttachment,    XmATTACH_POSITION,
			    XmNtopPosition,      0,
			    XmNleftAttachment,   XmATTACH_POSITION,
			    XmNleftPosition,     FRAC - 12,
			    XmNleftOffset,       20,
			    XmNrightAttachment,  XmATTACH_POSITION,
			    XmNrightPosition,    FRAC,
			    NULL);

    AnimationScale = XtVaCreateManagedWidget("Time",
		    xmScaleWidgetClass, AnimationForm,
		    XtVaTypedArg,       XmNtitleString,
		    XmRString,          "Time", 5,
		    XmNminimum,         (int) (SCALEMULT * IGAnimation.StartT),
		    XmNmaximum,         (int) (SCALEMULT * IGAnimation.FinalT),
		    XmNvalue,           (int) (SCALEMULT * IGAnimation.StartT),
		    XmNdecimalPoints,   4,
		    XmNscaleMultiple,   (int) SCALEMULT,
		    XmNshowValue,       True,
		    XmNorientation,     XmHORIZONTAL,
		    XmNbottomAttachment, XmATTACH_POSITION,
		    XmNbottomPosition,  FRAC - 5,
		    XmNtopAttachment,   XmATTACH_POSITION,
		    XmNtopPosition,     FRAC - 10,
		    XmNleftAttachment,  XmATTACH_FORM,
		    XmNleftOffset,	20,
		    XmNrightAttachment, XmATTACH_FORM,
		    XmNrightOffset,     20,
		    NULL);

    XtAddCallback(AnimationScale, XmNdragCallback,
		  (XtCallbackProc) ScaleCB, NULL);
    XtAddCallback(AnimationScale, XmNvalueChangedCallback,
		  (XtCallbackProc) ScaleCB, NULL);

    InitButtons();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of scale.                                             *
*                                                                            *
* PARAMETERS:                                                                *
*   Scale:       The widget to handle.                                       *
*   ClientData:  Not used.                                                   *
*   CallData:    Holds the scale's value.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ScaleCB(Widget Scale, XtPointer ClientData, XtPointer CallData)
{
    XmScaleCallbackStruct
	*Cbs = (XmScaleCallbackStruct *) CallData;
   
    MovieScaleCb(Cbs -> value / SCALEMULT);
}
	
/*****************************************************************************
* DESCRIPTION:                                                               *
*   Initialize the buttons.                                                  *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void InitButtons(void)
{
    IRIT_STATIC_DATA ButtonData ButtonDataArray[BUTTONSNUM];
    int i, Depth,
	NumOfInitButtons = 0;
    Pixel fg, bg;

    XtVaGetValues(AnimationForm,
		  XmNforeground, &fg,
		  XmNbackground, &bg,
		  XmNdepth, &Depth,
		  NULL);

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = FRAC;
    ButtonDataArray[NumOfInitButtons].Top = FRAC - 5;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 6;
    ButtonDataArray[NumOfInitButtons].Right = FRAC ;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "dismiss");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Dismiss");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = FRAC;
    ButtonDataArray[NumOfInitButtons].Top = FRAC - 5;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 12;
    ButtonDataArray[NumOfInitButtons].Right = FRAC - 6;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "save-geom");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "No Geom");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = FRAC;
    ButtonDataArray[NumOfInitButtons].Top = FRAC - 5;
    ButtonDataArray[NumOfInitButtons].Left = 6;
    ButtonDataArray[NumOfInitButtons].Right = 12;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "save-image");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "No Image");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = FRAC;
    ButtonDataArray[NumOfInitButtons].Top = FRAC - 5;
    ButtonDataArray[NumOfInitButtons].Left = 0;
    ButtonDataArray[NumOfInitButtons].Right = 6;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "restart");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Restart");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = FRAC - 10;
    ButtonDataArray[NumOfInitButtons].Top = FRAC - 14;
    ButtonDataArray[NumOfInitButtons].Left = 1;
    ButtonDataArray[NumOfInitButtons].Right = 8;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "repeated");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Repeated");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = FRAC - 10;
    ButtonDataArray[NumOfInitButtons].Top = FRAC - 14;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 8;
    ButtonDataArray[NumOfInitButtons].Right = FRAC - 1;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "twoways");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Two ways");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm; 
    ButtonDataArray[NumOfInitButtons].Bot =  15;
    ButtonDataArray[NumOfInitButtons].Top =  10;
    ButtonDataArray[NumOfInitButtons].Left = 0; 
    ButtonDataArray[NumOfInitButtons].Right = 6; 
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "min-time");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Min\ntime");
    NumOfInitButtons++;
    
    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 15;
    ButtonDataArray[NumOfInitButtons].Top = 10;
    ButtonDataArray[NumOfInitButtons].Left = 6; 
    ButtonDataArray[NumOfInitButtons].Right = 12; 
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "max-time");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Max\ntime");
    NumOfInitButtons++;
    
    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 15;
    ButtonDataArray[NumOfInitButtons].Top = 10;
    ButtonDataArray[NumOfInitButtons].Left = 12;
    ButtonDataArray[NumOfInitButtons].Right = FRAC - 12;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "single-step");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Cont");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 15;
    ButtonDataArray[NumOfInitButtons].Top = 10;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 12; 
    ButtonDataArray[NumOfInitButtons].Right = FRAC - 6; 
    strcpy(ButtonDataArray[NumOfInitButtons].Name,  "interval-dt");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "dt");
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = PUSHB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 15;
    ButtonDataArray[NumOfInitButtons].Top = 10;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 6;
    ButtonDataArray[NumOfInitButtons].Right = FRAC;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "fast-interval-dt");
    strcpy(ButtonDataArray[NumOfInitButtons].Data.Label, "Fast\ndt");
    NumOfInitButtons++;
    
    ButtonDataArray[NumOfInitButtons].Kind = IMAGEB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 10;
    ButtonDataArray[NumOfInitButtons].Top = 5;
    ButtonDataArray[NumOfInitButtons].Left = 0;
    ButtonDataArray[NumOfInitButtons].Right = 6;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "rewind");
    ButtonDataArray[NumOfInitButtons].Data.BitMap =
	XCreatePixmapFromBitmapData(XtDisplay(IGTopLevel),
				    RootWindowOfScreen(XtScreen(IGTopLevel)),
				    (char *) RewBits, REW_WIDTH, REW_HEIGHT,
				    BlackPixelOfScreen(XtScreen(IGTopLevel)),
				    bg, Depth);
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = IMAGEB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 10;
    ButtonDataArray[NumOfInitButtons].Top = 5;
    ButtonDataArray[NumOfInitButtons].Left = 6;
    ButtonDataArray[NumOfInitButtons].Right = 12;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "play-backward");
    ButtonDataArray[NumOfInitButtons].Data.BitMap =
	XCreatePixmapFromBitmapData(XtDisplay(IGTopLevel),
				    RootWindowOfScreen(XtScreen(IGTopLevel)),
				    (char *) Play2Bits, PLAY2_WIDTH,
				    PLAY2_HEIGHT,
				    BlackPixelOfScreen(XtScreen(IGTopLevel)),
				    bg, Depth);
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = IMAGEB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 10;
    ButtonDataArray[NumOfInitButtons].Top = 5;
    ButtonDataArray[NumOfInitButtons].Left = 12;
    ButtonDataArray[NumOfInitButtons].Right = 18;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "stop");
    ButtonDataArray[NumOfInitButtons].Data.BitMap =
	XCreatePixmapFromBitmapData(XtDisplay(IGTopLevel),
				    RootWindowOfScreen(XtScreen(IGTopLevel)),
				    (char *) StopBits, STOP_WIDTH, STOP_HEIGHT,
				    BlackPixelOfScreen(XtScreen(IGTopLevel)),
				    bg, Depth);
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = IMAGEB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 10;
    ButtonDataArray[NumOfInitButtons].Top = 5;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 12;
    ButtonDataArray[NumOfInitButtons].Right = FRAC - 6;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "play");
    ButtonDataArray[NumOfInitButtons].Data.BitMap = 
	XCreatePixmapFromBitmapData(XtDisplay(IGTopLevel),
				    RootWindowOfScreen(XtScreen(IGTopLevel)),
				    (char *) PlayBits, PLAY_WIDTH, PLAY_HEIGHT,
				    BlackPixelOfScreen(XtScreen(IGTopLevel)),
				    bg, Depth); 
    NumOfInitButtons++;

    ButtonDataArray[NumOfInitButtons].Kind = IMAGEB;
    ButtonDataArray[NumOfInitButtons].Parent = AnimationForm;
    ButtonDataArray[NumOfInitButtons].Bot = 10;
    ButtonDataArray[NumOfInitButtons].Top = 5;
    ButtonDataArray[NumOfInitButtons].Left = FRAC - 6;
    ButtonDataArray[NumOfInitButtons].Right = FRAC;
    strcpy(ButtonDataArray[NumOfInitButtons].Name, "forward");
    ButtonDataArray[NumOfInitButtons].Data.BitMap =
	XCreatePixmapFromBitmapData(XtDisplay(IGTopLevel),
				    RootWindowOfScreen(XtScreen(IGTopLevel)),
				    (char *) FFBits, FF_WIDTH, FF_HEIGHT,
				    BlackPixelOfScreen(XtScreen(IGTopLevel)),
				    bg, Depth );
    NumOfInitButtons++;

    for (i = 0; i < NumOfInitButtons; i++) {
	VoidPtr TmpPtr;

	if (ButtonDataArray[i].Kind == PUSHB)
	    TmpPtr = (VoidPtr) (ButtonDataArray[i].Data.Label);
	else /* ButtonDataArray[i].Kind == IMAGEB */
	    TmpPtr = (VoidPtr) (ButtonDataArray[i].Data.BitMap);

        CreateButton(ButtonDataArray[i].Parent, i, ButtonDataArray[i].Top,
		     ButtonDataArray[i].Bot, ButtonDataArray[i].Left,
		     ButtonDataArray[i].Right, ButtonDataArray[i].Name,
		     ButtonDataArray[i].Kind,
		     TmpPtr);
    }

    SetLabel(TwowaysButton,
	     IGAnimation.TwoWaysAnimation ? "Two ways" : "One way");

    SetLabel(RepeatedButton,
	     IGAnimation.NumOfRepeat == 1 ? "Single" : "Repeated");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Creates one button.                                                      *
*                                                                            *
* PARAMETERS:                                                                *
*   w:          Class of widget.                                             *
*   index:      Into global (to this file) Buttons array.                    *
*   Top, Bot, Left, Right:  Location of button.                              *
*   Name:       Of button.                                                   *
*   Kind:       Push or Image.                                               *
*   Data:       Label.                                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void CreateButton(Widget w,
			 int index,
			 int Top,
			 int Bot,
			 int Left,
			 int Right,
			 char *Name,
			 ButtonKindType Kind,
			 VoidPtr Data)
{
    if (index < 0 || index >= BUTTONSNUM) {
	printf("CreateButton: index out of range\n");
	exit(1);
    }
    
    if (Kind == PUSHB) {
	AnimationButtons[index] =
	    XtVaCreateManagedWidget(Name,
				    xmPushButtonWidgetClass, w,
				    XmNpushButtonEnabled,    TRUE,
				    XmNtopAttachment,        XmATTACH_POSITION,
				    XmNtopPosition,          Top,
				    XmNtopOffset,	     OFFSET, 
				    XmNleftAttachment,       XmATTACH_POSITION,
				    XmNleftPosition,         Left,
				    XmNleftOffset,	     OFFSET, 
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        Right,
				    XmNrightOffset,	     OFFSET, 
				    XmNbottomAttachment,     XmATTACH_POSITION,
				    XmNbottomPosition,       Bot,
				    XmNbottomOffset,	     OFFSET, 
				    XtVaTypedArg,            XmNlabelString,
				    XmRString,		     (char *) Data,
						    strlen((char *) Data) + 1, 
				    XmNborderWidth,          2,
				    XmNshadowThickness,      5,
				    NULL);
	if (strcmp(Name, "repeated") == 0)
	    RepeatedButton = AnimationButtons[index];
	else if (strcmp(Name, "twoways") == 0)
	    TwowaysButton = AnimationButtons[index];
    }
    else if (Kind == IMAGEB) {
	AnimationButtons[index] =
	    XtVaCreateManagedWidget(Name,
				    xmPushButtonWidgetClass, w,
				    XmNpushButtonEnabled,    TRUE,     
				    XmNtopAttachment,        XmATTACH_POSITION,
				    XmNtopPosition,          Top,
				    XmNtopOffset,	     OFFSET,
				    XmNleftAttachment,       XmATTACH_POSITION,
				    XmNleftPosition,         Left,
				    XmNleftOffset,	     OFFSET,
				    XmNrightAttachment,      XmATTACH_POSITION,
				    XmNrightPosition,        Right,
				    XmNrightOffset,	     OFFSET,
				    XmNbottomAttachment,     XmATTACH_POSITION,
				    XmNbottomPosition,       Bot,
				    XmNbottomOffset,	     OFFSET,
				    XmNlabelType,            XmPIXMAP,
				    XmNlabelPixmap,          (Pixmap *) Data, 
				    XmNborderWidth,          2,
				    XmNshadowThickness,      5,
				    NULL);
    }

    XtAddCallback(AnimationButtons[index], XmNactivateCallback,
		  (XtCallbackProc) ButtonsCallBack, Name);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function of all buttons. Invokes the proper action.            *
*                                                                            *
* PARAMETERS:                                                                *
*   w:            Of button to handle.                                       *
*   ClientData:   To get the Name of button.                                 *
*   cbs:          Not used.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ButtonsCallBack(Widget w,
			    XtPointer ClientData,
			    XmPushButtonCallbackStruct *cbs)
{
    char
	*Name = (char *) ClientData;

    if (strcmp(Name, "dismiss") == 0)
	MovieDismissCb();
    else if (strcmp(Name, "save-geom") == 0)
	MovieSaveGeomCb(w);
    else if (strcmp(Name, "save-image") == 0)
	MovieSaveImageCb(w);
    else if (strcmp(Name, "interval-dt") == 0)
        IntervalDtCB(w);
    else if (strcmp(Name, "fast-interval-dt") == 0)
        FastIntervalDtCB(w);
    else if (strcmp(Name, "single-step") == 0)
        SingleStepCB(w);
    else if (strcmp(Name, "min-time") == 0)
        MinTimeCB(w);
    else if (strcmp(Name, "max-time") == 0)
        MaxTimeCB(w);
    else if (strcmp(Name, "play-backward") == 0)
        MoviePlayBackCb();
    else if (strcmp(Name, "rewind") == 0)
        MovieRewindCb();
    else if (strcmp(Name, "stop") == 0)
        MovieStopCb();
    else if (strcmp(Name, "play") == 0)
        MoviePlayCb();
    else if (strcmp(Name, "forward") == 0)
        MovieFastForwardCb();
    else if (strcmp(Name, "restart") == 0)
        MovieRestartCb();
    else if (strcmp(Name, "twoways") == 0)
        MovieTwoWaysCb();
    else if (strcmp(Name, "repeated") == 0)
        MovieRepeatedCb();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the interval button.                           *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:      widget.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void IntervalDtCB(Widget Parent)
{
    Widget DtDlg;
    XmString Msg, Title, Ok;
    char Buf[BUFSIZE];

    DtDlg = XmCreatePromptDialog(Parent, "Prompt", NULL, 0);
    Title = XmStringCreateSimple("New Dt");
    Msg = XmStringCreateSimple("Enter New Interval Dt Value:");
    Ok = XmStringCreateSimple("Yes");
    XtVaSetValues(DtDlg,
		  XmNdialogTitle,          Title,
		  XmNselectionLabelString, Msg,
		  XmNokLabelString,        Ok,
		  XmNnoResize,             True,
		  NULL);
    XmStringFree(Msg); 
    XmStringFree(Title);
    XmStringFree(Ok);
    XtUnmanageChild(XmSelectionBoxGetChild(DtDlg, XmDIALOG_HELP_BUTTON));

    XtAddCallback(DtDlg, XmNokCallback,
		  (XtCallbackProc) ReadValue, (VoidPtr) MOVIE_DT);

    sprintf(Buf, "Current Dt value is %0.4f", MovieDt); 
    Msg = XmStringCreateSimple(Buf);  
    XtVaCreateManagedWidget("CurrentValue",
			    xmLabelWidgetClass, DtDlg,
			    XmNlabelString,     Msg,
			    NULL);

    XmStringFree(Msg);
    
    XtManageChild(DtDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the fast interval button.                      *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:      widget.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void FastIntervalDtCB(Widget Parent)
{
    Widget FastDtDlg;
    XmString Msg, Title, Ok;
    char Buf[BUFSIZE];

    FastDtDlg = XmCreatePromptDialog(Parent, "Prompt", NULL, 0);
    Title = XmStringCreateSimple("New Fast Dt");
    Msg = XmStringCreateSimple("Enter New Fast Interval Dt Value");
    Ok = XmStringCreateSimple("Yes");
    XtVaSetValues(FastDtDlg,
		  XmNdialogTitle,          Title,
		  XmNselectionLabelString, Msg,
		  XmNokLabelString,        Ok,
		  XmNnoResize,             True,
		  NULL);
    XmStringFree(Msg);
    XmStringFree(Title);
    XmStringFree(Ok);
    XtUnmanageChild(XmSelectionBoxGetChild(FastDtDlg, XmDIALOG_HELP_BUTTON));

    XtAddCallback(FastDtDlg, XmNokCallback,
		  (XtCallbackProc) ReadValue, (VoidPtr) MOVIE_FAST_DT);

    sprintf(Buf, "Current Fast Dt is %0.4f", MovieFastDt);
    Msg = XmStringCreateSimple(Buf);
    XtVaCreateManagedWidget("CurrentFastValue",
			    xmLabelWidgetClass, FastDtDlg,
			    XmNlabelString,     Msg,
			    NULL);

    XmStringFree(Msg);

    XtManageChild(FastDtDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for single step button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void SingleStepCB(Widget Parent)
{
    if ((MovieSingleStep = !MovieSingleStep) != FALSE) {
	SetLabel(Parent, "SStp");
    }
    else {
	SetLabel(Parent, "Cont");
    }
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the minimum time button.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:      widget.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MinTimeCB(Widget Parent)
{
    XmString Msg, Title, Ok;
    Widget MinTimeDlg;
    char Buf[BUFSIZE];
    int ScaleValue;

    MinTimeDlg = XmCreatePromptDialog(Parent, "Prompt", NULL, 0);
    Title = XmStringCreateSimple("Update minimum time"); 
    Msg = XmStringCreateSimple("Enter new minimum time value\n");
    Ok = XmStringCreateSimple("Yes");
    XtVaSetValues(MinTimeDlg,
		  XmNselectionLabelString, Msg,
		  XmNdialogTitle,          Title,
		  XmNokLabelString,        Ok,
		  XmNnoResize,             True,
		  NULL);
    XmStringFree(Msg);
    XmStringFree(Title);
    XmStringFree(Ok);
    XtUnmanageChild(XmSelectionBoxGetChild(MinTimeDlg,
					   XmDIALOG_HELP_BUTTON));

    XtAddCallback(MinTimeDlg, XmNokCallback,
		  (XtCallbackProc) NewValue, (VoidPtr) MOVIE_MIN);

    XtVaGetValues(AnimationScale, XmNminimum, &ScaleValue, NULL);
    sprintf(Buf, "Minimum time is %0.4f", ScaleValue / SCALEMULT); 
    Msg = XmStringCreateSimple(Buf);  
    XtVaCreateManagedWidget("CurrentValue",
			    xmLabelWidgetClass, MinTimeDlg,
			    XmNlabelString,     Msg,
			    NULL);
    
    XmStringFree(Msg);

    XtManageChild(MinTimeDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   The call back function of the maximum time button.                       *
*                                                                            *
* PARAMETERS:                                                                *
*   Parent:      widget.                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MaxTimeCB(Widget Parent)
{
    XmString Msg, Title, Ok; 
    Widget MaxTimeDlg;
    char Buf[BUFSIZE];
    int ScaleValue;

    MaxTimeDlg = XmCreatePromptDialog(Parent, "Prompt", NULL, 0);
    Title = XmStringCreateSimple("Update maximum time"); 
    Msg = XmStringCreateSimple("Enter new maximum time value\n");
    Ok = XmStringCreateSimple("Yes");
    XtVaSetValues(MaxTimeDlg,
		  XmNselectionLabelString, Msg,
		  XmNdialogTitle,          Title,
		  XmNokLabelString,        Ok,
		  XmNnoResize,             True,
		  NULL);

    XmStringFree(Msg);
    XmStringFree(Title);
    XmStringFree(Ok);

    XtUnmanageChild(XmSelectionBoxGetChild(MaxTimeDlg,
					   XmDIALOG_HELP_BUTTON));
    
    XtAddCallback(MaxTimeDlg, XmNokCallback,
		  (XtCallbackProc) NewValue, (VoidPtr) MOVIE_MAX);

    XtVaGetValues(AnimationScale, XmNmaximum, &ScaleValue, NULL);

    sprintf(Buf, "Maximum time is %0.4f", ScaleValue / SCALEMULT);
    Msg = XmStringCreateSimple(Buf);
    XtVaCreateManagedWidget("CurrentValue",
			    xmLabelWidgetClass, MaxTimeDlg,
			    XmNlabelString,     Msg,
			    NULL);

    XmStringFree(Msg);

    XtManageChild(MaxTimeDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets one real value from user.                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   widget:       Not used.                                                  *
*   ClientData:   Is this for fast or regular motion!?                       *
*   CallData:     To get a handle on string.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void ReadValue(Widget widget,
		      XtPointer ClientData,
		      XtPointer CallData)
{
    XmSelectionBoxCallbackStruct
	*cbs = (XmSelectionBoxCallbackStruct *) CallData;
    char *Buf;
    double Value;

    if (!XmStringGetLtoR(cbs -> value, XmSTRING_DEFAULT_CHARSET, &Buf)) {
	printf("Can't convert compound String\n");
	exit(1);
    }

    if (IsNumber(Buf, &Value)) {
	if (((long) ClientData) == MOVIE_FAST_DT) {
	    MovieFastDt = Value;
	    MovieFastDtCb();
	}
	else {
	    MovieDt = Value;
	    MovieDtCb();
	}
    }
    else
        DisplayErrorMsg();
    XtFree(Buf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Gets one real value from user.                                           *
*                                                                            *
* PARAMETERS:                                                                *
*   widget:       Not used.                                                  *
*   ClientData:   Is this for fast or regular motion!?                       *
*   CallData:     To get a handle on string.                                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void NewValue(Widget widget,
		     XtPointer ClientData,
		     XtPointer CallData)
{
    int ScaleValue;
    XmSelectionBoxCallbackStruct
	*Cbs = (XmSelectionBoxCallbackStruct *) CallData;
    char *Buf;
    double Value;
    if (!XmStringGetLtoR(Cbs->value, XmSTRING_DEFAULT_CHARSET, &Buf)) {
	printf("Can't convert compound String\n");
	exit(1);
    }

    if (!IsNumber(Buf, &Value)) {
        DisplayErrorMsg();
    }
    else {
	Value = SCALEMULT * Value;
	if (Value != (int) Value) {
	    DisplayWrnMsg();
	    Value = (int) Value;
	}

	if (((long) ClientData) == MOVIE_MIN) {
	    XtVaGetValues(AnimationScale, XmNmaximum, &ScaleValue, NULL);
	    if (ScaleValue <= Value) 
	        DisplayErrValue("Max");
	    else {
		IGAnimation.StartT = Value / SCALEMULT;
		if (IGAnimation.RunTime < IGAnimation.StartT) {
		    IGAnimation.RunTime = IGAnimation.StartT;
		    XtVaSetValues(AnimationScale, XmNvalue, (int) Value, NULL);
		}
		XtVaSetValues(AnimationScale, XmNminimum, (int) Value, NULL);
	    }
	}
	else {
	    XtVaGetValues(AnimationScale, XmNminimum, &ScaleValue, NULL);
	    if (ScaleValue >= Value)
	        DisplayErrValue("Min");
	    else {
		IGAnimation.FinalT = Value / SCALEMULT;
		if (IGAnimation.RunTime > IGAnimation.FinalT) {
		    IGAnimation.RunTime = IGAnimation.FinalT;
		    XtVaSetValues(AnimationScale, XmNvalue, (int) Value, NULL);
		}
		XtVaSetValues(AnimationScale, XmNmaximum, (int) Value, NULL);
	    }
	}
    }
    XtFree(Buf);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Error messages for input of numbers.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DisplayErrorMsg(void)
{
    XmString Text;
    Widget ErrDlg;

    ErrDlg = XmCreateErrorDialog(IGTopLevel, "Error", NULL, 0);
    Text = XmStringCreateSimple("Value is not a number.");
    XtVaSetValues(ErrDlg,
		  XmNmessageString,     Text,
		  XmNdefaultButtonType, XmDIALOG_OK_BUTTON,
		  XmNnoResize,          True,
		  NULL);
    XmStringFree(Text);
    XtUnmanageChild(XmMessageBoxGetChild(ErrDlg, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(ErrDlg, XmDIALOG_HELP_BUTTON));
    XtManageChild(ErrDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Warning messages for input of numbers.                                   *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DisplayWrnMsg(void)
{
    XmString Text;
    Widget WrnDlg;

    WrnDlg = XmCreateWarningDialog(IGTopLevel, "warning", NULL, 0);
    Text = XmStringCreateSimple("Only 2 digits after decimal point. Value will be rounded.");
    XtVaSetValues(WrnDlg,
		  XmNmessageString,     Text,
		  XmNdefaultButtonType, XmDIALOG_OK_BUTTON,
		  XmNnoResize,          True,
		  NULL);
    XmStringFree(Text);
    XtUnmanageChild(XmMessageBoxGetChild(WrnDlg, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(WrnDlg, XmDIALOG_HELP_BUTTON));
    XtManageChild(WrnDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Error messages for input of numbers.                                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Msg:   To print.                                                         *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DisplayErrValue(char *Msg)
{
    Widget ErrDlg;
    XmString Text;

    ErrDlg = XmCreateErrorDialog(IGTopLevel, "Error", NULL, 0);
    if (strcmp(Msg, "Max") == 0)
        Text = XmStringCreateSimple("Maximum time >= new given minimum");
    else
        Text = XmStringCreateSimple("Minimum time <= new given maximum");

    XtVaSetValues(ErrDlg,
		  XmNmessageString,     Text,
		  XmNdefaultButtonType, XmDIALOG_OK_BUTTON,
		  XmNnoResize,          True,
		  NULL);
    XmStringFree(Text);
    XtUnmanageChild(XmMessageBoxGetChild(ErrDlg, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(ErrDlg, XmDIALOG_HELP_BUTTON));
    XtManageChild(ErrDlg);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Verifies and converts a string into a numeric value.                     *
*                                                                            *
* PARAMETERS:                                                                *
*   Buf:    To convert to a number.                                          *
*   Value:  Numeric result.                                                  *
*                                                                            *
* RETURN VALUE:                                                              *
*   int:   TRUE is is a number, FALSE otherwise.                             *
*****************************************************************************/
static int IsNumber(char *Buf, double *Value)
{
    return sscanf(Buf, "%lf", Value);
}

/*****************************************************************************
* DESCRIPTION:                                                               M
*   Should we stop this animation? Senses the event queue of X.              M
*                                                                            *
* PARAMETERS:                                                                M
*   Anim:     The animation to abort.                                        M
*                                                                            *
* RETURN VALUE:                                                              M
*   int:      TRUE if we needs to abort, FALSE otherwise.                    M
*                                                                            *
* KEYWORDS:                                                                  M
*   GMAnimCheckInterrupt                                                     M
*****************************************************************************/
int GMAnimCheckInterrupt(GMAnimationStruct *Anim)
{
    XEvent Event;
    Display
	*XDpy = XtDisplay(IGTopLevel);

    XFlush(XDpy);
    XmUpdateDisplay(IGTopLevel);
    while (XCheckMaskEvent(XDpy,
			   ButtonPressMask |
			   ButtonReleaseMask |
			   ButtonMotionMask |
			   PointerMotionMask |
			   KeyPressMask |
			   KeyReleaseMask,
			   &Event))
	XtDispatchEvent(&Event);

    return Anim -> StopAnim;
}

/*****************************************************************************
* DESCRIPTION:								     M
*   Getting input values from the user.					     M
*									     *
* PARAMETERS:								     M
*   None                                 				     M
*									     *
* RETURN VALUE:								     M
*   void 								     M
*                                                                            *
* KEYWORDS:                                                                  M
*   AnimationCB                                                              M
*****************************************************************************/
void AnimationCB(void)
{  
    Position MainWindowX, MainWindowY;
    Dimension MainWindowW;
  
    XtVaGetValues(IGTopLevel,
		  XmNwidth,	&MainWindowW,
		  XmNx,		&MainWindowX,
		  XmNy,		&MainWindowY,
		  NULL);
    XtVaSetValues(AnimationForm,
		  XmNdefaultPosition, FALSE,
		  XmNx,		      MainWindowX + MainWindowW + 16,
		  XmNy,		      MainWindowY,
		  NULL);
    XtManageChild(AnimationForm);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Executes a single animation step according to Anim info an PObj geom.    *
*                                                                            *
* PARAMETERS:                                                                *
*   Anim:    Prescription of the animation step to perform.                  *
*   PObj:    Current geometry to display.                                    *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void DoSingleStepAnimation(GMAnimationStruct *Anim,
				  IPObjectStruct *PObj)
{
    IGAnimation.StopAnim = FALSE;

    GMAnimDoSingleStep(Anim, PObj);

    if (IGAnimation.RunTime > IGAnimation.FinalT)
	IGAnimation.RunTime = IGAnimation.FinalT;
    if (IGAnimation.RunTime < IGAnimation.StartT)
	IGAnimation.RunTime = IGAnimation.StartT;

    XmScaleSetValue(AnimationScale, (int) (IGAnimation.RunTime * SCALEMULT));
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for play back button.                                 *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MoviePlayBackCb(void)
{
    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime -= MovieDt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

        for ( ;
	     IGAnimation.RunTime >= IGAnimation.StartT + IRIT_EPS &&
	     !IGAnimation.StopAnim;
	     IGAnimation.RunTime -= MovieDt) {
    	    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	}
    }

    IGGlblAnimation = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for rewind button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieRewindCb(void)
{
    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime -= MovieFastDt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

        for ( ;
	     IGAnimation.RunTime >= IGAnimation.StartT + IRIT_EPS &&
	     !IGAnimation.StopAnim;
	     IGAnimation.RunTime -= MovieFastDt) {
    	    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	}
    }

    IGGlblAnimation = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for rewind button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   w:            Of button to handle.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieSaveGeomCb(Widget w)
{
    IGAnimation.SaveAnimationGeom = !IGAnimation.SaveAnimationGeom;
    SetLabel(w, IGAnimation.SaveAnimationGeom ? "Sv Geom" : "No Geom");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for rewind button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   w:            Of button to handle.                                       *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieSaveImageCb(Widget w)
{
    IGAnimation.SaveAnimationImage = !IGAnimation.SaveAnimationImage;
    SetLabel(w, IGAnimation.SaveAnimationImage ? "Sv Image" : "No Image"); 
    IGGlblImageFileName = "IAnim%04d.ppm";
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for stop button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieStopCb(void) 
{
    IGAnimation.StopAnim = TRUE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for play button. 	                             *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MoviePlayCb(void) 
{
    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime += MovieDt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

	do {
	    for ( ;
		 IGAnimation.RunTime <= IGAnimation.FinalT + IRIT_EPS &&
		 !IGAnimation.StopAnim;
		 IGAnimation.RunTime += MovieDt) {
	        DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	    }

	    if (IGAnimation.TwoWaysAnimation) {
	        for ( ;
		     IGAnimation.RunTime >= IGAnimation.StartT - IRIT_EPS &&
		     !IGAnimation.StopAnim;
		     IGAnimation.RunTime -= MovieDt) {
		    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
		}
	    }

	    if (IGAnimation.NumOfRepeat > 1) {
	        IGAnimation.NumOfRepeat--;
		IGAnimation.RunTime = IGAnimation.StartT;
	    }
	}
	while (IGAnimation.NumOfRepeat > 1 && !IGAnimation.StopAnim);
    }

    IGGlblAnimation = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for fast forward button. 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieFastForwardCb(void) 
{
    IGGlblAnimation = TRUE;

    if (MovieSingleStep) {
	IGAnimation.RunTime += MovieFastDt;
	DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
    }
    else {
	IGAnimation.StopAnim = FALSE;

        for ( ;
	     IGAnimation.RunTime <= IGAnimation.FinalT + IRIT_EPS &&
	     !IGAnimation.StopAnim;
	     IGAnimation.RunTime += MovieFastDt) {
    	    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
	}
    }

    IGGlblAnimation = FALSE;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for restart button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieRestartCb(void) 
{
    IGAnimation.RunTime = IGAnimation.StartT;
    MoviePlayCb();
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for repeated button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieRepeatedCb(void)
{
    if (IGAnimation.NumOfRepeat == 1)
        IGAnimation.NumOfRepeat = 32767;
    else
        IGAnimation.NumOfRepeat = 1;

    SetLabel(RepeatedButton,
	     IGAnimation.NumOfRepeat == 1 ? "Single" : "Repeated");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for two ways button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieTwoWaysCb(void) 
{
    IGAnimation.TwoWaysAnimation = !IGAnimation.TwoWaysAnimation;

    SetLabel(TwowaysButton,
	     IGAnimation.TwoWaysAnimation ? "Two ways" : "One way");
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for dismiss button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieDismissCb(void) 
{
    if (XtIsManaged(AnimationForm))
        XtUnmanageChild(AnimationForm);
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for set fast Dt button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieFastDtCb(void)
{
    IGAnimation.Dt = MovieFastDt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for set Dt button.	 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   None                                                                     *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieDtCb(void)
{
    IGAnimation.Dt = MovieDt;
}

/*****************************************************************************
* DESCRIPTION:                                                               *
*   Call back function for set scale animation value. 	                     *
*                                                                            *
* PARAMETERS:                                                                *
*   ScaleValue:     Value of scale to update animation with.                 *
*                                                                            *
* RETURN VALUE:                                                              *
*   void                                                                     *
*****************************************************************************/
static void MovieScaleCb(float ScaleValue)
{
    IGAnimation.RunTime = ScaleValue;

    DoSingleStepAnimation(&IGAnimation, IGGlblDisplayList);
}
