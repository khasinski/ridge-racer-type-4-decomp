#include "game/ui/uicnt.h"
#include "game/course/trkintp2.h"
#include "game/ui/uicode.h"
extern int ChecksumBytes();
void EncodePasswordCode(a0, a1) unsigned char *a0; int a1;
{
    int s5, s2, s1, s0, t;
    unsigned char *s3, *v0, *tab, *p;
    XorBytes37(a0, 6);
    a0[5] = a0[5] & 0xF;
    s5 = ChecksumBytes(a0, 6);
    s2 = 0;
    s3 = a0;
    tab = PasswordCipherKey;
    s1 = ((unsigned)(s5 & 0xFF) % 11) & 0xFF;
    do {
        v0 = (unsigned char *)(s1 + (int)tab);
        s1++;
        p = s3 + s2;
        *p = AddOrSubtractByte(*p, *v0, 0);
        if (s1 >= 0xB) s1 = 0;
        s2++;
    } while (s2 < 6);
    ByteToCodeChars(s5 & 0xFF, a1);
    s3 = a0;
    s2 = 0;
    s0 = 2;
    do {
        t = *s3;
        s3++;
        ByteToCodeChars(t, a1 + s0);
        s0 += 2;
        s2++;
    } while (s2 < 6);
}
