#include "rr/frame_loop.h"

extern int DebugPrintStub();

extern int CdInit();

extern int InitCdFileDirectory();

extern int SsInit();

extern int SoundSystemInit();

extern int CdAudioInit();

extern int ResetGraph();

extern int SetGraphDebug();

extern int SetDispMask();

extern int InitGeom();

extern int CardSystemInit();

extern int InitGamePads();

extern int InitOptionsDefaults();

extern int ApplyOptionsPadButtonMaps();

extern int AddSIO();

extern int _sio_control();

extern int SetupRenderer();

extern int PsyqBzero();

extern int CopyStringClearTail();

extern int InitMenuArenas();

extern int InitDefaultTimeRecords();

extern int InitCourseChoices();

extern int InitTimTemplate();

extern int ResetRankEntries();

extern int InitCarTable();

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

extern int LoadDestCursor;

extern int LoadCursor;

extern int MirrorMode;

extern signed char SelectionState[];

extern short LinkRoleSlot;

extern char RankEntries[];

extern char SaveDataWork[];

extern char SaveProgress[];

extern char PlayerNameSlots[];

extern char SaveDataBackup[];

extern char SelectionRows[];

extern signed char SaveSlotSelections[];

extern int RankTable[];

extern int TeamStringPtrTable[];

extern int RivalTeamNamePtrTable[];

extern int NumLaps;

extern int D_800F4C24;

extern int D_800F4C2C;

extern int DialogueContext;

extern int MenuArenaBase;

extern short SaveNoticeFlags;

extern short RaceType;

extern char *DefaultCarClut;

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
