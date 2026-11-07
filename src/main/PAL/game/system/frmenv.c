#include "rr/frame_environment.h"
#include "rr/frame_loop.h"
#include "game/car/carstat.h"
#include "game/data/cardini.h"
#include "game/data/cdfile1.h"
#include "game/data/trkld8.h"
#include "game/gfx/fadespr.h"
#include "game/gfx/model.h"
#include "game/menu/menu8.h"
#include "game/race/racesamp.h"
#include "game/race/ranking.h"
#include "psyq/libgpu/gpuenv.h"
#include "psyq/libsio/siostart.h"
#include "psyq/libspu/sinit2.h"
#include "game/data/ldscn1.h"
#include "game/menu/menusave.h"
#include "game/system/frmenv.h"
#include "psyq/libgpu/gpuenv.h"
#include "rr/audio_cd_controller.h"
#include "rr/library.h"
#include "rr/driver_control.h"
#include "rr/bios_devices.h"
#include "game/data/selection.h"

extern int CdAudioInit();
extern int InitGeom();
extern int ApplyOptionsPadButtonMaps();
extern int SetupRenderer();
extern int CopyStringClearTail();
extern int InitMenuArenas();
extern int CdDirectoryBuffer;
extern char StrTotalInitLog[];
extern char DefaultPlayerName[];
extern char TeamNameDigRacingTeam[];
extern char TeamAbbrevDRT[];
extern char TeamNameMicroMouseMappy[];
extern char TeamAbbrevMicroMouseMappy[];
extern char StrPacRacingClub[];
extern char TeamAbbrevPRC[];
extern char TeamNameSolvalu[];
extern char TeamAbbrevRTS[];
extern char TeamNameAssoluto[];
extern char TeamNameLizard[];
extern char StrTerrazi[];
extern char StrAgeSolo[];
extern char PlayerCars[];
extern int LoadCursor;
extern int MirrorMode;
extern signed char SelectionState[];
extern short LinkRoleSlot;
extern char RankEntries[];
extern char SaveDataWork[];
extern char SaveProgress[];
extern char PlayerNameSlots[];
extern char SaveDataBackup[];
extern signed char SaveSlotSelections[];
extern int RankTable[];
extern int TeamStringPtrTable[];
extern int RivalTeamNamePtrTable[];
extern int NumLaps;
extern int D_800F4C24;
extern int D_800F4C2C;
extern int DialogueContext;
extern int MenuArenaBase;
extern short RaceType;
extern char *DefaultCarClut;
extern void SetGeomOffset(s32,s32);
extern void SetGeomScreen(s32);
extern void *SetDefDispEnv(void *,s32,s32,s32,s32);

void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags);

void SystemInit(void)
{
    int i;
    int *p;
    char *s, *q;
    signed char *r;
    int k, c5;
    int m;

    DebugPrintStub(StrTotalInitLog);
    CdInit();
    InitCdFileDirectory();
    SsInit();
    SoundSystemInit();
    CdAudioInit();
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
    InitGeom();
    CardSystemInit(1);
    InitGamePads();
    InitOptionsDefaults();
    ApplyOptionsPadButtonMaps();
    AddSIO();
    _sio_control(1, 1, 0);
    SetupRenderer(5);
    VSyncThreshold = 0x80;
    D_800F4C24 = 0;
    D_800F4C2C = 0;
    DrawSubOtFlag = 0;
    NumLaps = 2;
    MirrorMode = 0;
    PsyqBzero(SaveProgress, 0x7C);
    PsyqBzero(SelectionRows, 0x20);
    PsyqBzero(SelectionState, 0x2C);
    SelectionState[0x17] = -1;
    PsyqBzero(RankEntries, 0x40);
    i = 7;
    do {
        RankTable[i] = (int)(RankEntries + i * 8);
        i--;
    } while (i >= 0);
    PsyqBzero(PlayerCars, 0x320);
    PsyqBzero(PlayerCars + 0x320, 0x320);
    SaveNoticeFlags = 0;
    i = 0;
    c5 = 5;
    q = SaveProgress;
    do {
        *(volatile char *)(q + 0x6C) = c5;
        *(volatile char *)(q + 0x6D) = c5;
        *(volatile char *)(q + 0x6E) = 0;
        __asm__ volatile("" : : : "memory"); /* ASM_BARRIER: keep i++ after the byte stores like the original schedule */
        i++;
        q += 4;
    } while (i < 4);
    LinkRoleSlot = -1;
    CopyStringClearTail(SaveProgress, DefaultPlayerName);
    CopyStringClearTail(PlayerNameSlots, SaveProgress);
    CopyStringClearTail(PlayerNameSlots + 0xC, SaveProgress);
    CopyStringClearTail(PlayerNameSlots + 0x18, SaveProgress);
    CopyStringClearTail(PlayerNameSlots + 0x24, SaveProgress);
    TeamStringPtrTable[0] = (int)TeamNameDigRacingTeam;
    TeamStringPtrTable[4] = (int)TeamAbbrevDRT;
    TeamStringPtrTable[1] = (int)TeamNameMicroMouseMappy;
    TeamStringPtrTable[5] = (int)TeamAbbrevMicroMouseMappy;
    TeamStringPtrTable[2] = (int)StrPacRacingClub;
    TeamStringPtrTable[6] = (int)TeamAbbrevPRC;
    TeamStringPtrTable[3] = (int)TeamNameSolvalu;
    TeamStringPtrTable[7] = (int)TeamAbbrevRTS;
    RivalTeamNamePtrTable[0] = (int)TeamNameAssoluto;
    RivalTeamNamePtrTable[1] = (int)TeamNameLizard;
    RivalTeamNamePtrTable[2] = (int)StrTerrazi;
    RivalTeamNamePtrTable[3] = (int)StrAgeSolo;
    LoadCursor = CdDirectoryBuffer;
    MenuArenaBase = CdDirectoryBuffer;
    InitMenuArenas();
    k = 0xAF000;
    m = -1;
    i = 2;
    DialogueContext = 0;
    RaceType = 0;
    LoadDestCursor = CdDirectoryBuffer + k;
    r = SaveSlotSelections + i * 0x2C;
    do {
        r[0x17] = m;
        i--;
        r -= 0x2C;
    } while (i >= 0);
    InitDefaultTimeRecords(k, i, r, m);
    InitCourseChoices();
    InitTimTemplate(SaveDataBackup);
    InitTimTemplate(SaveDataBackup + 0x840);
    InitTimTemplate(SaveDataWork);
    DefaultCarClut = SaveDataBackup;
    ResetRankEntries();
    InitCarTable(1);
    ResetGraph(3);
}

