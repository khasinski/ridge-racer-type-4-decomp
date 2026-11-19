#include "rr/audio_state_types.h"
#include "game/camera/viewcam.h"
#include "types.h"
#include "scratchpad.h"
#include "rr_asm.h"
#include "rr/game_motion.h"
#include "rr/matrix_words.h"
#include "game/demo/opening.h"
#include "game/camera/camrun.h"
#include "game/gfx/motion1.h"
#include "game/gfx/ring.h"
#include "game/camera/campath.h"
#include "rr/bios_devices.h"
#include "rr/overlay665_render.h"

typedef struct {
    PreviewVector8 vertex;
    Vec2s screen;
    s32 reserved0C;
    Vector scale, position;
    PreviewVector8 rotation;
    MatrixWords32 lightMatrix, viewMatrix;
    s32 flags, depth;
} MotionMarkerWork;

extern u16 HudFinalLapBannerParts[];

void DrawPresentationTrackModel(void) {
    MotionMarkerWork work;
    PresentationController *controller;
    u16 *row;
    s32 i, j, x, y, v;
    controller = RaceContext.controller3D8;
    work.position.x = 0;
    work.position.y = 0;
    work.position.z = 0;
    work.rotation.x = (u16)controller->track.position.x;
    work.rotation.y = (u16)controller->track.position.y;
    work.rotation.z = (u16)controller->track.position.z;
    SetupObjectLightMatrices(&work.position.x, &controller->lighting20, work.lightMatrix.w, work.viewMatrix.w);
    v = RaceContext.controller3D8->track.scale;
    work.scale.z = v;
    work.scale.y = v;
    work.scale.x = v;
    ScaleMatrix(work.lightMatrix.w, &work.scale.x);
    SetGteObjectTransform(
        (ObjectTransform38 *)(SCRATCHPAD + 0x7C),
        (const s16 *)&work.position.x,
        work.lightMatrix.w);
    row = HudFinalLapBannerParts; i = 0; y = -0x438;
    do {
        j = 0;
        x = -0x438;
        do {
            if (*row & (0x8000 >> j)) {
                work.vertex.x = x;
                work.vertex.y = y;
                work.vertex.z = 0;
                RR_GTE_RTPS_LDV(&work.vertex.x);
                RR_GTE_SWC2_SXY2(&work.screen);
                RR_GTE_STORE_IR0_THEN_FLAG(&work.flags);
                RR_GTE_STORE_SZ3_SRA2(&work.depth);
                work.depth = work.depth >> 3;
                EmitProjectedMarkerSprt(work.screen.x, work.screen.y, work.depth);
            }
            j++;
            x += 0x90;
        } while (j < 16);
        y += 0x90;
        i++;
        row++;
    } while (i < 16);
    EmitOpeningDrawModePrim();
}

void InitMotionTrack(int selection) {
    MotionTrack44 *track = &RaceContext.controller3D8->track;
    PsyqBzero(track, sizeof(*track));
    track->disabled = 0;
    track->key = &MotionTrackTable[selection];
    track->scale = 0x1000;
}

void UpdateMotionTrack(MotionClock20 *clock) {
    s16 counter, next;
    u32 delta;
    s32 start, code, units;
    s32 rate;
    u32 tempo;
    MotionKey32 *key;
    MotionKey32 *finishKey;
    MotionKey32 *restKey;
    register MotionKey32 *scaleKey __asm__("$4");
    MotionTrack44 *track;
    PresentationController *controller;

    controller = RaceContext.controller3D8;
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
                units *= 2;
                track->elapsed = 0;
                track->duration = (s16)((tempo * units) / 1200U);
                controller->track.state = (controller->track.state + 1U);
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
                delta = (s32)((delta - start) *
                              track->elapsed) / track->duration;
                next = (u16)track->elapsed + 1;
                track->elapsed = next;
                track->scale = (delta + start);
                finishKey = key;
                if (track->duration < next) {
                    /* Keep the first destination write before the key reload. */
                    /* ASM_BARRIER: preserve original register scheduling. */
                    __asm__("" : "+r"(finishKey));
                    track->position.x = (u16)finishKey->to.x;
                    restKey = key;
                    /* ASM_BARRIER: preserve original register scheduling. */
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
            {
                u32 current = track->scale;
                s32 rate;
                track->scale = current + track->velocity;
                rate = track->velocity * 31U;
                track->velocity = rate / 32;
            }
            track->position.x += 16;
            track->position.y += 8;
            track->position.z += 8;
            break;
        }
    }
}

