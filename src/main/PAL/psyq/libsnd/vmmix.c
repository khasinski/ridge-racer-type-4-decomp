/* MASPSX_FLAGS: --assembler-profile=gnu */
/* GCC_VERSION: 2.7.2 */
/* CC1_FLAGS: -mhard-float */

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef struct 
{
  u8 pad00[0x17];
  u8 unk17;
  u8 pad18[0x58 - 0x18];
  u16 value58;
  u16 value5A;
  u8 pad5C[0x60 - 0x5C];
  s16 unk60[16];
  u8 pad80[0xB0 - 0x80];
} ChannelRecordB0;
typedef struct 
{
  u8 pad00[0x08];
  s16 unk08;
  u8 unk0A;
  u8 pad0B;
  s16 unk0C;
  s16 unk0E;
  s16 match10;
  s16 unk12;
  s16 match14;
  s16 unk16;
  s16 match18;
  u8 pad1A[0x36 - 0x1A];
} ControlRow36;
typedef struct 
{
  u8 value00;
  u8 value01;
  u8 value02;
  u8 value03;
  u8 value04;
  u8 pad05[11];
} SpuProgram10;
typedef struct 
{
  u8 pad00[2];
  u8 unk02;
  u8 unk03;
  u8 pad04[28];
} SpuTone20;
typedef struct 
{
  u8 pad00[0x18];
  u8 unk18;
} SpuBankData;
typedef struct 
{
  s16 left;
  s16 right;
  u8 pad04[12];
} SpuCachedVoice10;
typedef struct 
{
  u8 flags[0x20];
  SpuCachedVoice10 voices[24];
} SpuVolumeCache;
extern ChannelRecordB0 *_ss_score[];
extern ControlRow36 _svm_voice[];
extern s16 D_8010F910[];
extern s16 D_8010F918[];
extern s16 D_8010F91A[];
extern s16 D_8010F91C[];
extern s16 D_8010F91E[];
extern s16 D_8010F920[];
extern SpuVolumeCache _svm_sreg_dirty;
extern SpuCachedVoice10 _svm_sreg_buf[];
extern u32 _snd_vmask;
extern s16 _svm_stereo_mono;
extern SpuProgram10 *_svm_pg;
extern SpuBankData *_svm_vh;
extern SpuTone20 *_svm_tn;
extern s8 spuVmMaxVoice;
extern s16 SvmLastChannel;
extern s32 _SsVmVSetUp();
s32 _SsVmSetVol(s32 a0, s16 a1, s16 a2, u16 a3, u16 a4)
{
  u8 new_var;
  s16 i;
  ChannelRecordB0 *rec;
  SpuTone20 *tone;
  u32 vol;
  u32 left;
  u32 right;
  u32 l;
  u32 r;
  u8 pan;
  /* ASM_BARRIER: pin count to s7 to match original register allocation */
  register s32 count asm("$23") = 0;
  s32 k;
  s16 ch;
  s32 v1_tmp;
  s32 a0_off;
  a0_off = (a0 & 0xFF) << 2;
  /* ASM_BARRIER: schedule a0 & 0xFF before sra 16 */
  __asm__("" : "=r"(a0_off) : "0"(a0_off));
  v1_tmp = ((s16) a0) & 0xFF00;
  /* ASM_BARRIER: prevent combining sra 16 and andi 0xff00 into srl 24 */
  __asm__("" : "=r"(v1_tmp) : "0"(v1_tmp));
  rec = *(ChannelRecordB0 **)(a0_off + (s32)_ss_score) + (v1_tmp >> 8);
  _SsVmVSetUp(a1, a2);
  SvmLastChannel = a0;
  if (a4 == 0)
  {
    a4 = 1;
  }
  if (a3 == 0)
  {
    a3 = 1;
  }
  /* ASM_BARRIER: prevent constant propagation of count=0 into loop header */
  __asm__("" : "=r"(count) : "0"(count));
  i = 0;
  if (count < spuVmMaxVoice)
  do {
    new_var = _svm_vh->unk18;
    if (!(_snd_vmask & (1 << i)))
    {
      if (((_svm_voice[i].match10 == ((s16) a0)) && (_svm_voice[i].match14 == a2)) && (_svm_voice[i].match18 == a1))
      {
        vol = (((_svm_voice[i].unk08 * a3) / 127) * (new_var * 0x3FFF)) / 16129;
        if ((rec->unk60[rec->unk17] != a3) && (rec->unk60[rec->unk17] == 0))
        {
          rec->unk60[rec->unk17] = 1;
        }
        /* ASM_BARRIER: give vol two deaths in block so global-alloc allocates it to a0 */
        __asm__("" : "=r"(vol));
        /* ASM_BARRIER: second death of vol in block for global-alloc */
        __asm__ volatile("" : : "r"(vol));
        l = (((_svm_voice[i].unk08 * a3) / 127) * (k = _svm_vh->unk18 * 0x3FFF)) / 16129;
        tone = (SpuTone20 *)((((_svm_voice[i].unk12 << 4) + _svm_voice[i].unk16) << 5) + (s32)_svm_tn);
        vol = ((l * _svm_pg[a2].value01) * tone->unk02) / 16129;
        l = (vol * rec->value58) / 127;
        r = (vol * rec->value5A) / 127;
        pan = tone->unk03;
        if (pan < 0x40)
        {
          /* ASM_BARRIER: preserve r allocation priority over l in global-alloc */
          __asm__ volatile("" : : "r"(r));
          right = (r * pan) / 63;
          left = l;
        }
        else
        {
          left = (l * (0x7F - pan)) / 63;
          right = r;
        }
        pan = _svm_pg[_svm_voice[i].match14].value04;
        if (pan < 0x40)
        {
          right = (right * pan) / 63;
        }
        else
        {
          left = (left * (0x7F - pan)) / 63;
        }
        pan = a4;
        if (pan < 0x40)
        {
          right = (right * pan) / 63;
        }
        else
        {
          left = (left * (0x7F - pan)) / 63;
        }
        if (_svm_stereo_mono == 1)
        {
          u32 dummy_hi;
          /* ASM_BARRIER: preserve loop counter i allocation priority and mark hi unused for bne delay slot */
          __asm__ volatile("" : "=h"(dummy_hi) : "r"(i));
          if (left < right)
          {
            left = right;
          }
          else
          {
            right = left;
          }
        }
        left = (left * left) / 16383;
        right = (right * right) / 16383;
        *((s16 *) (&_svm_sreg_buf[i].left)) = left;
        *((s16 *) (&_svm_sreg_buf[i].right)) = right;
        count++;
        _svm_sreg_dirty.flags[i] |= 3;
      }
    }
    i++;
  } while (i < spuVmMaxVoice);

  return count;
}
