#include "types.h"
#include "scratchpad.h"
#include "rr/gpu.h"
#include "m2c_macros.h"
#include "game/menu/mnsel.h"
#include "game/ui/barsel1.h"
#include "game/gfx/fontdraw.h"
#include "game/menu/mntrow.h"
#include "rr/frame_environment.h"
#include "rr/menu_background.h"
#include "game/data/selection.h"
#include "game/gfx/drawbuf.h"

#define ROW(i) ((RrRow60 *)((s32)MenuTextRows + (i) * 0x3C))

typedef struct {
    u8 unk0;
    u8 unk1;
    s8 unk2;
    s8 unk3;
    u8 unk4[24];
    u8 unk1C[24];
    u16 unk34;
    u16 unk36;
    s16 unk38;
    s16 unk3A;
} RrRow60;

typedef struct 
{
  char pad[0x1C];
  char c;
  char pad2[0x17];
  short d;
  unsigned short e;
} E;

typedef struct 
{
  int pad[6];
  int f;
} E_1;

typedef struct { short f0, f2; } U;

typedef struct { char pad[4]; char f4; char pad2[2]; char f7; } T;

extern RrSpritePacket *AddDrawModePrim();
extern s32 rsin();
extern s32 rcos();
extern RrRow60 *MenuTextRows;
extern unsigned short MenuTextCursorGlyphs[];
extern E_1 *GlyphRows;
extern s32 VSyncThreshold;
extern s16 ModeId;
extern s16 ModeGroup;
extern s16 ModeSubState;
extern void InitBadgeRow();
extern void InitRect16Row();
extern void InitGlyphRow();
extern void InitUiRow(int,int,int,int,int,int,int,int,int,int,...);
extern void SeqStartTrack();
extern short MenuFadeTimer;
extern short SceneId;
extern short SelectedTeamIndex;
extern int ModeFrame;
extern short CourseListIndex;
extern U *BadgeRows;
extern T *Dialogue;
extern int DrawUiRow();
extern int MenuStepDirection;
extern int MenuConfirmPressed;
extern char *UiRows;

void EmitMenuRowTransitionGrid();
void SetMenuTextRowCursor();
void JumpToMenuMode8Enter(void);
void TeamSelectSceneEnter(void);
void FourChoiceSelectModeUpdate();

