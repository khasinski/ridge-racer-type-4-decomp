/* GCC_VERSION: 2.7.2 */

#include "rr/commands.h"

extern s32 _cardCmd59Tx(void), _cardCmd59Rx(s32);

int _cardCmd59Tx() {
    int a; 
    register int r __asm__("$2");
    int t; 
    register int c; 
    a = CardTxCount;
    t = 2;
    if (a == t) {
        r = 0x59;
        goto out;
    }
    r = 0;
    if (a < 2) goto out;
    if (a >= 6) goto out;
    c = a < 4;
    if (c) goto out;
    /* ASM_BARRIER: stops 2.7.2 if-converting the last compare into xori/negu/andi */
    __asm__("" : "=r"(r) : "0"(0xFF));
out:
    return r;
}
