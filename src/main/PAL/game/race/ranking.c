#include "types.h"
#include "m2c_macros.h"
#include "game/data/ldscn1.h"
#include "game/gfx/poly1.h"
#include "game/menu/menutra21.h"
#include "game/demo/splash.h"
#include "rr/bios_devices.h"
#include "rr/audio_cd_controller.h"
#include "rr/driver_control.h"
#include "game/gfx/color.h"
#include "game/race/raceset.h"
#include "game/system/modeinit.h"
#include "game/data/globals.h"

typedef struct {
    char name[10];
    unsigned short car;
} StructC;

typedef struct {
    char name[10];
    unsigned short car;
    int time;
} Struct10;

extern int DefaultCourseTimes[2][8];
extern StructC DefaultRecordEntries[5];
extern unsigned char CourseLapCounts[2][8];
extern int CourseBestLap[2][8];
extern Struct10 RaceRecords[2][8][5];
extern int CourseBestLapTimes[2][8][3];
extern int CourseBestLapRecords[8];
extern Struct10 LapRecords[2][8][5];
extern char CourseChoiceTable[];
extern unsigned short DefaultCourseChoiceValues[];
extern unsigned char SaveDataWork[];
extern unsigned char SaveDataBackup[];
/* extern */
/* extern */
/* extern */
/* extern */
extern s32 CdDirectoryBuffer;
extern M2C_UNK StrTitle0Newline;
extern s32 VSyncThreshold;
extern s32 LoadCursor;
extern s32 ModeFrame;
extern s16 ModeId;
extern s16 ModeGroup;
extern s8 LoadState;
extern s16 ModeSubState;
extern s16 MenuResultCode;
extern int SetCdMixMode();

void InitDefaultTimeRecords(void);
void InitCourseChoices(void);
void BackupSaveData(void);
void RestoreSaveData(void);
void MenuSceneNopEntry(void);
void SplashModeEnter(void);
void SplashModeUpdate(void);
void DrawTextStub(void);

void InitDefaultTimeRecords(void) {
    int i, j, k;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 8; j++) {
            int c = 0x2FD;
            CourseBestLapRecords[j] = DefaultCourseTimes[0][j];
            CourseBestTotalTimes[j] = (DefaultCourseTimes[0][j] - 0x2FD) * CourseLapCounts[1][j] + (CourseLapCounts[1][j] * 1000 + c);
            CourseBestLap[i][j] = DefaultCourseTimes[1][j];
            CourseBestTotal[i][j] = (DefaultCourseTimes[1][j] - 0x2FD) * CourseLapCounts[0][j] + (CourseLapCounts[0][j] * 1000 + c);
            for (k = 0; k < 3; k++) {
                CourseBestLapTimes[i][j][k] = DefaultCourseTimes[1][j];
            }
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < 8; j++) {
            for (k = 0; k < 5; k++) {
                *(int *)&LapRecords[i][j][k].name[0] = *(int *)&DefaultRecordEntries[k].name[0];
                *(int *)&LapRecords[i][j][k].name[4] = *(int *)&DefaultRecordEntries[k].name[4];
                *(int *)&RaceRecords[i][j][k].name[0] = *(int *)&DefaultRecordEntries[k].name[0];
                *(int *)&RaceRecords[i][j][k].name[4] = *(int *)&DefaultRecordEntries[k].name[4];
                LapRecords[i][j][k].time = CourseBestLap[i][j] + k * 3000;
                {
                    int t = k * 3000;
                    RaceRecords[i][j][k].time = CourseBestTotal[i][j] + t * CourseLapCounts[0][j];
                }
                LapRecords[i][j][k].car = DefaultRecordEntries[k].car;
                RaceRecords[i][j][k].car = DefaultRecordEntries[k].car;
            }
        }
    }
}

void InitCourseChoices(void)
{
    /* ASM_BARRIER: register pins reproduce original t/a register allocation */
    register int a2 asm("$6");
    register int a3 asm("$7");
    int t1;
    int t2;
    register int t4 asm("$12");
    register int e asm("$13");
    unsigned short *t6;
    register unsigned short *t0 asm("$8");
    short *t3;
    char *a1;
    unsigned short u;
    PsyqBzero(CourseChoiceTable, 0x90);
    PsyqBzero(CourseChoiceTable + 0x24, 0x90);
    t4 = 0;
    e = 8;
    t6 = DefaultCourseChoiceValues;
    t2 = 0x20;
    t1 = t4;
    do {
        a2 = 0;
        t3 = (short *)(t2 + CourseChoiceTable);
        t0 = t6;
        a3 = t1;
loop:
        a1 = a3 + CourseChoiceTable;
        if (a2 == e) {
            int m;
            __asm__ volatile("" : "=r"(m) : "0"(-1)); /* ASM_BARRIER: keep -1 inside branch */
            *t3 = m;
        } else {
            u = *t0;
            a1[3] = (a2 / 2) + (a2 < 4);
            *(unsigned short *)a1 = u;
        }
        t0++;
        a2++;
        a3 += 4;
        if (a2 < 9) goto loop;
        t2 += 0x24;
        t4++;
        t1 += 0x24;
    } while (t4 < 4);
}

void BackupSaveData(void) {
    int i;
    for (i = 0; i < 0x840; i++) {
        SaveDataBackup[i] = SaveDataWork[i];
    }
}

void RestoreSaveData(void) {
    int i;
    for (i = 0; i < 0x840; i++) {
        SaveDataWork[i] = SaveDataBackup[i];
    }
}

void MenuSceneNopEntry(void) {
}

void SplashModeEnter(void) {
    int pad[6];
    SetDispMask(0);
    DebugPrintStub(&StrTitle0Newline);
    LoadCursor = CdDirectoryBuffer;
    SplashLogoInit();
    MenuResultCode = 0;
    LoadSaveFromCardStep(0);
    VSyncThreshold = 0x80;
    ModeFrame = 0;
    ModeSubState = 0;
    LoadState = 0;
    AttractCycleCounter = 0;
    PendingModeId = 0x203;
    ModeGroup = 1;
    ModeId = 2;
    ModeSubState = 0;
}

/* legacy typing of shared globals in this part of the unit */
#define LoadSaveFromCardStep ((int (*)())LoadSaveFromCardStep)
#define PendingModeId (*(u16 (*))&PendingModeId)

void SplashModeUpdate(void) {
    s32 temp_v0;

    temp_v0 = LoadSplashAssetsStep();
    if (temp_v0 == 1) {
        ModeFrame += 1;
        if (SplashLogoUpdate(LoadSaveFromCardStep(1) == 1) == temp_v0) {
            SetCdMixMode(SoundMonoFlag);
            ModeSubState = 0;
            ModeGroup = (s16) ((u16) PendingModeId >> 8);
            ModeId = PendingModeId & 0xFF;
        }
    }
}

#undef LoadSaveFromCardStep
#undef PendingModeId

void DrawTextStub(void){ int a[6];
/* ASM_BARRIER: empty-frame function needs jr/addiu sp delay-slot epilogue form */
asm volatile("" ::: "memory"); }
