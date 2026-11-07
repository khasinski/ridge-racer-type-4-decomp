/* MASPSX_FLAGS: --assembler-profile=gnu */
/* CC1_FLAGS: -mno-split-addresses */
#include "types.h"
#include "psyq/libspu/libspu.h"
#include "psyq/libsnd/vmtick2.h"
#include "psyq/libspu/sminit.h"
#include "game/data/globals.h"



typedef struct {
    s16 f00;
    s16 f02;
    s16 f04;
    s16 f06;
    s16 f08;
    s8 f0A;
    s8 f0B;
    s16 f0C;
    s16 f0E;
    s16 f10;
    s16 f12;
    s16 f14;
    s16 f16;
    u8 pad18[5];
    s8 f1D;
    s16 f1E;
    s16 f20;
    s16 f22;
    s16 f24;
    s16 f26;
    s16 f28;
    s16 f2A;
    s16 f2C;
    s16 f2E;
    s16 f30;
    s16 f32;
    s16 f34;
} SpuVoiceRow36;

typedef struct {
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
} Unk80110008;

extern void SpuSetVoiceAttr(SpuVoiceAttr *);
extern void _spu_setInTransfer(s32);
extern void _SsVmKeyOffNow(s32);

extern SpuVoiceRow36 _svm_voice[];
extern u8 _svm_sreg_dirty[];
extern u16 _svm_sreg_buf[];
extern Unk80110008 _svm_rattr;
extern s16 _svm_damper;
extern s16 kMaxPrograms;
extern s8 spuVmMaxVoice;
extern s16 SvmVoiceIndex;
extern u8 _svm_vab_used[];
extern s16 _svm_vab_count;
extern u8 SpuMallocTable[];

void _SsVmInit(s32 arg0) {
    SpuVoiceAttr attr;
    u16 i;
    u16 *p;
    s32 v;
    s32 c18, cff, cm1, c40, c1;

    _spu_setInTransfer(0);
    _svm_damper = 0;
    SpuInitMalloc(0x20, SpuMallocTable);

    i = 0;
    p = _svm_sreg_buf;
    do {
        p[i] = 0;
        i++;
    } while (i < 0xC0);

    for (i = 0; i < 0x18; i++) {
        _svm_sreg_dirty[i] = 0;
    }

    _svm_vab_count = 0;

    for (i = 0; i < 0x10; i++) {
        _svm_vab_used[i] = 0;
    }

    v = (s8)arg0;
    if ((u32)v >= 0x18) {
        spuVmMaxVoice = 0x18;
    } else {
        spuVmMaxVoice = v;
    }

    attr.mask = 0x60093;
    i = 0;
    attr.pitch = 0x1000;
    attr.addr = 0x1000;
    attr.adsr1 = 0x80FF;
    attr.volume.left = 0;
    attr.volume.right = 0;
    attr.adsr2 = 0x4000;

    if (spuVmMaxVoice > 0) {
        c18 = 0x18;
        cff = 0xFF;
        cm1 = -1;
        c40 = 0x40;
        c1 = 1;
        do {
            _svm_voice[i].f02 = c18;
            _svm_voice[i].f00 = cff;
            _svm_voice[i].f1D = 0;
            _svm_voice[i].f04 = 0;
            _svm_voice[i].f06 = 0;
            _svm_voice[i].f10 = cm1;
            _svm_voice[i].f12 = 0;
            _svm_voice[i].f14 = 0;
            _svm_voice[i].f16 = cff;
            _svm_voice[i].f08 = 0;
            _svm_voice[i].f0C = 0;
            _svm_voice[i].f0A = c40;
            _svm_voice[i].f1E = 0;
            _svm_voice[i].f20 = 0;
            _svm_voice[i].f22 = 0;
            _svm_voice[i].f24 = 0;
            _svm_voice[i].f2A = 0;
            _svm_voice[i].f2C = 0;
            _svm_voice[i].f2E = 0;
            _svm_voice[i].f30 = 0;
            _svm_voice[i].f32 = 0;
            _svm_voice[i].f26 = 0;
            attr.voice = c1 << i;
            SpuSetVoiceAttr(&attr);
            SvmVoiceIndex = i;
            _SsVmKeyOffNow(1);
            i++;
        } while (i < spuVmMaxVoice);
    }

    _svm_rattr.unk0 = 0;
    _svm_rattr.unk8 = 0x3FFF;
    _svm_rattr.unkA = 0x3FFF;
    _svm_rattr.unk4 = 0;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_okof1 = 0;
    _svm_orev1 = 0;
    _svm_orev2 = 0;
    _svm_onos1 = 0;
    _svm_onos2 = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    _SsVmFlush();
}
