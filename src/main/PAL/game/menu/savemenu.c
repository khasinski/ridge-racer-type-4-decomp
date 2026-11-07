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
extern unsigned char D_800A41E8;
extern unsigned char D_800A41E9 __asm__("D_800A41E8+1");
extern short D_800A41EA __asm__("D_800A41E8+2");
extern char D_800A45C8[];
extern char D_800A4648[][0x80];
extern Struct4000 *SaveBuffer;
extern Struct2C SelectionState;
extern Struct2C SaveSlotSelections[3];
extern short SaveSlotSelect;
extern short SaveSlotIndex;
extern char CardTargetFileName[];
extern short MenuResultCode;
extern Struct2C SaveSlotSelectionsBackup[3];
extern int D_801272F0[];

int CardSaveStep(a0, a1)
short a0;
int a1;
{
    int i;
    int r;
    Struct4000 *p;

    if (a0 > 0) {
        D_800A41E8 = 0;
    }
    switch (D_800A41E8) {
    case 0:
        D_800A41EA = 0;
        PsyqBzero(SaveBuffer, 0x1AA8);
        i = D_800F3970 != 0;
        if (a0 == 1) {
            D_800A41E9 = 1;
            if (SaveSlotSelect < 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardHeaderTitles[i], 0x250, 0x1FC, 0x254, 0x1D8, D_800A45C8);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect & 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardTitleStrings[SaveSlotIndex], 0x250, 0x1FC, 0x254, 0x1D8, D_800A4648[SaveSlotIndex]);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect == 0x20) {
                *SaveBuffer = *CdDirectoryBuffer;
            }
        } else if (a0 >= 2) {
            D_800A41E9 = 0;
            if (SaveSlotSelect < 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardHeaderTitles[i], 0x250, 0x1FC, 0x254, 0x1D8, D_800A45C8);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect & 0x10) {
                BuildCardHeaderStatic(SaveBuffer, CardTitleStrings[SaveSlotIndex], 0x250, 0x1FC, 0x254, 0x1D8, D_800A4648[SaveSlotIndex]);
                BuildSaveData(SaveSlotSelect);
            } else if (SaveSlotSelect == 0x20) {
                *SaveBuffer = *CdDirectoryBuffer;
            }
        }
        D_800A41E8 = 1;
        break;
    case 1:
        D_800A41E8 = 2;
        break;
    case 2:
        for (i = 0x1D; i >= 0; i--) {
        }
        if (SaveSlotSelect == 0x20) {
            r = CardWriteFile((short)a1, 0, CardTargetFileName, SaveBuffer, 2, D_800A41E9);
        } else {
            r = CardWriteFile((short)a1, 0, CardTargetFileName, SaveBuffer, 1, D_800A41E9);
        }
        if (r == -1) {
            if (D_800A41EA < 3) {
                D_800A41EA = (unsigned short)D_800A41EA + 1;
                break;
            }
            D_800A41E8 = 5;
        } else {
            D_800A41E8 = 3;
        }
        break;
    case 3:
        i = GetHwCardEvent();
        if (i != 0) {
            if (i == 1) {
                D_800A41E8 = 4;
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
                    D_801272F0[SaveSlotIndex] = *(int *)((char *)p + 0x280);
                    break;
                }
            } else {
                if (D_800A41EA >= 3) {
                    D_800A41E8 = 5;
                } else {
                    D_800A41E8 = 1;
                    D_800A41EA = (unsigned short)D_800A41EA + 1;
                }
            }
        }
        break;
    case 4:
    case 5:
        break;
    }
    return D_800A41E8;
}
