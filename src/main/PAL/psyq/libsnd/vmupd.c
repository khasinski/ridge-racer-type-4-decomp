/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channels.h"
#include "rr/spu.h"
#include "psyq/libsnd/vmnote2.h"
#include "game/data/globals.h"

typedef struct {
    s8 f00, f01, f02, f03, f04;
    u8 f05;
    s8 f06, f07, f08, f09, f0A;
    u8 f0B;
    s8 f0C, f0D;
    u8 f0E;
    s8 f0F, f10, f11;
    u8 f12;
    s8 f13;
    s16 f14, f16, f18;
} SvmCur;

extern u16 SvmVoicePitch[];
extern s16 _svm_sreg_buf[];
extern s16 SvmSregVolR[];
extern s16 SvmSregPitch[];
extern SvmCur _svm_cur;
extern char SvmVoiceNote[];
extern char SvmVoiceSeqSep[];
extern char SvmVoiceProg[];
extern char SvmVoiceTone[];
extern char SvmVoiceVabId[];
extern signed char SvmCurProgIndex;
extern char SvmCurTone[];

void _SsVmKeyOnNow(s16 arg0, s16 arg1);
int _SsVmPBVoice(short a0, short a1, short a2, short a3, int a4);

void _SsVmKeyOnNow(s16 arg0, s16 arg1) {
    u32 vol;
    u32 a3;
    u32 left, right;
    u16 voice_off;
    s32 v;
    RrChannelValueRecordB0 *rec;
    u8 pan;
    u16 low_mask, high_mask;

    v = _svm_cur.f18;
    voice_off = v << 3;
    a3 = ((_svm_cur.f04 * 0x3FFF) * _svm_vh[0x18]) / 16129;
    vol = (a3 * _svm_cur.f0A * _svm_cur.f0D) / 16129U;
    rec = _ss_score[(u16)_svm_cur.f14 & 0xFF] + ((_svm_cur.f14 & 0xFF00) >> 8);
    a3 = vol;
    if (_svm_cur.f14 != 0x21) {
        /* ASM_BARRIER: prevent replacing vol with a3 in first multiply */
        __asm__("" : "+r"(vol));
        a3 = (vol * rec->value58) / 127U;
        vol = (vol * rec->value5A) / 127U;
    }
    pan = _svm_cur.f0E;
    if (pan < 0x40U) {
        left = a3;
        right = (vol * pan) / 63U;
    } else {
        right = vol;
        left = (a3 * (0x7F - pan)) / 63U;
    }
    pan = _svm_cur.f0B;
    if (pan < 0x40U) {
        right = (right * pan) / 63U;
    } else {
        left = (left * (0x7F - pan)) / 63U;
    }
    pan = _svm_cur.f05;
    if (pan < 0x40U) {
        right = (right * pan) / 63U;
    } else {
        left = (left * (0x7F - pan)) / 63U;
    }
    if (_svm_stereo_mono == 1) {
        if (left < right) {
            left = right;
        } else {
            right = left;
        }
    }
    if (_svm_cur.f14 != 0x21) {
        left = (left * left) / 16383U;
        right = (right * right) / 16383U;
    }
    SvmSregPitch[voice_off] = arg1;
    _svm_sreg_buf[voice_off] = left;
    SvmSregVolR[voice_off] = right;
    ((u8 *)&_svm_sreg_dirty)[_svm_cur.f18] |= 7;
    SvmVoicePitch[_svm_cur.f18 * 27] = arg1;
    if (_svm_cur.f18 < 0x10) {
        low_mask = 1 << _svm_cur.f18;
        high_mask = 0;
    } else {
        low_mask = 0;
        high_mask = 1 << (_svm_cur.f18 - 0x10);
    }
    if (_svm_cur.f12 & 4) {
        _svm_orev1 |= low_mask;
        _svm_orev2 |= high_mask;
    } else {
        _svm_orev1 &= ~low_mask;
        _svm_orev2 &= ~high_mask;
    }
    _svm_onos1 &= ~low_mask;
    _svm_onos2 &= ~high_mask;
    _svm_okon2 |= high_mask;
    _svm_okon1 |= low_mask;
    _svm_okof1 &= ~_svm_okon1;
    _svm_okof2 &= ~_svm_okon2;
}

