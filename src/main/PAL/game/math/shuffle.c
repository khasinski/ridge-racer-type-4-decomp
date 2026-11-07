#include "game/math/shuffle.h"
#include "rr/frame_loop.h"
extern struct { int p; int a[8]; } SlotOrder;
extern int SlotOrderCursor;
extern int PsyqRand();
extern int PsyqSrand();
void ShuffleSlotOrder()
{
    int i, a, b, x, y, t;
    PsyqSrand(FrameCounter);
    for (i = 7; i >= 0; i--)
        SlotOrder.a[i] = i;
    i = 0;
    do {
        a = PsyqRand();
        x = a % 8;
        b = PsyqRand();
        y = (x + b % 4) % 7;
        t = SlotOrder.a[x];
        i++;
        SlotOrder.a[x] = SlotOrder.a[y];
        SlotOrder.a[y] = t;
    } while (i < 0x20);
    SlotOrderCursor = 0;
}
