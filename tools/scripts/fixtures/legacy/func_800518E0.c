#include "rr/audio_loop.h"

void SoundLoopUpdate(void) {
    RrAudioLoopState *state=&SoundState;
    RrAudioSettings *settings=&Options.audio;
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register u16 left asm("$19"),right asm("$18");
    s32 rp,lp;
    s32 rv,lv;
    s32 gain,volume;
    s32 rdiv,ldiv;
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register s32 rinput asm("$2"),linput asm("$2");
    /* ASM_BARRIER: retain this phase value in its original MIPS register. */
    register s32 lresult asm("$3");
    s32 short_right;
    s32 i;
    left=state->left19F8;right=state->right19FA;
    if(settings->mono32) {
        /* ASM_BARRIER: retain this phase value in its original MIPS register. */
        register s32 l asm("$2")=(s16)left;
        /* ASM_BARRIER: retain this phase value in its original MIPS register. */
        register s32 r asm("$3")=(s16)right;

        l=(l+r)/2;

        left=l;
        /* ASM_BARRIER: preserve this arithmetic or helper boundary. */
        __asm__("" : "+r"(l));
        right=l;
    }
    if(state->muted04) {right=0;left=right;}
    if(RaceType==5) {right=0;left=right;}
    rinput=(s32)((u32)right<<16);

    gain=state->gain18;
    rinput>>=16;
    rp=rinput*gain;
    if(rp<0)rp+=127;

    linput=(s16)left;

    lp=linput*gain;
    rdiv=rp>>7;
    if(lp<0)lp+=127;

    rdiv=(s32)((u32)rdiv<<16);

    volume=settings->volume31;
    rv=(rdiv>>16)*volume;
    ldiv=lp>>7;
    if(rv<0)rv+=15;

    lv=(s16)ldiv*volume;
    rv>>=4;

    right=rv;
    if(lv<0)lv+=15;
    lresult=lv>>4;

    left=lresult;

    lresult=(s16)lresult;
    short_right=(s16)rv;

    lresult+=short_right;

    i=0;
    if(lresult>0) {
        RrAudioLoopState *live=state;
        /* ASM_BARRIER: retain this phase value in its original MIPS register. */
        register s32 voice_word asm("$17");
        /* ASM_BARRIER: retain this phase value in its original MIPS register. */
        register s32 left_high asm("$20"),right_high;
        u32 pitch;
        s32 first;
        s32 second;
        pitch=live->pitch19FC;

        voice_word=0x60000;
        first=(s32)((u32)pitch<<9)>>16;
        second=pitch&127;
        do {
            /* ASM_BARRIER: retain this phase value in its original MIPS register. */
            register s32 out_left asm("$5");
            /* ASM_BARRIER: retain this phase value in its original MIPS register. */
            register s32 voice asm("$4");
            /* ASM_BARRIER: retain this phase value in its original MIPS register. */
            register s32 group asm("$7");
            /* ASM_BARRIER: retain this phase value in its original MIPS register. */
            register s32 out_voice asm("$4");

            voice=i+6;

            left_high=(s32)((u32)left<<16);
            out_left=left_high>>16;

            right_high=(s32)((u32)right<<16);
            SetVoiceVolumeScaled(voice,out_left,right_high>>16);
            out_voice=voice_word>>16;

            group=60;
            /* ASM_BARRIER: preserve this arithmetic or helper boundary. */
            __asm__("" : : "r"(i));

            i++;
            SsUtChangePitch(out_voice,live->bank26,live->program1A04,group,0,first,second);
            /* ASM_BARRIER: preserve this arithmetic or helper boundary. */
            __asm__("" : "+r"(left_high), "+r"(right_high));
            voice_word+=0x10000;
        }while(i<2);
        {
            RrAudioLoopState *after=&SoundState;
            if(!after->active1A00) {
                s32 key=60;

                SsUtKeyOnV(6,after->bank26,after->program1A04,0,key,0,left_high>>16,right_high>>16);
                /* ASM_BARRIER: preserve this arithmetic or helper boundary. */
                __asm__("" : : "r"(left_high));
                after->active1A00=1;
            }
        }
    }else if(state->active1A00) {
        SsUtKeyOffV(6);
        state->active1A00=0;
    }
}
