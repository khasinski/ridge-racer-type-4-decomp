/* GCC_VERSION: 970318 */
/* MASPSX_FLAGS: --assembler-profile=gnu */
#include "types.h"
#include "rr/spu_tick_control.h"
#include "psyq/libspu/s_i.h"
#include "rr/spu_common.h"

/* Anchors originate in the complete timer object. Signed native-width
 * offset differences keep every scalar access within that object. */
#define FIELD(p,type,member) (*(type *)((p)+((long)RR_FIELD_OFFSET(RrSpuTickState20,member)-(long)RR_FIELD_OFFSET(RrSpuTickState20,flags))))
#define FLAG(p,member) (*(s8 *)((p)+(long)RR_FIELD_OFFSET(RrSpuTickFlags4,member)))

extern void (*SndSeqTickEnvUserCallback[])();
extern s32 SndSeqTickEnvDividedPhase;
extern void (*SndSeqTickEnvCallback)(void);

void SsSetMVol(short a, short b);
void SsStart2(void);

void SsSetMVol(short a, short b) {
    struct { int x; short y, z; int pad[8]; } s;
    s.x = 3; s.y = a * 0x81; s.z = b * 0x81;
    SpuSetCommonAttr(&s);
}

void _SsStart(s32 direct)
{
    s32 wait=999;
    u32 counter;
    register s32 period __asm__("$17");
    RrSpuTickCallback callback;
    register s32 interrupt __asm__("$4");
    u8 *setup;
    u8 *state;
    do {wait--;} while(wait>=0);
    counter=0xF2000002;
    period=0x44E8;
    setup=(u8 *)&_snd_seq_tick_env+RR_FIELD_OFFSET(RrSpuTickState20,flags);
    FLAG(setup,interrupt)=6;
    FLAG(setup,direct)=0;
    FLAG(setup,divided)=0;
    FIELD(setup,RrSpuTickCallback,previous)=0;
    switch(FIELD(setup,s32,selector)) {
    case 0:FLAG(setup,interrupt)=127;return;
    case 5:
        FLAG(setup,interrupt)=0;
        if(direct==0) FLAG(setup,direct)=1;
        else {counter=0xF2000003;period=1;}
        break;
    case 2:break;
    case 3:period=0x89D0;break;
    default:
        {
        register u8 *active=(u8 *)&_snd_seq_tick_env+RR_FIELD_OFFSET(RrSpuTickState20,active);
        if(*(s32 *)active!=0) return;
        {
            register s32 frequency=*(s32 *)(active-4);
            register u8 *parent=active-4;
            if(frequency<70) {
                register s32 quotient=0x204CC0/frequency;
                /* ASM_BARRIER: Capture the initialized quotient before the flag phase. */
                __asm__("" : : "r"(quotient));
                parent[RR_FIELD_OFFSET(RrSpuTickState20,flags)+1]++;
                /* ASM_BARRIER: Publish the divided flag before assigning the period. */
                __asm__("" : : "m"(parent[RR_FIELD_OFFSET(RrSpuTickState20,flags)+1]), "r"(quotient) : "$17");
                period=quotient;
            } else { register s32 quotient __asm__("$2")=0x409980/frequency;  period=quotient; }
        }
        }
    }
    state=(u8 *)&_snd_seq_tick_env+RR_FIELD_OFFSET(RrSpuTickState20,flags);
    if(FLAG(state,direct)) {
        EnterCriticalSection();
        VSyncCallback(FIELD(state,RrSpuTickCallback,callback));
    } else {
        EnterCriticalSection();
        ResetRCnt(counter);
        SetRCnt(counter,(u16)period,0x1000);
        interrupt=FLAG(state,interrupt);
        if(interrupt==0) {
            interrupt=0;
            /* ASM_BARRIER: Keep the initialized IRQ argument distinct from null. */
            __asm__("" : "+r"(interrupt));
            FIELD(state,RrSpuTickCallback,previous)=InterruptCallback(interrupt,0);
            interrupt=FLAG(state,interrupt);
            callback=_SsTickCallback;
        } else {
            callback=_SsTickCallbackDivided;
            if(FLAG(state,divided)==0) callback=FIELD(state,RrSpuTickCallback,callback);
        }
        InterruptCallback(interrupt,callback);
    }
    ExitCriticalSection();
}

void SsStart(void) { _SsStart(1); }

void SsStart2(void) { _SsStart(0); }

void _SsTickCallback(void) {
 void (**p)() = SndSeqTickEnvUserCallback;
 /* ASM_BARRIER: keeps SndSeqTickEnvUserCallback address in one register (lui+addiu) so p[-1] uses a negative offset */
 __asm__("" : "=r"(p) : "0"(p));
 if (p[0] != 0) p[0]();
 p[-1]();
}

void _SsTickCallbackDivided(void) {
    if (SndSeqTickEnvDividedPhase == 0) {
        /* ASM_BARRIER: Retain the assembler temporary for the state publication. */
        register u8 *page __asm__("$1");
        s32 one = 1;
        /* ASM_BARRIER: Materialize the state value before its store address. */
        __asm__ volatile("" : : "r"(one) : "$1", "memory");
        /* PAL mapping: page + 0x4D4C is &SndSeqTickEnvDividedPhase. */
        page = (u8 *)0x800A0000U;
        /* ASM_BARRIER: Expose the PAL store so it can fill the epilogue jump delay slot. */
        __asm__("" : "=r"(page) : "0"(page));
        *(s32 *)(page + 0x4D4C) = one;
    } else {
        /* ASM_BARRIER: Retain the assembler temporary for the state publication. */
        register u8 *page __asm__("$1");
        void (*callback)(void) = SndSeqTickEnvCallback;
        /* ASM_BARRIER: Snapshot the callback before materializing the state-store address. */
        __asm__ volatile("" : : "r"(callback) : "$1", "memory");
        /* PAL mapping: page + 0x4D4C is &SndSeqTickEnvDividedPhase. */
        page = (u8 *)0x800A0000U;
        /* ASM_BARRIER: Expose the PAL store so it can fill the callback call delay slot. */
        __asm__("" : "=r"(page) : "0"(page));
        *(s32 *)(page + 0x4D4C) = 0;
        callback();
    }
}
