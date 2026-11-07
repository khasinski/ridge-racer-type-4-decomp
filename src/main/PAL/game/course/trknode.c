#include "game/car/carphys2.h"
#include "rr/movie_playback_camera.h"
#define W(p,o) (*(int *)((char *)(p) + (o)))
#define H(p,o) (*(short *)((char *)(p) + (o)))

struct S { char pad[0x12]; short x; char pad2[0x3C-0x14]; };

void GetTrackHeadingAt(int a, int b);
void InterpTrackPosition();

void GetTrackHeadingAt(int a, int b)
{
  int n = (a + 1) % TrackNodeCount;
  register struct S *t asm("$5"); register struct S *p asm("$3"); struct S *q; t = TrackNodes; p = &t[a]; q = &t[n]; LerpAngle12(p->x, q->x, b, n);
}

/* legacy typing of shared globals in this part of the unit */
#define TrackNodes ((char *)TrackNodes)

void InterpTrackPosition(a, r, c) int a; int *r; int c; {
    int n = (a + 1) % TrackNodeCount;
    int c2 = c;
    int t = 0x400 - c2;
    char *p = TrackNodes + a * 0x3C;
    char *q = TrackNodes + n * 0x3C;
    r[0] = (t * W(p,8) + c2 * W(q,8)) / 1024;
    r[1] = (t * H(p,0x10) + c2 * H(q,0x10)) / 1024;
    r[2] = (t * W(p,0xC) + c2 * W(q,0xC)) / 1024;
}

#undef TrackNodes
