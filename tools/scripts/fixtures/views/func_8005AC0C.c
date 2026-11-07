#include "rr/game_motion.h"

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(char (*)[])&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define MotionController (*(int (*))&MotionController)

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define PsyqBzero ((int (*)())PsyqBzero)

#undef PsyqBzero

/* legacy typing of shared globals in this part of the unit */
#define RaceContext (*(struct { char pad[0x3D8]; struct Q_1 *q; } (*))&RaceContext)

#undef RaceContext

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

#undef MotionController

/* legacy typing of shared globals in this part of the unit */
#define MotionController ((void *)MotionController)

#undef MotionController

void UpdateMotionTrack(RrMotionClock20 *clock) {
    s16 counter, next;
    s32 start, code, delta, units;
    s32 rate;
    register s32 velocity __asm__("$4");
    u32 tempo;
    RrMotionKey32 *key;
    RrMotionKey32 *finishKey;
    RrMotionKey32 *restKey;
    register RrMotionKey32 *scaleKey __asm__("$4");
    RrMotionTrack44 *track;
    RrMotionControllerPrefix *controller;

    controller = MotionController;
    track = &controller->track;
    if (track->disabled == 0) {
        switch (controller->track.state) {
        case 0:
            if (clock->active != 0) {
                controller->track.position = track->key->from;
                track->scale = track->key->fromScale;
                code = track->key->timeCode;
                units = (code >> 8) * 12 + (code & 255);
                tempo = clock->tempo;
                /* Capture the clock before doubling the decoded time. */
                __asm__("" :: "r"(tempo));
                units *= 2;
                track->elapsed = 0;
                track->duration = (s16)((tempo * (u32)units) / 1200U);
                controller->track.state = (s32)((u32)controller->track.state + 1U);
                /* Fall through: the first interpolation step is immediate. */
        case 1:
                key = track->key;
                counter = track->elapsed;
                track->position.x = (u16)key->from.x +
                    ((key->to.x - key->from.x) * counter) / track->duration;
                track->position.y = (u16)key->from.y +
                    ((key->to.y - key->from.y) * counter) / track->duration;
                track->position.z = (u16)key->from.z +
                    ((key->to.z - key->from.z) * counter) / track->duration;
                scaleKey = key;
                delta = scaleKey->toScale;
                start = scaleKey->fromScale;
                delta = (s32)(((u32)delta - (u32)start) *
                              (u32)(s32)track->elapsed) / track->duration;
                next = (u16)track->elapsed + 1;
                track->elapsed = next;
                track->scale = (s32)((u32)delta + (u32)start);
                finishKey = key;
                if (track->duration < next) {
                    /* Keep the first destination write before the key reload. */
                    __asm__("" : "+r"(finishKey));
                    track->position.x = (u16)finishKey->to.x;
                    restKey = key;
                    __asm__("" : "+r"(restKey) :: "memory");
                    track->position.y = (u16)restKey->to.y;
                    track->position.z = (u16)restKey->to.z;
                    track->scale = restKey->toScale;
                    if (restKey->continueAfter == 0) {
                        track->state = 3;
                        return;
                    }
                    track->state = 2;
                    track->velocity = 800;
                    return;
                }
            } else {
                return;
            }
            break;
        case 2:
            /* Publish scale, damped velocity, then x/z/y in wire order. */
            velocity = track->velocity;
            rate = (s32)((u32)velocity << 5);
            {
                s32 current = track->scale;
                s32 carry = velocity;
                __asm__("" :: "r"(current));
                __asm__("" : "+r"(carry) : "r"(rate));
                rate = (s32)((u32)rate - (u32)velocity);
                track->scale = (s32)((u32)current + (u32)carry);
            }
            if (rate < 0) {
                rate += 31;
            }
            {
                u16 x, y, z;
                s32 damped;
                x = (u16)track->position.x;
                __asm__("" : "+r"(rate), "+r"(x));
                damped = rate >> 5;
                __asm__("" :: "r"(rate), "r"(damped));
                track->velocity = damped;
                z = (u16)track->position.z;
                track->position.x = x + 16;
                y = (u16)track->position.y;
                track->position.z = z + 8;
                __asm__("" :: "r"(y));
                track->position.y = y + 8;
            }
            break;
        }
    }
}
