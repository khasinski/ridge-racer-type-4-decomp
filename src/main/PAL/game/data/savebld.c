/* RODATA 0x80011458 0x48 */
#include "game/data/savebld.h"
#include "game/data/savetype.h"
extern Struct48 CourseChoiceTable;
extern SaveBuf *SaveBuffer;
extern Struct2C SelectionState;
extern Struct40 Options;
extern Struct10 RaceRecords[2][8][5];
extern int CourseBestLapTimes[2][8][3];
extern Struct7C SaveProgress;
extern Struct840 *D_800F399C;
extern Struct840 *D_800F39A0;
extern int SaveDataBackup[];
extern int D_800F4D38;
extern unsigned short D_800F4D3C;
extern Struct10 LapRecords[2][8][5];
extern Struct2C SaveSlotSelectionsBackup[3];

void BuildSaveData(int arg0) {
    SaveBuf *s3;
    SaveBuf11 *p;
    int a3;
    int t0;
    int a1;
    int t1;

    switch (arg0) {
    case 0:
    case 2:
    case 4:
    default:
        for (a3 = 0xA0; a3 < 0x800; a3++) {
            ((int *)SaveBuffer)[a3] = 0;
        }
        s3 = SaveBuffer;
        s3->f280 = SaveProgress;
        s3->f2FC = Options;
        for (a3 = 0; a3 < 9; a3++) {
            s3->f33C[a3] = CourseChoiceTable.f24[a3];
        }
        for (a3 = 0; a3 < 3; a3++) {
            SaveSlotSelectionsBackup[a3].f10 = 0;
            t1 = 0;
            for (t0 = 0; (unsigned int)t0 < 0x16; t0++) {
                t1 += ((short *)&SaveSlotSelectionsBackup[a3])[t0];
            }
            SaveSlotSelectionsBackup[a3].f10 = t1;
            s3->f360[a3] = SaveSlotSelectionsBackup[a3];
        }
        if (arg0 == 3) {
            SelectionState.f10 = 0;
            t1 = 0;
            for (t0 = 0; (unsigned int)t0 < 0x16; t0++) {
                t1 += ((short *)&SelectionState)[t0];
            }
            SelectionState.f10 = t1;
            s3->f360[(short)((short)D_800F4D3C % 3)] = SelectionState;
        }
        for (a3 = 0; a3 < 2; a3++) {
            for (t0 = 0; t0 < 8; t0++) {
                for (a1 = 0; a1 < 3; a1++) {
                    s3->f3E4[a3][t0][a1] = CourseBestLapTimes[a3][t0][a1];
                }
                for (a1 = 0; a1 < 5; a1++) {
                    s3->f4A4[a3][t0][a1] = LapRecords[a3][t0][a1];
                    s3->f9A4[a3][t0][a1] = RaceRecords[a3][t0][a1];
                }
            }
        }
        for (a3 = 0; a3 < 0x210; a3++) {
            s3->fEA4[a3] = SaveDataBackup[a3];
        }
        t1 = 0;
        for (a3 = 0; (unsigned int)a3 < 0x6A9; a3++) {
            t1 += ((int *)s3)[a3];
        }
        s3->f1AA4 = t1;
        break;
    case 6:
        s3 = SaveBuffer;
        for (a3 = 0; a3 < 0x210; a3++) {
            s3->fEA4[a3] = SaveDataBackup[a3];
        }
        /* fallthrough */
    case 17:
        p = (SaveBuf11 *)SaveBuffer;
        p->f280 = D_800F4D38;
        for (t0 = 0; t0 < 2; t0++) {
            for (a3 = 0; a3 < 0x840; a3++) {
                if (t0 == 0) {
                    p->f300[0][a3] = D_800F39A0->b[a3];
                } else {
                    p->f300[t0][a3] = D_800F399C->b[a3];
                }
            }
        }
        break;
    }
}
