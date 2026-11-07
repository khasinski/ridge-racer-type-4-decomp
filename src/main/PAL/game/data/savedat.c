/* RODATA 0x800114a0 0xc4 */
#include "game/gfx/color.h"
#include "psyq/libc/printf.h"
#include "game/data/savedat.h"
#include "game/data/savetype.h"
#include "rr/menu_selection.h"
#include "rr/overlay662_frame_step.h"
#include "rr/overlay668_playback_step.h"
#include "game/race/raceset.h"
typedef struct { char b[0x80]; } B;

extern int CopyStringClearTail();
extern Struct48 CourseChoiceTable;
extern SaveBuf *SaveBuffer;
extern int CourseBestLap[2][8];
extern Struct40 Options;
extern Struct10 RaceRecords[2][8][5];
extern Struct2C SaveSlotSelections[3];
extern int CourseBestLapTimes[2][8][3];
extern Struct840 *D_800F3910;
extern Struct840 *D_800F3918;
extern Struct7C SaveProgress;
extern int SaveDataBackup[];
extern short SaveSlotIndex;
extern short RaceType;
extern Struct10 LapRecords[2][8][5];
extern Struct2C SaveSlotSelectionsBackup[3];
extern Struct48 CourseMenuChoices[2];
extern StructC PlayerNames[];
extern StructC D_80118420[];
extern Struct48 D_80118458[2];
extern int D_801272F0[];
extern int StoreImage();
extern char StrFmtPercentS[];
extern short SaveStoreRect[];
extern char StrCrd0[];
extern short SaveIconRect[];

void BuildCardHeaderAnim();

int ApplySaveData(int arg0) {
    int a2 = 0;
    SaveBuf *s3;
    SaveBuf11 *p;
    int a1;
    int s0;
    int t0;
    int k;
    int v10;
    int v11;

    switch (arg0) {
    case 0:
    case 1:
    case 0x30:
    default:
        s3 = SaveBuffer;
        a1 = 0;
        for (s0 = 0; (unsigned int)s0 < 0x6A9; s0++) {
            a1 += ((int *)s3)[s0];
        }
        if (s3->f1AA4 == a1) {
            SaveProgress = s3->f280;
            Options = s3->f2FC;
            for (s0 = 0; s0 < 3; s0++) {
                if (arg0 == 0x30) {
                    SaveSlotSelections[s0] = s3->f360[s0];
                } else {
                    SaveSlotSelectionsBackup[s0] = s3->f360[s0];
                }
            }
            for (s0 = 0; s0 < 9; s0++) {
                CourseChoiceTable.f24[s0] = s3->f33C[s0];
            }
            for (s0 = 0; s0 < 2; s0++) {
                for (t0 = 0; t0 < 8; t0++) {
                    for (k = 0; k < 3; k++) {
                        CourseBestLapTimes[s0][t0][k] = s3->f3E4[s0][t0][k];
                    }
                    for (k = 0; k < 5; k++) {
                        LapRecords[s0][t0][k] = s3->f4A4[s0][t0][k];
                        RaceRecords[s0][t0][k] = s3->f9A4[s0][t0][k];
                    }
                    CourseBestLap[s0][t0] = LapRecords[s0][t0][0].w[3];
                    CourseBestTotal[s0][t0] = RaceRecords[s0][t0][0].w[3];
                }
            }
            for (s0 = 0; s0 < 0x210; s0++) {
                SaveDataBackup[s0] = s3->fEA4[s0];
            }
            ApplySoundVolumes();
            a2 = 1;
        }
        break;
    case 2:
        s3 = SaveBuffer;
        a1 = 0;
        for (s0 = 0; (unsigned int)s0 < 0x6A9; s0++) {
            a1 += ((int *)s3)[s0];
        }
        if (s3->f1AA4 == a1) {
            for (s0 = 0; s0 < 9; s0++) {
                if (RaceType == 2 || RaceType == 3) {
                    D_80118458[SelectionPlayer & 1].f24[s0] = s3->f33C[s0];
                    CopyStringClearTail(&D_80118420[SelectionPlayer & 1], &s3->f280);
                } else if (RaceType == 4) {
                    CourseMenuChoices[SelectionPlayer & 1].f24[s0] = s3->f33C[s0];
                    CopyStringClearTail(&PlayerNames[SelectionPlayer], &s3->f280);
                }
            }
            for (s0 = 0; s0 < 0x210; s0++) {
                *(int *)((SelectionPlayer & 1) * 0x840 + (s0 << 2) + (int)SaveDataBackup) = s3->fEA4[s0];
            }
            a2 = 1;
        }
        break;
    case 0x10:
        v10 = *(int *)SaveBuffer;
        D_801272F0[SaveSlotIndex] = v10;
        a2 = 1;
        break;
    case 0x11:
        v11 = (p = (SaveBuf11 *)SaveBuffer)->f280;
        PlayTimeFrames = D_801272F0[SaveSlotIndex] = v11;
        for (t0 = 0; t0 < 2; t0++) {
            for (s0 = 0; s0 < 0x840; s0++) {
                if (t0 == 0) {
                    D_800F3918->b[s0] = p->f300[0][s0];
                } else {
                    D_800F3910->b[s0] = p->f300[t0][s0];
                }
            }
        }
        a2 = 1;
        break;
    }
    return a2;
}

