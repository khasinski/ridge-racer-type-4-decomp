/* CC1_FLAGS: -fno-schedule-insns */
#include "types.h"
#include "rr/frame_loop.h"
#include "rr/bios_devices.h"

#define W(p,o) (*(int*)((char*)(p)+(o)))

extern int D_800A099C[][3];
extern int D_800A0904[];
extern int CdAudioSetVolumeRaw();
extern int PsyqSrand();
extern int CdAudioSetVolume();
extern struct { char pad[0x2664]; s32 a; char p0[4]; s32 b, c, d, e; char p1[8]; s32 g; char p3[0x14]; s32 h; char p2[0x26CE - 0x26A0]; u16 i; } SoundState;

void CdAudioSelectTrack();
void CdAudioSetTarget(int a0, int a1, int a2);

/* legacy typing of shared globals in this part of the unit */
#define SoundState (*(char (*)[])&SoundState)

void CdAudioSelectTrack(a0, a1)
int a0, a1;
{
    register char *g = SoundState;
    register char *s2;
    register int s1;
    unsigned s0; 
    int a;
    int h;
    unsigned t2;
    int v2, tt;
    register unsigned m asm("$3"); /* ASM_BARRIER: mask constant must live in $v1 */
    int *v; 
    s2 = g + a0 * 8;
    a = W(s2, 0x2638);
    if (a != -1) {
        m = 0xFFF0000;
        /* ASM_BARRIER: keep lui/ori of the 0x0FFFFFFF mask as separate insns for scheduling */
        __asm__("" : "=r"(m) : "0"(m));
        s1 = a1 * 8;
        t2 = s1 + a;
        m |= 0xFFFF;
        s0 = *(unsigned *)t2;
        h = s0 >> 28;
        s0 &= m;
        s0 += W(s2, 0x2634);
        CdAudioSetTarget(s0, h, 0);
        W(g, 0x2690) = (*(int *)(s1 + W(s2, 0x2638) + 4) - 2) * 8;
        PsyqBzero(g + 0x26D0, 0x1C);
        { int b = (int)D_800A099C; v = (int *)(b + a1 * 12); }
        W(g, 0x26EC) = 0;
        W(g, 0x26F0) = 0;
        W(g, 0x26E0) = v[0];
        W(g, 0x26F4) = v[1];
        tt = W(g, 0x2690);
        v2 = v[2];
        W(g, 0x26A8) = 0;
        W(g, 0x2690) = tt + s0;
        W(g, 0x26E8) = v2;
        W(g, 0x26C0) = D_800A0904[a1];
        CdAudioSetVolumeRaw(0);
    }
    PsyqSrand(FrameCounter);
}

#undef SoundState

void CdAudioSetTarget(int a0, int a1, int a2) {
    SoundState.a = a0;
    SoundState.b = a1;
    SoundState.d = 0;
    if (a2 == 0) SoundState.g = a0;
    SoundState.c = -1;
    SoundState.e = 0;
    if (SoundState.h == 2) CdAudioSetVolume(SoundState.i);
}
