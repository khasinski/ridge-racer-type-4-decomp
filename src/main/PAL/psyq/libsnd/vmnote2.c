/* CC1_FLAGS: -mno-split-addresses */
extern signed char SvmCurNote[];
extern struct { signed char a; char p[4]; signed char b; } SvmCurProgIndex;
extern char *_svm_tn;
extern unsigned short SsSemitonePitchTable[];
extern unsigned short _svm_ptable[];

int note2pitch();
unsigned short note2pitch2();
unsigned short SsPitchFromNote(int cen_note_high, int cen_note_low, int note_high, int note_low);

int note2pitch() { return SsPitchFromNote(SvmCurNote[0], 0, (unsigned char)SvmCurNote[0xE], (unsigned char)SvmCurNote[0xF]); }

unsigned short note2pitch2(a0, a1) short a0; short a1; {
    short i = SvmCurProgIndex.a * 16 + SvmCurProgIndex.b;
    char *t = (char *)(i * 32 + (int)_svm_tn);
    return SsPitchFromNote(a0, a1, t[4], t[5]);
}

unsigned short SsPitchFromNote(int cen_note_high, int cen_note_low, int note_high, int note_low)
{
    int fine;
    short note;
    short frac;
    short oct;
    short scale;
    short octave;
    int q;
    int t;
    int pitch;
    int shift;

    fine = (short)((note_low & 0xFF) + cen_note_low);
    q = fine / 128;
    t = cen_note_high + q - (note_high & 0xFF);
    note = t;
    frac = fine - (q << 7);
    if (frac < 0) {
        frac = frac + 0x80;
        t--;
        note = t + frac / 128;
    }
    oct = note / 12;
    octave = oct - 2;
    scale = note % 12;
    if (scale < 0) {
        scale = scale + 12;
        octave = oct - 3;
    }
    pitch = (SsSemitonePitchTable[scale] * _svm_ptable[frac]) >> 16;
    if (octave >= 0) {
        pitch = 0x3FFF;
    } else {
        shift = -octave;
        pitch += 1 << (shift - 1);
        pitch = (unsigned)pitch >> shift;
    }
    return pitch;
}
