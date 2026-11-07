/* CC1_FLAGS: -mno-split-addresses */
#include "rr/spu.h"
#include "rr/channel_pitch.h"
#include "psyq/libsnd/vmnote2.h"

u32 _spu_note2pitch(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
u32 _spu_pitch2note(s32 first,s32 second,u32 pitch);
s32 SsUtChangePitch(s32 raw_input, s32 bank, s32 program, s32 group, s32 unused, u32 first_input, u32 second_input);

u32 _spu_note2pitch(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 sum;
    s32 diff;
    s32 q;
    s32 shift;
    s32 rem;
    /* ASM_BARRIER: Keep the adjusted remainder in $v1 before the halfword sign-extension shift. */
    register s32 rem2 __asm__("$3");
    /* ASM_BARRIER: Keep the negated shift count in $a0 for the rounding shift. */
    register s32 neg_shift __asm__("$4");
    /* ASM_BARRIER: Keep the scaled product in $a1 across the final shift and mask. */
    register u32 res __asm__("$5");

    arg3 += arg1;
    sum = arg3 & 0xFFFF;
    diff = (s16)((arg2 + ((u32)sum >> 7)) - arg0);
    arg3 = ({
        /* ASM_BARRIER: Keep the masked sum in $v1 for both the octave shift and fine mask. */
        register s32 s __asm__("$3") = sum;
        __asm__("" : "+r"(s));
        s & 0x7F;
    });
    q = diff / 12;
    shift = q - 2;
    /* ASM_BARRIER: Compute initial shift before the remainder check. */
    __asm__("" : "+r"(shift));
    rem = diff - (q * 12);
    if ((s16)rem < 0) {
        rem2 = rem + 12;
        shift = q - 3;
    } else {
        rem2 = rem;
    }
    /* ASM_BARRIER: Keep the unadjusted remainder live in $a0 so the positive path copies into $v1 in the delay slot. */
    __asm__("" : "+r"(rem2) : "r"(rem));
    res = (s32)(SvmSemitoneTable[(s16)rem2] * SvmPitchFineTable[(u16)arg3]) >> 16;
    if ((s16)shift >= 0) {
        res = 0x3FFF;
    } else {
        neg_shift = -(s16)shift;
        res += 1 << (neg_shift - 1);
        res >>= neg_shift;
    }
    return (u16)res;
}

/* Nonzero low16 pitch, with the fixed PAL tables: both searches always match. */
u32 _spu_pitch2note(s32 first,s32 second,u32 pitch) {
 register s32 top=0;
 register s32 i;
 register s32 note;
 /* ASM_BARRIER: Preserve the fine-search result and accumulated caller word. */
 register s32 fine __asm__("$9");
 s32 value;
 register u16 *table;
 u32 result;
 if((u16)pitch>=0x4000)pitch=0x3FFF;
 i=0;value=(u16)pitch;
 do {if((value>>i)&1)top=i;i++;}while(i<14);
 pitch=(u16)pitch<<(15-top);
 i=11;value=(u16)pitch;
 table=&SvmSemitoneTable[11];
 for(;i>=0;i--,table--) {
  if((u32)value>=*table){note=i;break;}
 }
 value=((u32)(u16)pitch<<15)/SvmSemitoneTable[(u16)note];
{
  register u16 *fine_table;
  i=127;
  fine_table=&SvmPitchFineTable[127];
  value=(u16)value;
  for(;i>=0;i--,fine_table--) {
   if((u32)value>=*fine_table){fine=i;break;}
  }
 }
 {
  /* ASM_BARRIER: Reuse the result register for two consecutive, disjoint sums. */
  register u32 next __asm__("$2")=(u32)(fine+1);
  register u32 total __asm__("$2");
  register u32 part;
  fine=(u32)second+next;
  part=(u32)(top-12);
  total=(u32)note+((u32)first+part*12U);
  {
   /* ASM_BARRIER: Reuse the consumed first argument for the packed low halfword. */
   register u32 packed __asm__("$4")=(u16)fine;
   part=packed>>7;
   note=total+part;
   result=(u32)(u16)note<<8;
   /* ASM_BARRIER: Finish the note bits before masking the independent fine bits. */
   __asm__("" : "+r"(result), "+r"(packed));
   packed&=0x7E;
   /* ASM_BARRIER: Retain unmasked search values until the final result merge. */
   __asm__("" : : "r"(top),"r"(note),"r"(fine));
   return result|packed;
  }
 }
}

s32 SsUtChangePitch(s32 raw_input, s32 bank, s32 program, s32 group, s32 unused, u32 first_input, u32 second_input)
{
    s32 raw_voice=raw_input;
    u32 first=(u16)first_input;
    u32 second=(u16)second_input;
    s32 voice, offset;
    s16 *state;
    u8 *byte_page;
    s32 selected_bank;
    s32 selected_program;
    /* ASM_BARRIER: stage the bank address in the eventual first selector argument. */
    register u8 *bank_page __asm__("$4");
    /* ASM_BARRIER: stage the program address after the incoming bank argument dies. */
    register u8 *program_page __asm__("$5");
    /* ASM_BARRIER: use the original temporary for the final selector address. */
    register u8 *group_page __asm__("$3");
    u16 pitch;
    /* ASM_BARRIER: prepare the first pitch argument before publishing the request. */
    register u32 pitch_first __asm__("$4");
    /* ASM_BARRIER: reuse v1 only after the pitch byte offset has been consumed. */
    register u8 *flags_page __asm__("$3");
    u8 flags;
    if ((u16)raw_voice >= 24) return -1;
    voice = (s16)raw_voice;
    offset = voice * sizeof(RrChannelControlRow36);
    /* ASM_BARRIER: finish the row offset before replacing the incoming a0 value. */
    __asm__("" : : "r"(offset) : "$4");
    bank_page=(u8*)0x80110000U;
    /* ASM_BARRIER: retain the actual PAL bank-field page for indexed addressing. */
    __asm__("" : "+r"(bank_page));
    selected_bank=*(s16*)(bank_page+offset-0x6E0);
    if (selected_bank != (s16)bank) return -1;
    program_page=(u8*)0x80110000U;
    /* ASM_BARRIER: retain the actual PAL program-field page for indexed addressing. */
    __asm__("" : "+r"(program_page));
    selected_program=*(s16*)(program_page+offset-0x6E4);
    if (selected_program != (s16)program) return -1;
    group_page=(u8*)0x80110000U;
    /* ASM_BARRIER: retain the actual PAL group-field page for indexed addressing. */
    __asm__("" : "+r"(group_page));
    if (*(s16*)(group_page+offset-0x6EA) != (s16)group) return -1;
    _SsVmVSetUp(selected_bank, selected_program);
    pitch_first=first;
    state = SvmCurTone.fields08;
    state[0] = 0x21;
    state[2] = raw_voice;
    /* ASM_BARRIER: publish the request halfwords before the fresh row-byte address. */
    __asm__("" : : : "$2","memory");
    byte_page=(u8*)0x80110000U;
    /* ASM_BARRIER: retain the PAL page for the row byte reloaded after selection. */
    __asm__("" : "+r"(byte_page));
    ((RrSpuPitchRequest*)((u8*)state-8))->instrument00=*(byte_page+offset-0x6E2);
    pitch = note2pitch2(pitch_first,second);
    *(u16 *)((voice * sizeof(RrSpuCachedVolumeEntry)) + (unsigned long)_svm_sreg_dirty.voices[0].pad04) = pitch;
    /* ASM_BARRIER: store pitch before reusing its offset register for fresh flags. */
    __asm__("" : : : "$3","memory");
    flags_page=(u8*)0x80110000U;
    /* ASM_BARRIER: retain the PAL flag page for the indexed load. */
    __asm__("" : "+r"(flags_page));
    flags=*(flags_page+voice-0x1E8);
    _svm_sreg_dirty.flags[voice]=flags|4;
    return 0;
}
