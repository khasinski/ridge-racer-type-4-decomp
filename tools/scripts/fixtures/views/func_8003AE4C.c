/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#define ModeMenuCursorSaved (*(unsigned char (*))&ModeMenuCursorSaved)

#undef LoadIndex

#undef ModeMenuCursorSaved

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define CarClutData (*(int (*))&CarClutData)

#undef CarClutData

/* legacy typing of shared globals in this part of the unit */
#define LoadIndex (*(unsigned char (*))&LoadIndex)

#undef LoadIndex

/* legacy typing of shared globals in this part of the unit */
#define LoadTimList ((void (*)())LoadTimList)

#define RaceLoadReset ((void (*)())RaceLoadReset)

#define SoundCloseVabsAndReset ((void (*)())SoundCloseVabsAndReset)

#undef LoadTimList

#undef RaceLoadReset

#undef SoundCloseVabsAndReset

extern char LoadIndex;

extern unsigned char LoadState;

extern short RaceType;

extern int RequestCarLoad();

extern int CarLoadStep();

extern int UploadCarClut();

extern struct { short a; short b; } PlayerCourseChoices[];

extern char *CarClutData;

extern char SaveDataBackup[];

extern unsigned char SelectionRows[][4];

int LoadCarRangeStep(int arg0, int arg1) {
    int r;
    int q4 = 0;
    int a1;
    char *q3;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
        break;
    case 1:
        arg0 += LoadIndex;
        if (RequestCarLoad(SelectionRows[arg0][1], SelectionRows[arg0][0], SelectionRows[arg0][2] % 20, PlayerCourseChoices[arg0].a, arg0) == 1) {
            LoadState = 2;
        }
        break;
    case 2:
        r = CarLoadStep();
        if (r != 1) break;
        arg0 += LoadIndex;
        a1 = PlayerCourseChoices[arg0].b;
        if (a1 >= 2) {
            q3 = CarClutData;
            a1 -= 2;
        } else {
            if (a1 == r) {
                q3 = SaveDataBackup + (arg0 & 1) * 0x840;
                a1 = 0;
            } else {
                a1 = -1;
            }
        }
        if (RaceType != 1) {
            UploadCarClut(q3, a1, arg0);
        }
        LoadIndex++;
        if (LoadIndex == arg1) {
            LoadState = 3;
        } else {
            LoadState = 1;
        }
        break;
    case 3:
        q4 = 1;
        break;
    default:
        break;
    }
    return q4;
}