void func_8001ECE0(s32 bank) {
}

void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags) {
    /* ASM_BARRIER: keep flags in S0 until the arena replaces their dead value. */
    register u32 input asm("$16")=flags;
    /* ASM_BARRIER: retain red in the original saved register across environment helpers. */
    register s32 red asm("$23")=r;
    register s32 green;
    register u32 temp;
    /* ASM_BARRIER: retain interlace independently of height and offset. */
    register s32 interlace asm("$19");
    register u32 height;
    register u32 other_y;
    register s32 width;
    /* ASM_BARRIER: retain the wide-mode bit across environment helpers. */
    register u32 wide asm("$21");
    /* ASM_BARRIER: preserve flags until the RGB24 loop value is extracted. */
    register u32 saved_flags asm("$22");
    /* ASM_BARRIER: reuse S0 for the arena after the input flags have been consumed. */
    register RrFrameStorage *env asm("$16");
    RrGpuRectangleFields window;
    register s32 i;
    register u16 *settings;
    register s32 one;
    /* ASM_BARRIER: keep the loop wide-mode value in A3 after the helper calls. */
    register u32 loop_wide asm("$7");
    /* ASM_BARRIER: keep the loop RGB24 value in T0 after the helper calls. */
    register u32 rgb24 asm("$8");
    register s32 center_x;
    register u32 center_y;
    /* ASM_BARRIER: capture red and flags before assigning green and geometry temporaries. */
    __asm__("" : "+r"(red), "+r"(input) : : "$2", "$30");
    green=g;
    center_x=160;
    /* ASM_BARRIER: capture green and the X center before using V0 for geometry. */
    __asm__("" : "+r"(green), "+r"(center_x) : : "$2");
    temp=input>>1;
    /* ASM_BARRIER: preserve the intermediate before masking its low bit. */
    __asm__("" : "+r"(temp));
    interlace=temp&1;
    temp=interlace+1;
    height=temp<<8;
    center_y=height>>1;
    /* ASM_BARRIER: finish the Y center before reusing V0 for the opposite bank offset. */
    __asm__("" : "+r"(center_y) : : "$2");
    temp=1-interlace;
    other_y=temp<<8;
    SetGeomOffset(center_x,center_y);
    SetGeomScreen(290);
    width=320;
    wide=input&4;
    saved_flags=input;
    if(wide)width=480;
    env=FrameEnvs;
    SetDefDrawEnv(env,0,0,width,height);
    SetDefDrawEnv(&D_800D0268,0,other_y,width,height);
    SetDefDispEnv(&env->environment.display,0,other_y,width,height);
    SetDefDispEnv(&D_800D02C4,0,0,width,height);
    i=0;
    rgb24=saved_flags&1;
    one=1;
    settings=Options.halves;
    loop_wide=wide;
    window.y=0;window.x=0;window.h=0;window.w=0;
loop:
    {
        env->environment.r=red;env->environment.g=green;env->environment.b=b;
        env->environment.rgb24=rgb24;
        env->environment.dither=one;
        env->environment.window=window;
        env->environment.screen.x=settings[0x34/2];
        env->environment.screen.y=settings[0x38/2];
        env->environment.display.h=height;
        env->environment.interlace=interlace;
        env->environment.screen.h=256;
        if(loop_wide) {
            env->environment.high_res=one;
            env->environment.display.w=(env->environment.display.w*2)/3;
        }
        env++;
    }
    i++;
    if(i<2)goto loop;
    func_8001ECE0(0);
    func_8001ECE0(1);
}
