/* MASPSX_FLAGS: --assembler-profile=gnu */
/* GCC_VERSION: 2.7.2 */
/* CC1_FLAGS: -fno-rerun-cse-after-loop */
#include "rr/spu.h"
#include "psyq/libspu/smutil.h"

typedef struct {
    u32 mask;
    s32 mode;
    s16 depth_left;
    s16 depth_right;
    s32 delay;
    s32 feedback;
} SpuReverbAttr;

typedef struct {
    s32 mode;
    s16 depth_left;
    s16 depth_right;
    s32 delay;
    s32 feedback;
} RevAttr;

extern RevAttr _spu_rev_attr;
extern s32 _spu_rev_offsetaddr;
extern s32 _spu_rev_startaddr[];
extern RrSpuRegisterPatch SpuReverbPresetTable[];

extern void _spu_FsetRXX();
extern s32 SpuClearReverbWorkArea();
extern void _spu_setReverbAttr();

/* Byte copy of one reverb register preset (inlined at each use). */
static __inline__ void copy(u8 *dst, u8 *src, s32 n) {
    for (n--; n != -1; n--) {
        *dst++ = *src++;
    }
}

s32 SpuSetReverbModeParam(SpuReverbAttr *attr) {
    RrSpuRegisterPatch patch;
    s32 mask;
    s32 all;
    s32 mode_set;
    s32 delay_set;
    s32 fb_set;
    s32 rev_on;
    s32 clear_mode;
    u32 mode;
    /* ASM_BARRIER: pin the start-address table base to $s1 as in the original allocation */
    register s32 *tbl asm("$17");

    rev_on = 0;
    mode_set = 0;
    delay_set = 0;
    clear_mode = 0;
    mask = attr->mask;
    fb_set = 0;
    all = (mask == 0);
    patch.mask = 0;

    if (all || (mask & 1)) {
        mode = attr->mode;
        if (mode & 0x100) {
            mode &= ~0x100;
            clear_mode = 1;
        }
        if (mode >= 10) {
            return -1;
        }
        tbl = _spu_rev_startaddr;
        if (_SpuIsInAllocateArea_(*(s32 *)((u8 *)_spu_rev_startaddr + (mode << 2))) != 0) {
            return -1;
        }
        mode_set = 1;
        _spu_rev_attr.mode = mode;
        _spu_rev_offsetaddr = tbl[_spu_rev_attr.mode];
        copy((u8 *)&patch, (u8 *)&SpuReverbPresetTable[_spu_rev_attr.mode], 0x44);
        switch (_spu_rev_attr.mode) {
        case 7:
            _spu_rev_attr.feedback = 0x7F;
            _spu_rev_attr.delay = 0x7F;
            break;
        case 8:
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0x7F;
            break;
        default:
            _spu_rev_attr.feedback = 0;
            _spu_rev_attr.delay = 0;
            break;
        }
    }

    /* Nested tests keep gcc from folding the 7..8 range check into one compare. */
    if ((all || (mask & 8)) && _spu_rev_attr.mode < 9) if (_spu_rev_attr.mode >= 7) {
        s32 d;
        delay_set = 1;
        if (!mode_set) {
            copy((u8 *)&patch, (u8 *)&SpuReverbPresetTable[_spu_rev_attr.mode], 0x44);
            patch.mask = 0x0C011C00;
        }
        _spu_rev_attr.delay = attr->delay;
        patch.values[10] = ((_spu_rev_attr.delay << 13) / 127) - patch.values[0];
        d = (_spu_rev_attr.delay << 12) / 127;
        patch.values[11] = d - patch.values[1];
        patch.values[12] = patch.values[13] + d;
        patch.values[16] = patch.values[17] + d;
        patch.values[26] = patch.values[28] + d;
        patch.values[27] = patch.values[29] + d;
    }

    if ((all || (mask & 0x10)) && _spu_rev_attr.mode < 9) if (_spu_rev_attr.mode >= 7) {
        fb_set = 1;
        if (!mode_set) {
            if (!delay_set) {
                copy((u8 *)&patch, (u8 *)&SpuReverbPresetTable[_spu_rev_attr.mode], 0x44);
                patch.mask = 0x80;
            } else {
                patch.mask |= 0x80;
            }
        }
        _spu_rev_attr.feedback = attr->feedback;
        patch.values[7] = (_spu_rev_attr.feedback * 0x8100) / 127;
    }

    if (mode_set) {
        rev_on = (_spu_RXX->spu_control >> 7) & 1;
        if (rev_on) {
            _spu_RXX->spu_control &= ~0x80;
        }
    } else {
        if (all || (mask & 2)) {
            ((volatile u16 *)_spu_RXX)[0x184 / 2] = attr->depth_left;
            _spu_rev_attr.depth_left = attr->depth_left;
        }
        if (all || (mask & 4)) {
            ((volatile u16 *)_spu_RXX)[0x186 / 2] = attr->depth_right;
            _spu_rev_attr.depth_right = attr->depth_right;
        }
        goto skip;
    }
    ((volatile u16 *)_spu_RXX)[0x184 / 2] = 0;
    ((volatile u16 *)_spu_RXX)[0x186 / 2] = 0;
    _spu_rev_attr.depth_left = 0;
    _spu_rev_attr.depth_right = 0;
skip:

    if (mode_set || delay_set || fb_set) {
        _spu_setReverbAttr(&patch);
    }

    if (clear_mode) {
        SpuClearReverbWorkArea(_spu_rev_attr.mode);
    }

    if (mode_set) {
        _spu_FsetRXX(0xD1, _spu_rev_offsetaddr, 0);
        if (rev_on) {
            _spu_RXX->spu_control |= 0x80;
        }
    }

    return 0;
}
