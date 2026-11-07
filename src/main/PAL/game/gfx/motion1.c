/* RODATA 0x8001129c 0x30 */
#include "types.h"
#include "m2c_macros.h"
#include "game/gfx/motion1.h"
#include "rr/bios_devices.h"
#include "rr/textured_panel.h"

struct E {
    int state; int mode; char pad8[0x44 - 8]; int x44; unsigned short h48; short h4A; unsigned short h4C;
};

struct Z { char pad[0x4C]; unsigned short h4C; char pad2[0xE4 - 0x4E]; struct E e; };

struct A { short h0; char pad2[0xE]; unsigned int i10; };

extern struct Z *MotionController;
extern s32 SlotOrder;

void UpdateMotionControllerPanel(struct A *a0);

/* legacy typing of shared globals in this part of the unit */
#define MotionController (*(s32 (*))&MotionController)

void InitMotionControllerPanel(s32 arg0) {
    void *temp_s1;

    temp_s1 = MotionController + 0xE4;
    PsyqBzero(temp_s1, 0x50);
    M2C_FIELD(temp_s1, s32 *, 4) = arg0;
}

#undef MotionController

void UpdateMotionControllerPanel(struct A *a0) {
    int tbl[2][3] = {{1, 2, 4}, {5, 6, 4}};
    struct Z *p = MotionController;
    struct E *s = &p->e;
    int mode = s->mode;
    unsigned int t, x;
    int val, ns, m;
    if (mode < 0) return;
    switch (mode) {
    case 0:
    case 1:
    {
        switch (p->e.state) {
        case 0:
            if (a0->h0 == 0) return;
            x = a0->i10 / 50 * 2;
            x = x / 3;
            s->h4A = x - (MotionController->h4C + 1);
            val = tbl[s->mode][0];
            s->h48 = 0;
            s->state = s->state + 1;
            s->x44 = val;
            break;
        case 1:
            s->h48 = s->h48 + 1;
            if ((short)s->h48 < s->h4A) return;
            m = s->mode;
            val = tbl[m][1];
            s->h48 = 0;
            s->state = s->state + 1;
            s->x44 = val;
            break;
        case 2:
            s->h48 = s->h48 + 1;
            if ((short)s->h48 < s->h4A) return;
            m = s->mode;
            val = tbl[m][2];
            s->h48 = 0;
            s->state = 3;
            s->x44 = val;
            break;
        case 4:
            s->h48 = s->h48 + 1;
            if ((short)s->h48 < s->h4A) return;
            s->h4A = (a0->i10 / 100) - (MotionController->h4C + 1);
            s->x44 = 5;
            s->h48 = 0;
            s->state = s->state + 1;
            break;
        case 5:
            s->h48 = s->h48 + 1;
            if ((short)s->h48 < s->h4A) return;
            s->x44 = 6;
            s->h48 = 0;
            s->state = s->state + 1;
            break;
        }
    }
    break;
    case 2:
    {
        switch (p->e.state) {
        case 0:
            if (a0->h0 == 0) return;
            s->x44 = 3;
            s->h48 = 0;
            p->e.state = p->e.state + 1;
            break;
        case 1:
            if (a0->h0 == 0) return;
            s->x44 = 0;
            s->h48 = 0;
            p->e.state = 0;
            break;
        }
    }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

void DrawMotionControllerPanel(void) {
    s32 temp_a0;

    temp_a0 = M2C_FIELD(MotionController, s32 *, 0x128);
    if (temp_a0 > 0) {
        EmitTexturedPanel(temp_a0 - 1);
    }
}

#undef MotionController

void ResetSlotOrder(void) {
    SlotOrder = -1;
}