void DrawMenuTextRow(s32 a0) {
    RrSpritePacket *s4;
    s32 fp;
    s32 sp10;
    s32 sp14;
    s32 sp18;
    u8 *sp1C;
    u16 s5;
    u16 sp20;
    s32 s2;
    s32 s0;
    s32 a3;
    s32 s1;
    s16 d34;
    s32 v1;

    s4 = *(RrSpritePacket **)SCRATCHPAD;
    fp = ROW(a0)->unk38;
    sp1C = ROW(a0)->unk1C;
    sp10 = ROW(a0)->unk3A;
    s5 = ROW(a0)->unk36;
    /* ASM_BARRIER: keep fp and s7 increments before sp1C reload in outer loop latch */
    for (s2 = 0; *sp1C != 0; fp += 0xD, s2++, ({ __asm__("" : "=m"(*s4) : "r"(fp), "r"(s2 * 0x61)); })) {
        s0 = *sp1C - 0x20;
        sp1C++;
        if (s0 != 0) {
            s0 = FontGlyphCells[s0 * 2] + (FontGlyphCells[s0 * 2 + 1] << 5);
            a3 = 0;
            sp14 = (s0 % 21) * 12;
            sp18 = (s0 / 21) * 24;
            d34 = ROW(a0)->unk34;
            s1 = ROW(a0)->unk4[s2];
            {
                s32 ext = d34;
                if (ROW(a0)->unk3 >= s2) {
                    /* ASM_BARRIER: keep sign-extension of d34 before unk3 >= s2 branch */
                    __asm__("" : : "r"(ext));
                    if (d34 == 1) {
                        if (s2 & 1) {
                            s5 = 0x2638;
                            sp20 = 0x2639;
                        } else {
                            sp20 = 0x2638;
                            s5 = 0x2639;
                        }
                        if (rsin(s1 << 6) > 0) {
                            a3 = s1 * 4;
                        } else {
                            a3 = 0xFF - (s1 - 0x20) * 4;
                        }
                        if (s1 >= 0x40) {
                            ROW(a0)->unk4[s2] = 0;
                        }
                        ROW(a0)->unk4[s2]++;
                    } else if (d34 == 3) {
                        sp20 = 0x2639;
                        s5 = 0x2638;
                        a3 = (rcos(((s1 << 5) - s2 * 0x61) & 0xFFF) * 0x4A) / 4096 + 0x4A;
                        if (s1 >= 0x80) {
                            ROW(a0)->unk4[s2] = 0;
                        }
                        ROW(a0)->unk4[s2]++;
                    } else if (d34 == 2) {
                        s5 = 0x2638;
                        a3 = (rcos(((s1 << 5) - s2 * 0x61) & 0xFFF) * 0x4A) / 4096 + 0x4A;
                        if (s1 >= 0x80) {
                            ROW(a0)->unk4[s2] = 0;
                        }
                        ROW(a0)->unk4[s2]++;
                    } else if (d34 >= 0) {
                        s0 = d34 >> 8;
                        v1 = rsin(ROW(a0)->unk0 << 6) / 256;
                        a3 = v1 + 0x80;
                        if (s0 - 1 == s2) {
                            a3 = v1 + 0xE4;
                            s5 = 0x2638;
                        } else {
                            s5 = ROW(a0)->unk36;
                            if (s1 < 0x15) {
                                a3 = (rsin((s1 * 0x555) / 20) * 0x94) / 4096;
                                ROW(a0)->unk4[s2]++;
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
            for (s0 = 0; s0 < (ROW(a0)->unk34 & 1) + 1; s0++) {
                s4->len = 4;
                s4->code = 0x64;
                s4->x = fp;
                s4->y = sp10;
                s4->w = 0xC;
                s4->h = 0x18;
                s4->u = sp14;
                s4->v = sp18;
                if (s0 == 1) {
                    s4->clut = sp20;
                    a3 = 0x94 - a3;
                } else {
                    s4->clut = s5;
                }
                s4->r = a3;
                s4->g = a3;
                s4->b = a3;
                s4->code |= 2;
                s4->addr = DRAWBUF->ot[0].addr;
                DRAWBUF->ot[0].addr = (u32)s4;
                s4++;
                /* ASM_BARRIER: count one more reg in inner loop so loop_optimize does not hoist 0x64 */
                __asm__("" : "=r"(v1));
            }
        }
    }
    *(RrSpritePacket **)SCRATCHPAD = AddDrawModePrim((char *)CurrentDrawBuf + 0x70, s4, 0x2E);
}

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim (*(unsigned *(*)())&AddDrawModePrim)
#define CurrentDrawBuf ((char *)CurrentDrawBuf)

void EmitMenuRowTransitionGrid()
{
    unsigned *a1;
    int a3;
    unsigned char *q;
    char *d;
    unsigned t;
    a1 = AddDrawModePrim(CurrentDrawBuf + 0x98, *(int *)SCRATCHPAD, 5);
    q = (unsigned char *)a1 + 7;
    a3 = 0;
    do {
        q[a3 * 20 - 4] = 4;
        q[a3 * 20] = 0x64;
        *(short *)(q + a3 * 20 + 9) = 0xC;
        *(short *)(q + a3 * 20 + 0xB) = 0x18;
        q[a3 * 20 + 5] = 0xF4;
        q[a3 * 20 + 6] = 0x68;
        *(short *)(q + a3 * 20 + 7) = 0x2678;
        q[a3 * 20] |= 1;
        *(short *)(q + a3 * 20 + 1) = (a3 % 22) * 13 + 0x11;
        *(short *)(q + a3 * 20 + 3) = (a3 / 22) * 0x1C + 0x60;
        d = CurrentDrawBuf;
        a3++;
        t = (unsigned)a1 & 0xFFFFFF;
        *a1 = (*a1 & 0xFF000000) | (*(unsigned *)(d + 0x98) & 0xFFFFFF);
        *(unsigned *)(d + 0x98) = (*(unsigned *)(d + 0x98) & 0xFF000000) | t;
        a1 += 5;
    } while (a3 < 0xF2);
    *(unsigned **)SCRATCHPAD = AddDrawModePrim(d + 0x98, a1, 0xE, a3);
}

#undef AddDrawModePrim
#undef CurrentDrawBuf

/* legacy typing of shared globals in this part of the unit */
#define MenuTextRows ((E *)MenuTextRows)

void SetMenuTextRowCursor(a0, a1, a2)
int a0;
int a1;
int a2;
{
  E *p;
  ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->e = MenuTextCursorGlyphs[a1];
  if (a2 < (-0xFF))
  {
    ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->d = -a2;
    return;
  }
  switch (a2)
  {
    case 0:
      ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->c = 0x20;
      return;

    case 1:
      ((E *) (((int) MenuTextRows) + (a0 * 0x3C)))->c = 0x3E;
      return;

    case -1:
      return;

  }

}

#undef MenuTextRows

void SetGlyphRowImageBase(a0, a1)
int a0;
int a1;
{
  int i;
  int new_var;
  for (i = 0; i < 4; i++)
  {
    new_var = i;
    GlyphRows[a1 + new_var].f = a0;
    new_var = 0xC000;
    a0 = a0 + new_var;
  }

}

void JumpToMenuMode8Enter(void) {
    SetupDrawDispEnvs(0, 0, 0, 2);
    VSyncThreshold = 0x80;
    ModeGroup = 1;
    ModeId = 8;
    ModeSubState = 0;
}

/* legacy typing of shared globals in this part of the unit */
#define SetupDrawDispEnvs ((void (*)())SetupDrawDispEnvs)
#define GlyphRows ((char *)GlyphRows)

void TeamSelectSceneEnter(void) {
    int i;
    SetupDrawDispEnvs(0, 0, 0, 2);
    VSyncThreshold = 0x80;
    for (i = 0; i < 4; i++) {
        InitGlyphRow(i, -2, 0, 0);
    }
    SelectedTeamIndex = 2;
    SelectionRows[0] = 2;
    CourseListIndex = 0;
    SeqStartTrack(0);
    InitRect16Row(*(unsigned char *)(GlyphRows + 7), *(short *)(GlyphRows + 8), *(short *)(GlyphRows + 0xA), *(short *)(GlyphRows + 0xC), *(short *)(GlyphRows + 0xE), 0, 0);
    InitBadgeRow(0, 0xC, 0x130, 0x128, 0x58, 0, 0);
    InitUiRow(0, 0xC, 0x48, 0xF0, 0x20, 0, 0, 4, 2, 0);
    BadgeRows->f2 = 1;
    Dialogue->f4 = 1;
    Dialogue->f7 = 0;
    ModeFrame = 0;
    SceneId = 0x108;
    MenuFadeTimer = 0;
}

#undef SetupDrawDispEnvs
#undef GlyphRows

/* legacy typing of shared globals in this part of the unit */
#define SelectedTeamIndex (*(unsigned char (*))&SelectedTeamIndex)
#define ModeFrame (*(unsigned (*))&ModeFrame)
#define BadgeRows ((char *)BadgeRows)
#define Dialogue ((signed char *)Dialogue)

void FourChoiceSelectModeUpdate()
{
    int a2;
    ModeFrame += 1;
    if (Dialogue[4] >= 0x22) {
        DrawUiRow(0);
        UpdateScreenFade(0x10002);
    } else {
        UpdateScreenFade(0x10003);
    }
    if (TeamSelectMenuStep(MenuStepDirection) != 0) {
        if (*(short *)(BadgeRows + 0x10) == 3) {
            UpdateDialogueChoiceCursor();
            return;
        }
    }
    if (Dialogue[4] == 1) {
        if (MenuConfirmPressed == 0) {
            if (ModeFrame < 0x1F5) return;
        }
        a2 = Dialogue[6];
        if (a2 == 4) {
            Dialogue[4] = SelectedTeamIndex + 0x22;
            UiRows[0x16] = SelectedTeamIndex;
            *(short *)(BadgeRows + 0x10) = a2;
        }
    }
}

#undef SelectedTeamIndex
#undef ModeFrame
#undef BadgeRows
#undef Dialogue
