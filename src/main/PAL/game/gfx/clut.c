#include "psyq/libgpu/gpuenv.h"
#include "rr/garage_palette_refresh.h"
struct R { short x, y, w, h; };

extern int DrawSync();

int DebugPrintStub(void);
void UploadCarClut(int a0, int a1, int a2);

/* Resident callback that reports success without changing state. */
int DebugPrintStub(void)
{
    return 1;
}

void UploadCarClut(int a0, int a1, int a2)
{
    struct R r;
    unsigned short *s;
    unsigned t;
    if (a1 < 0) {
        r.x = a2 * 16 + 0x280;
        r.y = 0x40;
        r.w = 0x10;
        r.h = 1;
        ClearImage2(&r, 0, 0, 0);
        goto done;
    }
    a0 += a1 * 0x840;
    a0 += 4;
    t = *(unsigned *)a0;
    a0 += 4;
    if (t & 8) {
        s = (unsigned short *)a0;
        r.x = a2 * 16 + 0x280;
        r.y = 0x40;
        r.w = s[4];
        r.h = s[5];
        LoadImage(&r, (char *)s + 0xC);
        DrawSync(0);
        a0 = (int)s + ((*(unsigned *)s >> 2) << 2);
    }
    s = (unsigned short *)a0;
    r.x = a2 * 16 + 0x280;
    r.y = 0;
    r.w = s[4];
    r.h = s[5];
    if (r.w > 0) {
        if (r.h > 0) {
            LoadImage(&r, (char *)s + 0xC);
done:
            DrawSync(0);
        }
    }
}