/* legacy typing of shared globals in this part of the unit */
#define _svm_sreg_dirty (*(char (*)[])&_svm_sreg_dirty)
#define SvmSregPitch (*(char (*)[])&SvmSregPitch)
#define _svm_tn (*(int (*))&_svm_tn)

int _SsVmPBVoice(short a0, short a1, short a2, short a3, int a4) {
    short t0;
    int a;
    unsigned short s;
    int e;
    unsigned short idx;
    t0 = a4 - 0x40;
    a = a0 * 0x36;
    if (*(short *)(SvmVoiceSeqSep + a) == a1) {
        if (*(short *)(SvmVoiceVabId + a) == a2) {
            if (*(short *)(SvmVoiceProg + a) == a3) {
                int d1, d2, d3, d4, d5, d6, d7, d8, d9;
                short ret;
                int v0 = SvmCurProgIndex;
                /* ASM_BARRIER: force load order of SvmCurProgIndex, SvmVoiceTone, and SvmVoiceNote */
                __asm__ volatile("" : "+r"(v0));
                idx = *(unsigned short *)(SvmVoiceTone + a);
                /* ASM_BARRIER: force load of SvmVoiceNote before shift/add into idx */
                __asm__ volatile("" : "+r"(idx));
                s = *(unsigned short *)(SvmVoiceNote + a);
                /* ASM_BARRIER: keep s load scheduled before shift/add */
                __asm__ volatile("" : "+r"(s), "+r"(v0), "+r"(idx));
                idx += v0 * 16;
                if (t0 > 0) {
                    int x1 = t0 * *(unsigned char *)((idx << 5) + _svm_tn + 0xD);
                    int sign;
                    int q;
                    /* ASM_BARRIER: reserve $t0 across division so mfhi conflicts with $t0 */
                    register int r8 __asm__("$8");
                    /* ASM_BARRIER: start $t0 live range before sign shift */
                    __asm__ volatile("" : "=r"(r8));
                    sign = x1 >> 31;
                    q = x1 / 63;
                    /* ASM_BARRIER: keep sign and $t0 live across division so sra gets $v1 and mfhi gets $t2 */
                    __asm__ volatile("" : : "r"(sign), "r"(r8));
                    s += q;
                    e = (x1 - q * 63) * 2;
                } else if (t0 < 0) {
                    int x2 = t0 * *(unsigned char *)((idx << 5) + _svm_tn + 0xC);
                    s = s + (x2 / 64) - 1;
                    e = ((x2 - ((x2 / 64) << 6)) * 2) + 0x7F;
                } else {
                    e = 0;
                }
                *(unsigned char *)SvmCurTone = *(unsigned char *)(SvmVoiceTone + a0 * 0x36);
                *(short *)(SvmCurTone + 0xC) = a0;
                ret = note2pitch2(s, e & 0xFFFF);
                *(short *)(SvmSregPitch + a0 * 16) = ret;
                /* ASM_BARRIER: prevent scheduling _svm_sreg_dirty load before SvmSregPitch store */
                __asm__ volatile("");
                *(unsigned char *)(_svm_sreg_dirty + a0) |= 4;
                /* ASM_BARRIER: keep ret ($v0) live across _svm_sreg_dirty update and allocate d9 to $t2 so mfhi picks $t2 */
                __asm__ volatile("" : "=r"(d1), "=r"(d2), "=r"(d3), "=r"(d4), "=r"(d5), "=r"(d6), "=r"(d7), "=r"(d8), "=r"(d9) : "r"(ret));
                /* ASM_BARRIER: end live range of d1..d9 */
                __asm__ volatile("" : : "r"(d1), "r"(d2), "r"(d3), "r"(d4), "r"(d5), "r"(d6), "r"(d7), "r"(d8), "r"(d9));
                return 1;
            }
        }
    }
    return 0;
}

#undef _svm_sreg_dirty
#undef SvmSregPitch
#undef _svm_tn
