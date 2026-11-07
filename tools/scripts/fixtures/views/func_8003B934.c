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

typedef struct { char p[7]; char b; } Z;

extern char LoadIndex;

extern unsigned char LoadState;

extern int GetR4FileIndex();

extern int CdFileReadStart();

extern int CdFileBytesLoaded();

extern char SharedWorkBuffer[];

extern int CdFileReadRange();

extern int InitDialogueState();

extern int SetupDialogueTable();

extern int LoadTimImage();

extern int DialogueContext;

extern int GameLanguage;

extern char DialogueLangEntry[];

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
        if (CdFileReadStart(GetR4FileIndex(1, 0x42), SharedWorkBuffer)) {
            LoadState = 2;
        }
        break;
    case 2:
        if (CdFileBytesLoaded()) {
            s = SharedWorkBuffer;
            LoadTimImage(s);
            s += 0x4E20;
            LoadState = 3;
            DialogueState = (Z *)s;
            InitDialogueState(s);
    case 3:
            if (DialogueContext == 2) {
                a1 = 0x44;
            } else if (DialogueContext == 4) {
                a1 = 0x46;
                DialogueState->b = 2;
            } else {
                a1 = 0x45;
            }
            if (CdFileReadRange(GetR4FileIndex(1, a1), GameLanguage << 2, 4, DialogueLangEntry)) {
                LoadState = 4;
            }
        }
        break;
    case 4:
        if (CdFileBytesLoaded()) {
            SetupDialogueTable(DialogueLangEntry);
            LoadState = 5;
    case 5:
            r = 1;
        }
        break;
    }
    return r;
}
