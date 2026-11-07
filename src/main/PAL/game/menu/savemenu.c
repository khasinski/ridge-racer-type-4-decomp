/* RODATA 0x80011570 0x18 */
#include "game/data/cardini.h"
#include "game/data/savedat.h"
#include "game/data/savebld.h"
#include "game/system/modeinit.h"
#include "game/menu/savemenu.h"
#include "rr/bios_devices.h"
typedef struct {
    char b[0x4000];
} Struct4000;

typedef struct {
    char b[0x2C];
} Struct2C;


extern Struct4000 *CdDirectoryBuffer;
extern char CardHeaderTitles[][0x40];
extern char CardTitleStrings[][0x40];
extern unsigned char CardSaveStepState;
extern unsigned char CardSaveCreateFlag __asm__("CardSaveStepState+1");
extern short CardSaveRetryCount __asm__("CardSaveStepState+2");
extern char CardIconImage[];
extern char SaveIconBitmaps[][0x80];
extern Struct4000 *SaveBuffer;
extern Struct2C SelectionState;
extern Struct2C SaveSlotSelections[3];
extern short SaveSlotSelect;
extern short SaveSlotIndex;
extern char CardTargetFileName[];
extern short MenuResultCode;
extern Struct2C SaveSlotSelectionsBackup[3];
extern int garage_slot_file_times[];

int CardSaveStep(a0, a1)
short a0;
int a1;
{
    int i;
    int r;
    Struct4000 *p;

    if (a0 > 0) {
        CardSaveStepState = 0;
    }
    switch (CardSaveStepState) {
    case 0:
        CardSaveRetryCount = 0;
        PsyqBzero(SaveBuffer, 0x1AA8);
        i = AllProgressComplete != 0;
        if (a0 == 1) {
            CardSaveCreateFlag = 1;
            if (SaveSlotSelect < 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardHeaderTitles[i], 0x250, 0x1FC, 0x254, 0x1D8, CardIconImage);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect & 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardTitleStrings[SaveSlotIndex], 0x250, 0x1FC, 0x254, 0x1D8, SaveIconBitmaps[SaveSlotIndex]);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect == 0x20) {
                *SaveBuffer = *CdDirectoryBuffer;
            }
        } else if (a0 >= 2) {
            CardSaveCreateFlag = 0;
            if (SaveSlotSelect < 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardHeaderTitles[i], 0x250, 0x1FC, 0x254, 0x1D8, CardIconImage);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect & 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardTitleStrings[SaveSlotIndex], 0x250, 0x1FC, 0x254, 0x1D8, SaveIconBitmaps[SaveSlotIndex]);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect == 0x20) {
                *SaveBuffer = *CdDirectoryBuffer;
            }
        }
        CardSaveStepState = 1;
        break;
    case 1:
        CardSaveStepState = 2;
        break;
    case 2:
        for (i = 0x1D; i >= 0; i--) {
        }
        if (SaveSlotSelect == 0x20) {
            r = CardWriteFile((short)a1, 0, CardTargetFileName, SaveBuffer, 2, CardSaveCreateFlag);
        } else {
            r = CardWriteFile((short)a1, 0, CardTargetFileName, SaveBuffer, 1, CardSaveCreateFlag);
        }
        if (r == -1) {
            if (CardSaveRetryCount < 3) {
                CardSaveRetryCount = (unsigned short)CardSaveRetryCount + 1;
                break;
            }
            CardSaveStepState = 5;
        } else {
            CardSaveStepState = 3;
        }
        break;
    case 3:
        i = GetHwCardEvent();
        if (i != 0) {
            if (i == 1) {
                CardSaveStepState = 4;
                switch (MenuResultCode) {
                case 3:
                    SaveSlotSelectionsBackup[SaveSlotIndex] = SelectionState;
                    for (i = 0; i < 3; i++) {
                        SaveSlotSelections[i] = SaveSlotSelectionsBackup[i];
                    }
                    break;
                case 6:
                case 7:
                    p = SaveBuffer;
                    garage_slot_file_times[SaveSlotIndex] = *(int *)((char *)p + 0x280);
                    break;
                }
            } else {
                if (CardSaveRetryCount >= 3) {
                    CardSaveStepState = 5;
                } else {
                    CardSaveStepState = 1;
                    CardSaveRetryCount = (unsigned short)CardSaveRetryCount + 1;
                }
            }
        }
        break;
    case 4:
    case 5:
        break;
    }
    return CardSaveStepState;
}
