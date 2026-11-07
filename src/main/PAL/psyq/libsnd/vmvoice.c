/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channels.h"
#include "rr/spu.h"
#include "psyq/libspu/s_snc.h"
#include "psyq/libsnd/vmrow.h"
#include "game/data/globals.h"

extern u16 D_8010F90C[];
extern s16 _svm_sreg_buf[];
extern s16 D_8010FE3A[];
extern s8 D_80110105;
extern s8 D_8011010B;
extern u8 D_80110112;

void vmNoiseOn(s32 arg0) {
    /* ASM_BARRIER: keep arg0 in s3 */
    register s32 s3 __asm__("$19") = arg0;
    /* ASM_BARRIER: keep masked voice index in s0 */
    register u32 s0 __asm__("$16");
    u32 left;
    u32 right;
    s8 *p2;
    u16 low_mask, high_mask;
    s16 i;
    u16 f8, fa;

    {
        /* ASM_BARRIER: keep _svm_vh pointer in v0 */
        register u8 *f4 __asm__("$2");
        /* ASM_BARRIER: keep f18 * 0x3FFF in v0 */
        register s32 v0 __asm__("$2");
        /* ASM_BARRIER: keep p[-0x10] and first dividend in v1 */
        register s32 v1 __asm__("$3");
        /* ASM_BARRIER: keep f4[0x18] in a0 */
        register s32 f18 __asm__("$4");
        /* ASM_BARRIER: keep SvmLastChannel pointer in t0 */
        register s8 *p __asm__("$8");
        u32 a3;
        u32 vol;
        /* ASM_BARRIER: keep ch in a0 */
        register s32 ch __asm__("$4");
        u32 offset;
        s32 group_off;
        /* ASM_BARRIER: preserve the indexed table-address computation in v1. */
        register RrChannelValueRecordB0 **table __asm__("$3");
        /* ASM_BARRIER: reuse v1 for the loaded channel-record pointer. */
        register RrChannelValueRecordB0 *rec __asm__("$3");
        /* ASM_BARRIER: keep pan in a0 */
        register s32 pan __asm__("$4");

        /* ASM_BARRIER: save arg0 in s3 before loading globals */
        __asm__("" : "+r"(s3));
        f4 = _svm_vh;
        /* ASM_BARRIER: load _svm_vh into v0 before SvmLastChannel */
        __asm__("" : "+r"(f4), "+r"(s3));
        p = (s8 *)&SvmLastChannel;
        /* ASM_BARRIER: preserve SvmLastChannel and _svm_vh base pointers */
        __asm__("" : "+r"(f4), "+r"(p));
        f18 = f4[0x18];
        v1 = p[-0x10];
        /* ASM_BARRIER: load f4[0x18] into a0 and p[-0x10] into v1 */
        __asm__("" : "+r"(f18), "+r"(v1), "+r"(p));
        v0 = f18 * 0x3FFF;
        /* ASM_BARRIER: compute f18 * 0x3FFF in v0 before multiply */
        __asm__("" : "+r"(v0), "+r"(v1));
        v1 *= v0;
        /* ASM_BARRIER: keep product in v1 and delay magic load in v0 until after mflo */
        __asm__("" : "+r"(v1), "+r"(v0));
        a3 = v1 / 16129;
        v1 = a3 * p[-0xA];
        v0 = p[-0x7];
        /* ASM_BARRIER: keep a3 * p[-0xA] in v1 and p[-0x7] in v0 before multiply */
        __asm__("" : "+r"(v1), "+r"(v0));
        vol = (u32)(v1 * v0) / 16129U;
        ch = *(u16 *)p;
        offset = (ch & 0xFF) * sizeof(RrChannelValueRecordB0 *);
        /* ASM_BARRIER: compute offset before sign-extending ch */
        __asm__("" : "+r"(ch) : "r"(offset));
        ch = (s16)ch;
        /* ASM_BARRIER: finish vol division and offset before computing group_off */
        __asm__("" : "+r"(vol), "+r"(ch) : "r"(offset), "r"(a3));
        group_off = ((ch & 0xFF00) >> 8) * 11;
        /* ASM_BARRIER: form group_off before materializing table base. */
        __asm__ volatile("" : "+r"(group_off) : "r"(offset) : "$3");
        table = (RrChannelValueRecordB0 **)0x80110000U;
        /* ASM_BARRIER: preserve the explicit table base through the indexed load. */
        __asm__("" : "=r"(table) : "0"(table));
        table += (s32)(offset / sizeof(*table));
        rec = table[-0xF98 / 4];
        rec = (RrChannelValueRecordB0 *)((u8 *)rec + (group_off << 4));
        a3 = vol;
        if (ch != 0x21) {
            /* ASM_BARRIER: prevent CSE from replacing vol with a3 */
            __asm__("" : "+r"(vol));
            a3 = (vol * rec->value58) / 127U;
            vol = (vol * rec->value5A) / 127U;
        }
        pan = p[-0x6];
        if ((u32)pan < 0x40U) {
            left = a3;
            right = (vol * pan) / 63U;
        } else {
            right = vol;
            left = (a3 * (0x7F - pan)) / 63U;
        }
        /* ASM_BARRIER: keep pan and a3 live across both branches */
        __asm__("" : : "r"(pan), "r"(a3));
        pan = D_8011010B;
        if ((u32)pan < 0x40U) {
            right = (right * pan) / 63U;
        } else {
            left = (left * (0x7F - pan)) / 63U;
        }
        /* ASM_BARRIER: keep pan and a3 live across both branches */
        __asm__("" : : "r"(pan), "r"(a3));
        pan = D_80110105;
        if ((u32)pan < 0x40U) {
            right = (pan * right) / 63U;
        } else {
            left = (left * (0x7F - pan)) / 63U;
        }
        /* ASM_BARRIER: keep pan and a3 live across both branches */
        __asm__("" : : "r"(pan), "r"(a3));
    }
    if (_svm_stereo_mono == 1) {
        if (left < right) {
            left = right;
        } else {
            right = left;
        }
    }
    p2 = (s8 *)&SvmLastChannel;
    /* ASM_BARRIER: preserve SvmLastChannel base pointer in register */
    __asm__("" : "+r"(p2));
    if (*(s16 *)p2 != 0x21) {
        left = (left * left) / 16383U;
        right = (right * right) / 16383U;
    }
    SpuSetNoiseClock((p2[-0x12] - p2[-0x4]) & 0x3F);
    s0 = s3 & 0xFF;
    D_8010FE3A[s0 * 8] = right;
    {
        /* ASM_BARRIER: use v0 for PAL flag page load */
        register u8 *flags_page __asm__("$2");
        u8 flags;
        /* ASM_BARRIER: finish right store before loading flag byte */
        __asm__ volatile("" : : : "memory");
        flags_page = (u8 *)0x80110000U;
        /* ASM_BARRIER: retain the PAL flag page for the indexed load. */
        __asm__("" : "+r"(flags_page));
        flags = *(flags_page + s0 - 0x1E8);
        _svm_sreg_buf[s0 * 8] = left;
        ((u8 *)&_svm_sreg_dirty)[s0] = flags | 3;
    }
    if (s0 < 0x10U) {
        low_mask = 1 << s0;
        high_mask = 0;
    } else {
        low_mask = 0;
        high_mask = 1 << (s0 - 0x10);
    }
    /* ASM_BARRIER: keep s0 live so s0 - 0x10 uses v1 instead of modifying s0 in place */
    __asm__("" : : "r"(s0));
    D_8010F90C[(s3 & 0xFF) * 27] = 0xA;
    for (i = 0; i < spuVmMaxVoice; i++) {
        if (!(_snd_vmask & (1 << i))) {
            /* ASM_BARRIER: keep row_off in v0 */
            register u32 row_off __asm__("$2") = i * 0x36;
            /* ASM_BARRIER: use v1 for PAL row flag load */
            register u8 *row_page __asm__("$3");
            /* ASM_BARRIER: compute row_off before loading row_page */
            __asm__("" : "+r"(row_off));
            row_page = (u8 *)0x80110000U;
            /* ASM_BARRIER: retain the PAL row flag page for the indexed load. */
            __asm__("" : "+r"(row_page));
            row_page += row_off;
            D_8010F925[row_off] = row_page[-0x6DB] & 1;
        }
    }
    D_8010F925[(s3 & 0xFF) * 0x36] = 2;
    /* ASM_BARRIER: keep s3 live so s3 & 0xFF uses v1 instead of modifying s3 in place */
    __asm__("" : : "r"(s3));
    f8 = _svm_okon1 | low_mask;
    fa = _svm_okon2 | high_mask;
    _svm_okon1 = f8;
    _svm_okon2 = fa;
    _svm_okof1 &= ~f8;
    _svm_okof2 &= ~fa;
    if (D_80110112 & 4) {
        /* ASM_BARRIER: retain assembler temporary for delay-slot halfword store */
        register u16 *base __asm__("$1");
        u16 fe;
        _svm_orev1 |= low_mask;
        fe = _svm_orev2 | high_mask;
        /* ASM_BARRIER: finish _svm_orev1 store before materializing _svm_orev2 base */
        __asm__ volatile("" : : "r"(fe) : "$1", "memory");
        base = (u16 *)0x80110000U;
        /* ASM_BARRIER: expose the final halfword store in the branch delay slot */
        __asm__("" : "=r"(base) : "0"(base));
        base[-0x702 / 2] = fe;
    } else {
        _svm_orev1 &= ~low_mask;
        _svm_orev2 &= ~high_mask;
    }
    _svm_onos1 = low_mask;
    _svm_onos2 = high_mask;
    /* ASM_BARRIER: keep epilogue after final global stores */
    __asm__ volatile("" : : : "memory");
}
