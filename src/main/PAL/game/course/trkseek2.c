#include "game/course/trkseek2.h"
#include "rr/track_projection.h"
typedef struct { char p0[0x38]; short s38; unsigned short s3A; unsigned short s3C; char p3E[0xC]; unsigned short s4A; int w4C; char p50[0x1A]; short s6A; } B;
typedef struct { char p0[0x172]; unsigned short s172; char p174[0x2C]; short s1A0; char p1A2[0x36]; short s1D8; char p1DA[0xA]; short s1E4; char p1E6[0xE]; B b; char pad[0x320 - 0x1F4 - sizeof(B)]; } A;
extern A ComCars[];
void UpdateComCarTrackProjections()
{
    int s2 = 0;
    A *s1 = ComCars;
    B *s0;
    int v;
    unsigned short u;
    do {
        s1 = &ComCars[s2];
        if (s1->s1E4 > 0) {
            s0 = &s1->b;
            s0->s4A = s0->s4A - 1;
            if ((short)s0->s4A < 0) {
                s0->s4A = 0;
            }
            if (s1->s1A0 != 0) {
                s0->s3A = s0->s3C;
                UpdateCarTrackProjection(s1);
                s0->s38 = 10;
                u = s1->s172;
                s0->s6A = 1;
                s0->s3C = u;
                v = s1->s1D8; if (v < 0) v = 0x20; else v = 0x10;
                s0->s4A = v;
                v = s1->s1D8;
                if (v < 0) v = -v;
                s0->w4C = v << 8;
            } else {
                s0->s4A = 0;
            }
        }
        s2++;
    } while (s2 < 0xB);
}

