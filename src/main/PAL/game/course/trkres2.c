#include "types.h"
#include "rr/steering.h"
#include "rr/track_resources.h"
#include "game/course/trkres2.h"
#include "game/gfx/dispenv.h"


void BuildRateCurve(Curve24 *curve, s32 displacement, s32 duration, s32 initialRate);
s32 EvalRateCurve(const Curve24 *curve, s32 time, s32 *output);

/* Build a two-phase rate curve in twelve-bit fixed-point units. */
void BuildRateCurve(Curve24 *curve, s32 displacement, s32 duration, s32 initialRate)
{
    s32 half = duration / 2;
    s32 scaledDisplacement = displacement * 4096;
    s32 middleRate;
    s32 scaledInitialRate = initialRate * 4096;
    curve->initialRate = scaledInitialRate;
    middleRate = scaledDisplacement / half - (initialRate * 2048);
    curve->displacement = scaledDisplacement;
    curve->duration = duration;
    curve->middleRate = middleRate;
    curve->firstAcceleration = (middleRate - curve->initialRate) / half;
    curve->secondAcceleration = -middleRate / half;
}

s32 EvalRateCurve(const Curve24 *curve, s32 time, s32 *output)
{
    s32 half;
    s32 rate;
    if (time >= curve->duration) {
        *output = -(curve->secondAcceleration / 2);
        return -1;
    }
    half = curve->duration / 2;
    if (time < half) {
        rate = curve->initialRate + curve->firstAcceleration / 2 + curve->firstAcceleration * time;
    } else {
        time -= half;
        rate = curve->middleRate + curve->secondAcceleration / 2 + curve->secondAcceleration * time;
    }
    *output = rate;
    return 0;
}

unsigned long AlignUp4(unsigned long a) {
    unsigned long v = a;
    unsigned long r = v & 3;
    if (r != 0) {
        v += 4 - r;
    }
    return v;
}

void RelocateTrackLookupTables(TrackEventDirectory *tables)
{
    RaceContext.lookupTables = tables;
    tables->pairs0.entries = (TrackEventPair *)(tables->pairs0.offset + (unsigned long)tables);
    RaceContext.lookupTables->pairs1.entries = (TrackEventPair *)(RaceContext.lookupTables->pairs1.offset + (unsigned long)tables);
    RaceContext.lookupTables->table0.entries = (TrackEventEntry *)(RaceContext.lookupTables->table0.offset + (unsigned long)tables);
    RaceContext.lookupTables->table1.entries = (TrackEventEntry *)(RaceContext.lookupTables->table1.offset + (unsigned long)tables);
    RaceContext.lookupTables->table2.entries = (TrackEventEntry *)(RaceContext.lookupTables->table2.offset + (unsigned long)tables);
    RaceContext.lookupTables->table3.entries = (TrackEventEntry *)(RaceContext.lookupTables->table3.offset + (unsigned long)tables);
    RaceContext.lookupTables->pairs2.entries = (TrackEventPair *)(RaceContext.lookupTables->pairs2.offset + (unsigned long)tables);
}

void SetTrackLaneOffsets(TrackLaneOffset8 *offsets)
{
    RaceContext.laneOffsets54C = offsets;
}

void SetTrackStartNode(RaceStartGrid60 *startGrid)
{
    RaceContext.startGrid53C = startGrid;
}

void SetTrackWorkspace(u8 *workspace)
{
    SceneRaceContext *context = &RaceContext;
    context->workspace554 = workspace;
    workspace += 0xA00;
    context->lightPresets558 = (TrackLightPreset8 *)workspace;
    workspace += 0x20;
    context->workspace55C = workspace;
    workspace += 0x200;
    context->cameraNodes550 = (CamNode44 *)workspace;
}

void LoadTrackResource(TrackResourceFile *file)
{
    RelocateTrackLookupTables((TrackEventDirectory *)((u8 *)file + file->lookupOffset04));
    SetTrackLaneOffsets((TrackLaneOffset8 *)((u8 *)file + file->laneOffset08));
    SetTrackStartNode((RaceStartGrid60 *)((u8 *)file + file->startNodeOffset0C));
    SetTrackWorkspace((u8 *)file + file->workspaceOffset10);
}

void SetCarSpecTable(RaceSpecEntry30 *specs) {
    RaceContext.carSpecs540 = specs;
}