void InitMotionController(void) {
    RaceContext.controller3D8 = &RaceContext.presentation250;
    PsyqBzero(RaceContext.controller3D8, sizeof(PresentationController));
    RaceContext.controller3D8->position.z = -0x1388;
    RaceContext.controller3D8->viewDistance48 = 0x1B8;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[0].x = 0x1C1;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[0].y = -0xDF8;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[0].z = -0x79A;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[0].red = 0xC0;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[0].green = 0xC0;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[0].blue = 0xC0;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[1].x = -0x64;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[1].y = 0;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[1].z = 0x64;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[1].red = 0xC0;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[1].green = 0xC0;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[1].blue = 0xC0;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[2].x = -0xA4D;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[2].y = -0x642;
    RaceContext.controller3D8->lighting20.matrix.fields.vectors[2].z = 0xA86;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[2].red = 0x20;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[2].green = 0x20;
    RaceContext.controller3D8->lighting20.matrix.fields.colors[2].blue = 0x20;
    RaceContext.controller3D8->lighting20.ambient[0] = 0;
    RaceContext.controller3D8->lighting20.ambient[1] = 0;
    RaceContext.controller3D8->lighting20.ambient[2] = 0;
    RaceContext.controller3D8->script64 = HudCircularGaugeVerts;
    RaceContext.controller3D8->scriptOperation6C = RaceContext.controller3D8->script64[RaceContext.controller3D8->scriptIndex68].operation;
    RaceContext.controller3D8->scriptArgument70 = RaceContext.controller3D8->script64[RaceContext.controller3D8->scriptIndex68].argument;
    switch (RaceContext.controller3D8->scriptOperation6C) {
    case 0:
        InitCameraMotion(RaceContext.controller3D8->scriptArgument70);
        break;
    case 1:
        InitMotionPanel(RaceContext.controller3D8->scriptArgument70);
        break;
    case 2:
        InitScrollTrack(RaceContext.controller3D8->scriptArgument70);
        break;
    case 3:
        InitMotionTrack(RaceContext.controller3D8->scriptArgument70);
        break;
    }
}

void RestartMotionSequence(void) {
    RaceContext.controller3D8->scriptIndex68 = 0;
    RaceContext.controller3D8->scriptTimer6A = 0;
    RaceContext.controller3D8->flags60 = 0;
    RaceContext.controller3D8->script64 = MotionSequenceTable[SoundState.seqTrack2868];
    RaceContext.controller3D8->scriptOperation6C = RaceContext.controller3D8->script64[RaceContext.controller3D8->scriptIndex68].operation;
    RaceContext.controller3D8->scriptArgument70 = RaceContext.controller3D8->script64[RaceContext.controller3D8->scriptIndex68].argument;
    switch (RaceContext.controller3D8->scriptOperation6C) {
    case 0:
        InitCameraMotion(RaceContext.controller3D8->scriptArgument70);
        break;
    case 1:
        InitMotionPanel(RaceContext.controller3D8->scriptArgument70);
        break;
    case 2:
        InitScrollTrack(RaceContext.controller3D8->scriptArgument70);
        break;
    case 3:
        InitMotionTrack(RaceContext.controller3D8->scriptArgument70);
        break;
    }
}

void SetPresentationDrawEnabled(s32 arg0) {
    RaceContext.controller3D8->drawEnabled54 = arg0;
}

void SetPresentationFullViewport(s32 arg0) {
    RaceContext.controller3D8->fullViewport50 = arg0;
}

void SetPresentationStencilEnabled(s32 arg0) {
    RaceContext.controller3D8->stencilEnabled58 = arg0;
}
