/* RODATA 0x800105b0 0x18 */
#include "types.h"
#include "m2c_macros.h"
#include "rr/menu_selection.h"
#include "game/gfx/fontdraw.h"
#include "game/menu/menu8.h"
#include "game/menu/menuslot.h"
#include "game/menu/menu13.h"

typedef struct {
    unsigned short *p;
    signed char b4, b5, st, b7;
    unsigned char b8;
    signed char b9;
    unsigned char bA, bB;
    short sC, sE, s10, s12, s14, s16;
    int i18;
    unsigned char b1C, b1D, b1E, b1F;
    int arr[0xC8];
} S;

extern int PlaySfx();
extern int MenuConfirmPressed;
extern char *BadgeRows;
extern char *Dialogue;
extern int AddSpriteWithDrawMode();

void UpdateDialogue(int a0, int a1, S *s);
void SkipDialoguePage(void *arg0);
int DecodeCourseIndex(s32 a0);
void func_80032EC8();

void UpdateDialogue(int a0, int a1, S *s) {
    unsigned short *p;
    int i;
    int v;
    char *q;
    int w;
    unsigned short *r;
    /* ASM_BARRIER: pin sum to v0 */
    register int e __asm__("$2");
    int c;

    if (s->b4 != s->b5) {
        s->st = 0;
    }
    s->b5 = s->b4;
    if (s->b4 < 0) {
        return;
    }
    switch (s->st) {
    case 0:
        p = DialogueLinePtrs[(signed char)(s->b4 % 40)];
        s->p = p;
        s->sC = p[0] & 0xFFF;
        s->p = p + 1;
        s->bA = *((unsigned char *)p + 2);
        s->p = p + 2;
        v = *((unsigned char *)p + 4);
        s->st = 5;
        s->b9 = 1;
        p += 3;
        s->p = p;
        s->bB = v;
        s->i18 = 0;
        v = D_800ACC20[s->b4];
        s->s12 = 0;
        s->sE = 1;
        s->s10 = 0;
        s->b1F = 0;
        s->s14 = 0;
        s->b8 = v;
        p = (unsigned short *)((char *)s + 0x31C);
        for (i = 0xC7; i >= 0; i--) {
            *(int *)((char *)p + 0x28) = 0;
            p = (unsigned short *)((char *)p - 4);
        }
        break;
    case 1:
        e = s->sC;
        c = s->i18;
        if (c >= e) {
            if (s->b7 == 1) {
                DrawDialogueContinueMark(a0 + 0x10D, a1 + 0x58);
                if (MenuConfirmPressed == 0) {
                    goto call1;
                }
                PlaySfx(3);
            }
            s->st = 3;
            s->s12 = s->bA - s->bB;
        } else {
            e = c + 1;
            s->i18 = e;
            if (s->sE <= s->s10) {
                if (s->s12 < s->bA - s->bB) {
                    s->s12 = s->s12 + 1;
                } else if (s->s12 == s->bA - s->bB) {
                    if (s->b7 < 2) {
                        s->i18 = s->sC;
                    }
                }
                s->s10 = 0;
            } else {
                s->s10 = s->s10 + 1;
            }
        }
call1:
        DrawAnimatedText(a0, a1, s, 0, 0x3FFC);
tail1:
        if ((signed char)s->b1C != (signed char)s->b1D) {
            s->b1F = 0;
            s->b1E = 0;
            s->b1D = s->b1C;
        }
        break;
    case 2:
        func_80031DA0();
        func_80031DA8();
        DrawAnimatedText(a0, a1, s, 1, 0x3FFC);
        break;
    case 3:
        if ((signed char)s->b8 > s->b9) {
            r = (unsigned short *)((char *)s->p + s->bA * 2);
            s->p = r;
            s->sC = r[0] & 0xFFF;
            s->p = r + 1;
            s->bA = *((unsigned char *)r + 2);
            s->p = r + 2;
            w = *((unsigned char *)r + 4);
            r += 3;
            s->p = r;
            s->s12 = 0;
            s->bB = w;
            for (i = 0xC7, q = (char *)s + 0x31C; i >= 0; i--) {
                *(int *)(q + 0x28) = 0;
                q -= 4;
            }
            s->i18 = 0;
            s->s10 = 0;
            s->b9 = s->b9 + 1;
            s->sE = 1;
            s->st = 1;
            if (s->b9 == (signed char)s->b8) {
                Dialogue[7] = 0;
            }
            break;
        }
    case 4:
        DrawAnimatedText(a0, a1, s, 1, 0x3FFC);
        s->st = 4;
        break;
    case 5:
        if (*(short *)(BadgeRows + s->s16 * 0x1C + 0x10) == 3) {
            s->st = 1;
        }
        break;
    }
}

void SkipDialoguePage(void *arg0) {
    if (M2C_FIELD(arg0, s32 *, 0x18) < M2C_FIELD(arg0, s16 *, 0xC)) {
        if (MenuConfirmPressed != 0) {
            if (M2C_FIELD(arg0, s8 *, 6) != 4) {
                PlaySfx(3);
                if (M2C_FIELD(arg0, s16 *, 0x12) < (M2C_FIELD(arg0, u8 *, 0xA) - M2C_FIELD(arg0, u8 *, 0xB))) {
                    M2C_FIELD(arg0, s16 *, 0x12) = (s16) (M2C_FIELD(arg0, u8 *, 0xA) - M2C_FIELD(arg0, u8 *, 0xB));
                    M2C_FIELD(arg0, s32 *, 0x18) = (s32) (M2C_FIELD(arg0, s16 *, 0xC) - 1);
                    return;
                }
                M2C_FIELD(arg0, s32 *, 0x18) = (s32) (M2C_FIELD(arg0, s16 *, 0xC) - 1);
            }
        }
    }
}

s32 DecodeCourseIndex(s32 a0) {
    ReverseCourse = a0 >= 8;
    return CourseIndexMap[a0 & 7];
}

void func_80032EC8(a0, a1, a2, a3, a4) int a0, a1, a2, a3, a4;
{
    int m = (a4 << 5) % 256;
    unsigned short s = (a4 << 6) + 0x3828;
    AddSpriteWithDrawMode(a0, a1, a2, a3, 0x20, 0x40, m, 0xC0, 0x9A, s, 0, 0x80);
}
