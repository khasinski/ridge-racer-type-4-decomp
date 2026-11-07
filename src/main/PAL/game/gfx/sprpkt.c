#include "scratchpad.h"
#include "rr/gpu.h"
#include "game/ui/uipart1.h"
#include "game/race/raceset.h"
#include "rr/sprite_emit.h"
#include "rr/fade_sprite.h"
#include "rr/audio_loop.h"
#include "rr/race_setup.h"
struct O { char pad[0x70]; u32 addr:24; u32 len:8; };
extern s32 SoundQueueEnqueue();
extern s32 RaceState;
extern s32 SplitTimes[];
void DrawSplitTimeHud(a0) s32 a0;
{
    s32 s1, s2, s3, s4, s5, t;
    s32 c;
    RrSpritePacket *p;
    if (RaceType == 0 && RaceState != 0 && RaceState < 4) {
        t = SplitTimes[NextSplitIndex];
        if (t < 0) {
            s3 = -1;
            s4 = -1;
            s2 = 0;
        } else {
            s4 = t;
            if (SplitDeltaTimer > 0) {
                if (a0 == 0) SplitDeltaTimer--;
                s3 = LatchedBestSplitTime;
                s2 = SplitDeltaIndex;
                s4 = ShownSplitTime;
                s1 = SplitTimes[s2] - s3;
                s5 = 0x84;
                if (s1 < 0) {
                    s5 = 0x78;
                    s1 = -s1;
                    c = 0x15;
                } else {
                    c = 0x16;
                }
                if (SplitDeltaTimer == 0x3B && s2 < 2 && a0 == 0) SoundQueueEnqueue(c, 0);
                *(RrSpritePacket **)SCRATCHPAD = GameQueueSprite((char *)CurrentDrawBuf + 0x70, *(RrSpritePacket **)SCRATCHPAD, 0x70, 0x60, 0xC, 0xC, s5, 0xA8, 0x7EF8);
                DrawRaceTime(0x80, 0x60, s1, 0x7EF8, 1);
            } else {
                s3 = BestSplitTimes[NextSplitIndex];
                s2 = NextSplitIndex;
            }
        }
        DrawRaceTime(0x14, 0x1C, s3, 0x7EF8, 0);
        DrawRaceTime(0x14, 0x26, s4, 0x7EF8, 0);
        p = *(RrSpritePacket **)SCRATCHPAD;
        p->len = 4;
        p->code = 0x64;
        p->x = 0xA;
        p->y = 0x26;
        p->w = 0xC;
        p->h = 0xC;
        p->u = s2 * 12 + 12;
        p->v = 0xA8;
        p->clut = 0x7EF8;
        p->code |= 1;
        p->addr = ((struct O *)CurrentDrawBuf)->addr;
        ((struct O *)CurrentDrawBuf)->addr = (u32)p++;
        /* ASM_BARRIER: keeps the SCRATCHPAD pointer store after the bitfield store (original order) */
        __asm__ volatile("" : : : "memory");
        *(RrSpritePacket **)SCRATCHPAD = p;
    }
}
