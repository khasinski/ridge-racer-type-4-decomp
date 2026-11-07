/* CC1_FLAGS: -mno-split-addresses */
#include "rr/channel_pitch.h"
extern u16 note2pitch2(s32 first, s32 second);
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