void BuildCardHeaderAnim(a0, a1, a2, a3, a4, a5)
char *a0;
int a1;
short a2;
short a3;
int a4;
int a5;
{
    int i;
    int *new_var;
    a0[0] = 0x53;
    a0[1] = 0x43;
    a0[2] = 0x13;
    a0[3] = 1;
    PsyqSprintf(a0 + 4, StrFmtPercentS, a1);
    SaveStoreRect[0] = a2;
    SaveStoreRect[2] = 0x10;
    SaveStoreRect[1] = a3;
    SaveStoreRect[3] = 1;
    StoreImage(SaveStoreRect, a0 + 0x60);
    DrawSync(0);
    i = 0;
    do {
        SaveStoreRect[0] = a4;
        SaveStoreRect[1] = a5;
        SaveStoreRect[2] = 4;
        SaveStoreRect[3] = 0x10;
        /* (new_var = &a4) is a dead address-of that keeps gcc from hoisting the li 0x10 out of the loop (found by permuter) */
        StoreImage(SaveStoreRect, a0 + 0x80 + (*(new_var = &a4), i) * 0x80);
        DrawSync(0);
        i++;
        a4 += 4;
    } while (i < 3);
}

void BuildCardHeaderStatic(a0, a1, a2, a3, a4, a5, a6)
unsigned char *a0;
int a1;
short a2;
unsigned short a3;
int a4;
int a5;
B *a6;
{
    int i;
    a0[0] = 0x53;
    a0[1] = 0x43;
    a0[2] = 0x11;
    a0[3] = 1;
    PsyqSprintf(a0 + 4, StrFmtPercentS, a1);
    a0[0x50] = 1;
    a0[0x51] = 0;
    PsyqSprintf(a0 + 0x52, StrFmtPercentS, StrCrd0);
    SaveIconRect[0] = a2;
    SaveIconRect[2] = 0x10;
    SaveIconRect[1] = a3;
    SaveIconRect[3] = 1;
    StoreImage(SaveIconRect, a0 + 0x60);
    DrawSync(0);
    i = 0;
    do {
        SaveIconRect[0] = a4;
        SaveIconRect[1] = a5;
        SaveIconRect[2] = 4;
        SaveIconRect[3] = 0x10;
        StoreImage(SaveIconRect, a0 + 0x80 + i * 0x80);
        DrawSync(0);
        i++;
        a4 += 4;
    } while (i <= 0);
    *(B *)(a0 + 0x100) = *a6;
    *(B *)(a0 + 0x180) = *a6;
    *(B *)(a0 + 0x200) = *a6;
}
