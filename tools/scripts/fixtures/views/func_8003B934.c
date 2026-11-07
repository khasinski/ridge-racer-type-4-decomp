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
#define D_800F3B24 (*(int (*))&D_800F3B24)

#undef D_800F3B24

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

typedef struct { char p[7]; char b; } Z;

extern char LoadIndex;

extern unsigned char LoadState;

extern int GetR4FileIndex();

extern int CdFileReadStart();

extern int CdFileBytesLoaded();

extern char D_800FFC48[];

extern int CdFileReadRange();

extern int InitDialogueState();

extern int SetupDialogueTable();

extern int LoadTimImage();

extern int D_800F4CA8;

extern int D_800AD564;

extern char D_80104DB0[];

extern Z *DialogueState;

int LoadDialogueTableStep(void) {
    int r = 0;
    char *s;
    int a1;
    switch (LoadState) {
    case 0:
        LoadState = 1;
        LoadIndex = 0;
    case 1:
        if (CdFileReadStart(GetR4FileIndex(1, 0x42), D_800FFC48)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            s = D_800FFC48;
            LoadTimImage(s);
            s += 0x4E20;
            LoadState = 3;
            DialogueState = (Z *)s;
            InitDialogueState(s);
    case 3:
            if (D_800F4CA8 == 2) {
                a1 = 0x44;
            } else if (D_800F4CA8 == 4) {
                a1 = 0x46;
                DialogueState->b = 2;
            } else {
                a1 = 0x45;
            }
            if (CdFileReadRange(GetR4FileIndex(1, a1), D_800AD564 << 2, 4, D_80104DB0)) {
                LoadState = 4;
            }
        }
        break;
    case 4:
        if (CdFileBytesLoaded()) {
            SetupDialogueTable(D_80104DB0);
            LoadState = 5;
    case 5:
            r = 1;
        }
        break;
    }
    return r;
}
