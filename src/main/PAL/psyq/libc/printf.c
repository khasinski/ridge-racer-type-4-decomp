/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses -ffixed-fp */
/* RODATA 0x80012020 0x000000B4 */
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;

extern s32 PsyqStrlen(const s8 *);
extern s8 *PsyqMemchr(const s8 *, s32, s32);
extern void *PsyqMemmove(void *, const void *, s32);

typedef struct {
    u32 left : 1;
    u32 plus : 1;
    u32 alt : 1;
    u32 zero : 1;
    u32 has_prec : 1;
    u32 is_short : 1;
    u32 is_long : 1;
    u32 is_long_double : 1;
    s8 sign;
    s32 width;
    s32 prec;
} FmtState;

extern FmtState D_800ABC20;
extern const s8 HexDigitsUpper[];
extern const s8 StrHexDigitsLower[];

s32 PsyqSprintf(s8 *dst, const s8 *fmt, ...) {
    s8 buf[512];
    FmtState st;
    s32 * volatile ap;
    s32 total;
    s32 c;
    s8 *p;
    s32 len;
    const s8 *digits;
    u32 uval;
    s32 sval;
    register s32 ch_minus asm("s7"); /* ASM_BARRIER: pin '-' to s7 */
    register s32 ch_plus asm("s6"); /* ASM_BARRIER: pin '+' to s6 */
    register s32 ch_space asm("s5"); /* ASM_BARRIER: pin ' ' to s5 */
    register s32 ch_zero asm("s3"); /* ASM_BARRIER: pin '0' to s3 */

    ap = (s32 *)(&fmt + 1);
    total = 0;
    c = *fmt;
    if (c != 0) {
        ch_minus = '-';
        ch_plus = '+';
        ch_space = ' ';
        ch_zero = '0';
        do {
            if (c != '%') {
                goto literal;
            }
            {
                register FmtState *pst asm("a1"); /* ASM_BARRIER: pin &D_800ABC20 to a1 */
                register s32 w0 asm("v0"); /* ASM_BARRIER: pin struct copy w0 to v0 */
                register s32 w1 asm("v1"); /* ASM_BARRIER: pin struct copy w1 to v1 */
                register s32 w2 asm("a0"); /* ASM_BARRIER: pin struct copy w2 to a0 */
                pst = &D_800ABC20;
                __asm__ volatile("" : "+r"(pst)); /* ASM_BARRIER: materialize &D_800ABC20 in a1 */
                w0 = ((s32 *)pst)[0];
                w1 = ((s32 *)pst)[1];
                w2 = ((s32 *)pst)[2];
                __asm__ volatile("" : "+r"(w0), "+r"(w1), "+r"(w2)); /* ASM_BARRIER: force lw order */
                ((s32 *)&st)[0] = w0;
                ((s32 *)&st)[1] = w1;
                ((s32 *)&st)[2] = w2;
                __asm__ volatile("" : : "m"(st)); /* ASM_BARRIER: complete struct stores before '#' init */
            }
            {
                register s32 ch_hash asm("v1"); /* ASM_BARRIER: pin '#' to v1 */
                ch_hash = '#';
                __asm__ volatile("" : "+r"(ch_hash)); /* ASM_BARRIER: hoist '#' before flag loop */
                for (;;) {
                    register const s8 *f asm("a2"); /* ASM_BARRIER: pin fmt reload to a2 */
                    register const s8 *f1 asm("v0"); /* ASM_BARRIER: pin fmt+1 to v0 */
                    f = fmt;
                    f1 = f + 1;
                    __asm__ volatile("" : "+r"(f1)); /* ASM_BARRIER: keep f+1 in v0 */
                    fmt = f1;
                    c = f[1];
                    if (c == ch_minus) {
                        st.left = 1;
                    } else if (c == ch_plus) {
                        st.plus = 1;
                    } else if (c == ch_space) {
                        st.sign = c;
                    } else if (c == ch_hash) {
                        st.alt = 1;
                    } else if (c == ch_zero) {
                        st.zero = 1;
                    } else {
                        if (c == '*') {
                            sval = *ap++;
                            st.width = sval;
                            if (sval < 0) {
                                st.width = -sval;
                                st.left = 1;
                            }
                            f1 = f + 2;
                            fmt = f1;
                            c = f[2];
                        } else {
                            register u32 d asm("v0"); /* ASM_BARRIER: pin c - '0' to v0 */
                            while ((d = c - '0') < 10U) {
                                s32 w = st.width * 10 - '0';
                                __asm__ volatile("" : "+r"(w)); /* ASM_BARRIER: compute width*10 - '0' before + c */
                                st.width = w + c;
                                c = *++fmt;
                            }
                        }
                        break;
                    }
                }
            }
            if (c == '.') {
                register const s8 *f asm("a0"); /* ASM_BARRIER: pin fmt reload to a0 */
                register const s8 *f1 asm("v0"); /* ASM_BARRIER: pin fmt+1 to v0 */
                f = fmt;
                f1 = f + 1;
                __asm__ volatile("" : "+r"(f1)); /* ASM_BARRIER: keep f+1 in v0 */
                fmt = f1;
                c = f[1];
                if (c == '*') {
                    register s32 *a asm("v0"); /* ASM_BARRIER: pin ap reload to v0 */
                    register s32 pr asm("v1"); /* ASM_BARRIER: pin *ap to v1 */
                    a = ap;
                    pr = *a;
                    a++;
                    ap = a;
                    f1 = f + 2;
                    __asm__ volatile("" : "+r"(f1), "+r"(pr)); /* ASM_BARRIER: schedule f+2 before sw prec */
                    st.prec = pr;
                    fmt = f1;
                    c = f[2];
                } else {
                    register u32 d asm("v0"); /* ASM_BARRIER: pin c - '0' to v0 */
                    while ((d = c - '0') < 10U) {
                        s32 pr = st.prec * 10 - '0';
                        __asm__ volatile("" : "+r"(pr)); /* ASM_BARRIER: compute prec*10 - '0' before + c */
                        st.prec = pr + c;
                        c = *++fmt;
                    }
                }
                if (st.prec >= 0) {
                    st.has_prec = 1;
                }
            }
            if (st.left) {
                p = (s8 *)&st;
                st.zero = 0;
            } else {
                p = (s8 *)&st;
            }
            again:
                switch (c) {
                case 'h':
                    st.is_short = 1;
                    c = *++fmt;
                    goto again;
                case 'l':
                    st.is_long = 1;
                    c = *++fmt;
                    goto again;
                case 'L':
                    st.is_long_double = 1;
                    c = *++fmt;
                    goto again;
                case 'd':
                case 'i':
                    sval = *ap++;
                    __asm__ volatile("" : : "m"(st)); /* ASM_BARRIER: load st after ap++ */
                    if (st.is_short) {
                        sval = (s16)sval;
                    }
                    if (sval < 0) {
                        uval = -sval;
                        st.sign = ch_minus;
                    } else {
                        uval = sval;
                        if (st.plus) {
                            st.sign = ch_plus;
                        }
                    }
                    goto do_dec;
                case 'u':
                    uval = *ap++;
                    __asm__ volatile("" : : "m"(st)); /* ASM_BARRIER: load st after ap++ */
                    if (st.is_short) {
                        uval &= 0xFFFF;
                    }
                    st.sign = 0;
                do_dec:
                    if (!st.has_prec) {
                        if (st.zero) {
                            st.prec = st.width;
                            if (st.sign != 0) {
                                st.prec = st.width - 1;
                            }
                        }
                        if (st.prec <= 0) {
                            st.prec = 1;
                        }
                    }
                    len = 0;
                    while (uval != 0) {
                        *--p = (uval % 10U) + '0';
                        uval /= 10U;
                        len++;
                    }
                    while (len < st.prec) {
                        *--p = ch_zero;
                        len++;
                    }
                    if (st.sign != 0) {
                        *--p = st.sign;
                        len++;
                    }
                    break;
                case 'o':
                    uval = *ap++;
                    __asm__ volatile("" : : "m"(st)); /* ASM_BARRIER: load st after ap++ */
                    if (st.is_short) {
                        uval &= 0xFFFF;
                    }
                    if (!st.has_prec) {
                        if (st.zero) {
                            st.prec = st.width;
                        }
                        if (st.prec <= 0) {
                            st.prec = 1;
                        }
                    }
                    len = 0;
                    while (uval != 0) {
                        *--p = (uval & 7) + '0';
                        uval >>= 3;
                        len++;
                    }
                    if (st.alt && len != 0 && *p != ch_zero) {
                        *--p = ch_zero;
                        len++;
                    }
                    while (len < st.prec) {
                        *--p = ch_zero;
                        len++;
                    }
                    break;
                case 'p':
                    st.prec = 8;
                    st.has_prec = 1;
                    st.is_long = 1;
                    /* fallthrough */
                case 'X':
                    digits = HexDigitsUpper;
                    goto do_hex;
                case 'x':
                    digits = StrHexDigitsLower;
                do_hex:
                    uval = *ap++;
                    __asm__ volatile("" : : "m"(st)); /* ASM_BARRIER: load st after ap++ */
                    if (st.is_short) {
                        uval &= 0xFFFF;
                    }
                    if (!st.has_prec) {
                        if (st.zero) {
                            register s32 w asm("a2"); /* ASM_BARRIER: pin st.width to a2 */
                            w = st.width;
                            st.prec = w;
                            if (st.alt) {
                                register s32 w2 asm("v0"); /* ASM_BARRIER: pin w - 2 to v0 */
                                w2 = w - 2;
                                st.prec = w2;
                            }
                        }
                        if (st.prec <= 0) {
                            st.prec = 1;
                        }
                    }
                    len = 0;
                    while (uval != 0) {
                        *--p = digits[uval & 0xF];
                        uval >>= 4;
                        len++;
                    }
                    while (len < st.prec) {
                        *--p = ch_zero;
                        len++;
                    }
                    if (st.alt) {
                        *--p = c;
                        *--p = ch_zero;
                        len += 2;
                    }
                    break;
                case 'c':
                    {
                        register s32 *a asm("v0"); /* ASM_BARRIER: pin ap reload to v0 */
                        register u8 ch asm("v1"); /* ASM_BARRIER: pin char to v1 */
                        p--;
                        a = *(s32 **)&ap;
                        __asm__ volatile("" : "+r"(a)); /* ASM_BARRIER: emit lw v0, 544(sp) before len=1 */
                        len = 1;
                        __asm__ volatile("" : "+r"(len)); /* ASM_BARRIER: schedule len=1 in load delay slot */
                        ch = *(u8 *)a;
                        a++;
                        *p = ch;
                        *(s32 **)&ap = a;
                    }
                    break;
                case 's':
                    p = (s8 *)*ap++;
                    if (st.alt) {
                        len = (u8)*p++;
                        if (st.has_prec && st.prec < len) {
                            len = st.prec;
                        }
                    } else if (!st.has_prec) {
                        len = PsyqStrlen(p);
                    } else {
                        s8 *end = PsyqMemchr(p, 0, st.prec);
                        len = end - p;
                        if (end == 0) {
                            len = st.prec;
                        }
                    }
                    break;
                case 'n':
                    {
                        register s32 *a asm("v0"); /* ASM_BARRIER: pin ap reload to v0 */
                        register u32 flags asm("v1"); /* ASM_BARRIER: pin st flags to v1 */
                        register u32 is_sh asm("v0"); /* ASM_BARRIER: pin is_short shift to v0 */
                        a = *(s32 **)&ap;
                        flags = *(u32 *)&st;
                        __asm__ volatile("" : "+r"(a), "+r"(flags)); /* ASM_BARRIER: schedule flags load in ap load delay slot */
                        p = (s8 *)*a;
                        a++;
                        *(s32 **)&ap = a;
                        __asm__ volatile("" : : "m"(ap)); /* ASM_BARRIER: store ap before testing is_short */
                        is_sh = (flags >> 5) & 1;
                        if (is_sh) {
                            *(s16 *)p = total;
                        } else {
                            *(s32 *)p = total;
                        }
                        goto next_char;
                    }
                default:
                    if (c != '%') {
                        goto done;
                    }
                literal:
                    dst[total++] = c;
                    goto next_char;
                }
                if (len < st.width && !st.left) {
                    do {
                        dst[total++] = ch_space;
                        st.width--;
                    } while (len < st.width);
                }
                PsyqMemmove(dst + total, p, len);
                total += len;
                while (len < st.width) {
                    dst[total++] = ch_space;
                    len++;
                }
            next_char:
            c = *++fmt;
        } while (c != 0);
    }
done:
    dst[total] = 0;
    return total;
}
