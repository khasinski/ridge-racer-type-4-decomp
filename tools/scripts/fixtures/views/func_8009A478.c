/* CC1_FLAGS: -mno-split-addresses */

/* MASPSX_FLAGS: --assembler-profile=gnu */

struct C { char p0[0x14]; int f14; int (*f18)(); char p1c[0x14]; unsigned char *f30; char p34[8]; unsigned char *f3c; char p40[4]; unsigned char f44, f45, f46, f47, f48, f49, f4a; char p4b[5]; unsigned char f50; char p51[0x97]; unsigned char fE8; };

extern int _padRecvAtLoadInfo();

extern int (*PadResetChannelFn)();

void _padPacketDone(a0) struct C *a0; {
    unsigned int t;
    int a1;
    unsigned char b;
    int i;
    t = a0->f3c[0];
    if ((t & 0xF0) == 0) {
        a0->f30[0] = 0xFF;
        a0->f30[1] = 0;
        a0->fE8 = 0;
        *((unsigned char *)a0 + 0x35) = 0;
        PadResetChannelFn();
        return;
    }
    a1 = a0->fE8;
    a0->fE8 = t >> 4;
    if ((t >> 4) == 0xF) {
        a0->fE8 = a1;
    } else {
        a0->f30[0] = 0;
        a0->f30[1] = a0->f3c[0];
        *((unsigned char *)a0 + 0x35) = *(volatile unsigned char *)&a0->f44;
        for (i = 2; i < a0->f44; i++) a0->f30[i] = a0->f3c[i];
    }
    if (a0->f3c[1] == 0) {
        if (a0->f46 == 1 && a0->f14 == 0) goto l10;
        if (a0->f50 != 0) goto l10;
        goto l11;
    }
l10:
    if (a0->fE8 != a1) {
l11:
        PadResetChannelFn(a0, a1);
    }
    b = a0->f46;
    a0->f4a = 0;
    if (b != 0xFF) {
        if (b != 0 && *((unsigned char *)a0 + 0x37) == 0) return;
        if ((unsigned char)(b - 2) < 0xFC) {
            if (a0->f3c[0] != 0xF3) { PadResetChannelFn(a0); return; }
        }
        {
            /* ASM_BARRIER: keep switch value in $v1 so constant 1 is allocated to $a0. */
            register unsigned char sw asm("$3");
            int one;
            switch (sw = a0->f46, one = 1, sw) {
            case 0:
                a0->f49 = one;
                a0->f46 += 1;
                break;
            case 1:
                a0->f47 = 0;
                a0->f46 += 1;
                break;
            case 0xFE:
                a0->f46 = 0xFF;
                break;
            default: {
                /* ASM_BARRIER: keep the callback return value in $v0 across the join label. */
                register int v asm("$2");
                if (a0->f18 != 0) v = a0->f18(a0); else v = _padRecvAtLoadInfo(a0);
                a0->f46 += v;
                break;
            }
            }
        }
    }
}
