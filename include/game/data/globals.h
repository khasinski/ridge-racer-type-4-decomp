#ifndef RR_GAME_DATA_GLOBALS_H
#define RR_GAME_DATA_GLOBALS_H

/* Scalar globals shared by several units: one declaration each. */
#include "types.h"

extern int BackColorR;
extern s32 CurrentViewIndex;
extern int SaveBuffer;
extern signed char D_800AD4EF;
extern int DefaultSegmentOctantValue;
extern unsigned short PendingModeId;
extern short MainMenuLastIndex;
extern s32 RearViewMirrorOffsetY;
extern int FarColorB;
extern int FarColorG;
extern int FarColorR;
extern int BackColorB;
extern int BackColorG;
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_orev1;
extern u16 _svm_orev2;
extern u16 _svm_onos1;
extern u16 _svm_onos2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern s16 _svm_stereo_mono;
extern s8 _svm_auto_kof_mode;
extern int StStartFrame;

#endif
