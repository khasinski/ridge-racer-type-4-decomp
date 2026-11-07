/* RODATA 0x80011178 0x14 */
extern struct {
    char pad0[0x2670];
    int f2670;
    char pad1[0x2678 - 0x2674];
    int f2678;
    char pad2[0x269C - 0x267C];
    int f269C;
    char pad3[0x26BC - 0x26A0];
    int f26BC;
    char pad4[0x26D0 - 0x26C0];
    char blk[0x1C];
} SoundState;
extern unsigned int CdAudioFadeState;
extern int CdAudioSetVolumeRaw();
extern int CdSync();
extern int CdControl();
extern int PsyqBzero();

void CdAudioStopStep()
{
    int r;
    switch (CdAudioFadeState) {
    case 0:
        SoundState.f269C = 4;
        SoundState.f26BC = 0;
        CdAudioSetVolumeRaw(0);
        SoundState.f2678 = SoundState.f2678 + 1;
    case 1:
        if (CdSync(1, 0) == 0) break;
        SoundState.f2678 = SoundState.f2678 + 1;
    case 2:
        if (CdControl(9, 0, 0) == 0) break;
        goto inc;
    case 3:
        r = CdSync(1, 0);
        if (r == 2) {
    inc:
            SoundState.f2678 = SoundState.f2678 + 1;
        } else if (r == 5) {
            CdAudioFadeState = 1;
        }
        break;
    case 4:
        SoundState.f269C = -1;
        SoundState.f2670 = -1;
        SoundState.f2678 = 0;
        PsyqBzero(SoundState.blk, 0x1C);
        break;
    }
}
