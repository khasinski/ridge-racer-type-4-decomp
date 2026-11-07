#include "types.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/race/ranking.h"
#include "game/menu/menutra22.h"
#include "game/gfx/drawbuf.h"

extern int DrawUiRow();
extern char *UiRows;
extern short MenuResultCode;
extern int AddTilePrim();
extern char StrName[];
extern char TeamLabel[];
extern char StrMaker[];

void func_8007D448(int a0, int a1);
void func_8007D558(void);
void DrawCardMenuChoiceRows(int a0, int a1);
void func_8007D710(void);

void func_8007D448(int a0, int a1) {
    int i = 0;
    unsigned char *p, *q;
    int v;
    do {
        if (a1 == 1) {
            p = (unsigned char *)(i * 0x24 + (int)UiRows + 0x558);
            if (p[0x14] == 5) { p[0x14] = 0; goto tail; }
        }
        if (a1 == 0) {
            q = (unsigned char *)(i * 0x24 + (int)UiRows + 0x558);
            v = q[0x14];
            if (v == 2) { q[0x14] = 3; goto tail; }
            if (v != 0) goto tail;
            return;
        }
tail:
        DrawUiRow(i + 0x26);
        if (i == a0) *(char *)(i * 0x24 + (int)UiRows + 0x564) = 2;
        else *(char *)(i * 0x24 + (int)UiRows + 0x564) = 0;
        i++;
    } while (i < 2);
}

void func_8007D558(void) {
}

void DrawCardMenuChoiceRows(int a0, int a1) {
 int n, i;
 if (MenuResultCode == 0) n = 4; else if (MenuResultCode == 6) n = 3; else n = 2;
 i = 0;
 if (n != 0) {
  do {
   DrawUiRow(i + 0x22);
   if (a1 == 1) {
    if (i == a0) *(UiRows + i*0x24 + 0x4D4) = 2;
    else *(UiRows + i*0x24 + 0x4D4) = 0;
   } else {
    signed char *p = (signed char *)(i*0x24 + (int)UiRows) + 0x4C8;
    if (p[0xC] == 2) p[0xC] = 1;
   }
   i++;
  } while (i < n);
 }
}

void DrawNameTeamMakerHeader(int a0)
{
  int new_var;
  int new_var2;
  DrawTextStub(0x28, 0xA0, StrName, 0x414);
  DrawTextStub(0x68, 0xA0, TeamLabel, 0x414);
  DrawTextStub(0xC2, 0xA0, StrMaker, 0x414);
  new_var2 = *((int *) SCRATCHPAD);
  *((int *) SCRATCHPAD) = (new_var = AddTilePrim(CurrentDrawBuf + 0x74, new_var2, 0x20, (a0 << 5) + 0xB2, 0x104, 0xC, 0x80, 0x20, 0x20));
}

void func_8007D710(void) {

}
