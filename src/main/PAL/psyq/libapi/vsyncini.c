/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/spu_tick_control.h"
#include "rr/timing.h"
#include "psyq/libapi/vsyncini.h"

extern s32 *RCNT1_MODE;

void trapIntrVSync();
void setIntrVSync(s32 slot,RrSpuTickCallback callback);
void VSync_memclr();

/* legacy typing of shared globals in this part of the unit */
#define InterruptCallback ((int (*)(int, int *))InterruptCallback)
#define VSyncCallbackTable (*(int (*))&VSyncCallbackTable)

int *startIntrVSync(void) {
    *RCNT1_MODE = 0x100;
    Vcount = 0;
    VSync_memclr(&VSyncCallbackTable, 8);
    InterruptCallback(0, &trapIntrVSync);
    return &setIntrVSync;
}

#undef InterruptCallback
#undef VSyncCallbackTable

/* legacy typing of shared globals in this part of the unit */
#define VSyncCallbackTable (*(void (*(*)[8])())&VSyncCallbackTable)

void trapIntrVSync() {
    int i = 0;
    void (**p)() = VSyncCallbackTable;
    Vcount++;
    do {
        if (*p) (*p)();
        i++;
        p++;
    } while (i < 8);
}

#undef VSyncCallbackTable

void setIntrVSync(s32 slot,RrSpuTickCallback callback)
{
    RrSpuTickCallback *base=VSyncCallbackTable;
    RrSpuTickCallback *p=base+slot;
    if(callback!=*p) *p=callback;
}

void VSync_memclr(p, n) int *p; int n; {
    while (n--) *p++ = 0;
}
