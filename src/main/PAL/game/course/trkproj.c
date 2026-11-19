#include "rr/scene_camera_state.h"
#include "types.h"
#include "rr/scene_camera_state.h"
#include "game/course/control.h"
#include "rr/track_projection.h"
#include "rr/geometry.h"
#include "rr/trig.h"
#include "rr/race_front_search.h"
#include "rr/track_interpolation.h"
#include "rr/race.h"
#include "rr/track_lateral.h"
#include "game/data/globals.h"
#include "game/course/trkproj.h"
#include "rr/audio_loop.h"
#include "rr/vehicle_drive.h"

/* Keep the original table-relative cursors, while using the canonical native
 * record stride and array offset (the pointer-bearing row grows on a host). */
#define RR_STANDING_PREFIX_SIZE \
    (sizeof(RaceStandingsTable)-sizeof(((RaceStandingsTable*)0)->entries18))
#define RR_STANDING_ROW(base) \
    ((RaceStandingEntry0C*)((base)+RR_STANDING_PREFIX_SIZE))
#define SIN(x) (((x) & 0x800) ? (((x) & 0x400) ? -SinTable[0x400 - ((x) & 0x3FF)] : -SinTable[(x) & 0x3FF]) : (((x) & 0x400) ? SinTable[0x400 - ((x) & 0x3FF)] : SinTable[(x) & 0x3FF]))

extern s8 CarShadowOffsetTableA[][16];
extern s8 CarShadowOffsetTableB[][16];

void ProjectToTrackSegment(TrackOffsetArgument offsetX, TrackOffsetArgument offsetZ, s32 node, s16 *fraction, s16 *lateral, s32 *remaining);
s32 ScoreOvertakeOpportunity(RaceVehicle *arg0, s32 arg1);
void InterpolateLanePointXZ(const TrackSample3C *first, const TrackSample3C *second, s32 lateral, s32 fraction, s32 *output);
void InterpolateLanePointXYZ(const TrackSample3C *first, const TrackSample3C *second, s32 lateral, s32 fraction, s32 *output);

void ComputeCarTrackSurfaceProj(RaceVehicle *vehicle)
{
    struct {
        TrackOffsetArgument a;
        TrackOffsetArgument b;
        Vector first;
        Vector second;
        s16 fraction;
        int remaining;
    } line;
    int distance = 0x400 - vehicle->trackFraction170;
    int rounded = distance;
    int firstNode;
    int secondNode;
    int fraction;
    TrackLaneOffset8 *offset;
    TrackLaneOffset8 *sample;
    RaceVehicleControl *control = &vehicle->control;
    if (rounded < 0) {
        rounded += 0x3FF;
    }
    firstNode = vehicle->trackNodeF0 - (rounded >> 10);
    if (firstNode < 0) {
        firstNode += TrackNodeCount;
    }
    secondNode = firstNode - 1;
    if (secondNode < 0) {
        secondNode += TrackNodeCount;
    }
    offset = RaceContext.laneOffsets54C + firstNode;
    sample = offset;
    line.first.x = sample->x;
    line.first.z = sample->z;
    sample = RaceContext.laneOffsets54C + secondNode;
    line.second.x = sample->x;
    line.second.z = sample->z;
    fraction = distance & 0x3FF;
    {
        int dx = (line.second.x - line.first.x) * fraction;
        int dz;
        if (dx < 0) {
            dx += 0x3FF;
        }
        dz = (line.second.z - line.first.z) * fraction;
        line.a.value = line.first.x + (dx >> 10);
        if (dz < 0) {
            dz += 0x3FF;
        }
        line.b.value = line.first.z + (dz >> 10);
    }
    ProjectToTrackSegment(line.a, line.b, vehicle->trackNodeF0, &line.fraction, &vehicle->control.laneCurrent3E, &line.remaining);
    if (control->laneTimer38 == 0) {
        control->laneTarget3C = control->laneCurrent3E;
    }
}

#define SIN(x) (((x) & 0x800) ? (((x) & 0x400) ? -SinTable[0x400 - ((x) & 0x3FF)] : -SinTable[(x) & 0x3FF]) : (((x) & 0x400) ? SinTable[0x400 - ((x) & 0x3FF)] : SinTable[(x) & 0x3FF]))

void ProjectToTrackSegment(TrackOffsetArgument offsetX, TrackOffsetArgument offsetZ, s32 node, s16 *fraction, s16 *lateral, s32 *remaining) {
    s32 nextNode;
    TrackSample3C *sample;
    TrackSample3C *nextSample;
    s32 angle;
    s32 cosine;
    s32 along;
    s32 across;
    s32 position;
    s32 width;
    s32 nextWidth;

    nextNode = (node + 1) % TrackNodeCount;
    sample = &TrackNodes[node];
    nextSample = &TrackNodes[nextNode];

    angle = sample->road.heading + 0x800;
    node = sample->road.heading + 0x400;

    across = SIN(angle);
    cosine = SIN(node);

    along = (across * offsetX.value + cosine * offsetZ.value) / 4096;
    across = (-cosine * offsetX.value + across * offsetZ.value) / 4096;

    if (along < 0) {
        along = 0;
    }

    node = nextSample->road.heading - sample->road.heading;
    node = sample->road.segmentLength - ((across * SIN(node)) / 4096);
    if (along > node) {
        along = node;
    }

    position = (along << 10) / node;
    *fraction = position;
    *remaining = node - ((node * (s16)position) >> 10);

    if (across < 0) {
        width = sample->road.leftWidth + sample->road.leftExtension;
        nextWidth = nextSample->road.leftWidth + nextSample->road.leftExtension;
        node = width - nextWidth;
        if (node == 0) {
            *lateral = (across << 10) / width;
        } else {
            node = ((node * *fraction) / 1024) + nextWidth;
            *lateral = (across << 10) / node;
        }
    } else {
        width = sample->road.rightWidth + sample->road.rightExtension;
        nextWidth = nextSample->road.rightWidth + nextSample->road.rightExtension;
        node = width - nextWidth;
        if (node == 0) {
            *lateral = (across << 10) / width;
        } else {
            node = ((node * *fraction) / 1024) + nextWidth;
            *lateral = (across << 10) / node;
        }
    }
}

void UpdateCarTrackProjection(RaceVehicle *vehicle)
{
    struct { TrackOffsetArgument x, z; } offset;
    offset.x.value = vehicle->position20.words[0] - TrackNodes[vehicle->trackNodeF0].road.x;
    offset.z.value = vehicle->position20.words[2] - TrackNodes[vehicle->trackNodeF0].road.z;
    ProjectToTrackSegment(offset.x, offset.z, vehicle->trackNodeF0,
        &vehicle->trackFraction170, &vehicle->lateralOffset172, &vehicle->second);
}

s32 FindNearestCarAhead(RaceVehicle *vehicle) {
    s32 best=-1,index=0,selected,limit,count,fresh_count,id;
    s32 first_count,second_count,distance,half,negative_half;
    /* ASM_BARRIER: keep the captured track length in the consumed mode register. */
    register s32 circumference __asm__("$5");
    RaceStandingsTable *table;
    /* ASM_BARRIER: retain the second pass's table-relative cursor. */
    register u8 *cursor __asm__("$9");
    u8 *reference;
    SceneRaceContext *context;
    u32 flags;
    s32 input;
    u8 *lookup_cursor;
    context=&RaceContext;
    flags=vehicle->flags1E4;
    circumference=context->playerCount14;
    input=vehicle->input;
    table=context->standings;
    flags=(flags&15)+best;
    input++;
    limit=input*16;
    count=NumCars;
    selected=(flags+circumference);
    if(count>0) {
        first_count=count;
        lookup_cursor=(u8*)table;
lookup:
        if(selected==RR_STANDING_ROW(lookup_cursor)->unk00)selected=index;
        else {
            index++;
            lookup_cursor+=sizeof(RaceStandingEntry0C);
            if(index<first_count)goto lookup;
        }
    }
    /* ASM_BARRIER: preserve the original second count capture after lookup. */
    __asm__("" : : : "memory");
    index=0;
    fresh_count=NumCars;
    if(fresh_count>0) {
        reference=(u8*)table+selected*sizeof(RaceStandingEntry0C);
        second_count=fresh_count;
        circumference=TrackLength;
        cursor=(u8*)table;
        half=circumference/2;
        negative_half=(s32)(0U-circumference)/2;
        do {
            if(index!=selected) {
                id=RR_STANDING_ROW(cursor)->unk00;
                distance=table->entries18[id].totalDist04;
                distance=((u32)distance-RR_STANDING_ROW(reference)->totalDist04);
                distance%=circumference;
                if(distance>=half)distance=((u32)distance-circumference);
                if(distance<negative_half)distance=((u32)distance+circumference);
                if(distance>0&&distance<limit) {
                    best=id;
                    limit=distance;
                    table->gap14=distance;
                }
            }
            index++;
            cursor+=sizeof(RaceStandingEntry0C);
        }while(index<second_count);
    }
    return best;
}

s32 ScoreOvertakeOpportunity(RaceVehicle *arg0, s32 arg1) {
    s32 sp18;
    s32 sp1C;
    s32 s0;
    RaceVehicleControl *s1;
    RaceStandingsTable *t;
    RaceVehicle *other;
    s32 i;
    s32 v1;
    s32 a2;
    s32 a1;
    s16 temp_v0;
    s32 v1_2;

    s0 = 0;
    t = RaceContext.standings;
    s1 = &arg0->control;
    other = t->entries18[arg1].vehicle08;

    i = -other->heading1D4 + 0x400;
    i -= TrackNodes[other->trackNodeF0].road.heading;
    v1 = -((SIN(i) * other->input) / 4096);
    a1 = t->gap14;
    a2 = s1->lateralSpeed58 - v1;
    a1 /= 100;
    if (a1 >= 16) {
        a1 = 15;
    }

    for (i = 0; i < 2; i++) {
        s0 += CarShadowOffsetTableA[i][a1];
    }

    a2 = (a2 * 160) / 1168;
    if (a2 > 0) {
        if (a2 >= 80) {
            a2 = 79;
        }
        a2 /= 5;
        if (GameModeType != 5) {
            s0 += CarShadowOffsetTableA[2][a2];
        } else {
            s0 += CarShadowOffsetTableB[2][a2];
        }
    } else {
        s0 -= 8;
    }

    s1->curveDistance70 = ScanCurveAhead(arg0, 0, &s1->curveSample80, &s1->relDist74, &s1->relDist78);
    s1->previewDistance7C = ScanCurveAhead(arg0, 10, &s1->previewSample84.node, &sp18, &sp1C);
    temp_v0 = CalcSteerLimit(arg0);
    s1->limit6E = temp_v0;
    v1_2 = temp_v0 << 12;

    if (s0 > 0) {
        if (v1_2 < s1->previewDistance7C) {
            s0 = 0;
        }
    }
    return s0;
}

void UpdateComCarOvertake(RaceVehicle *p)
{
  RaceVehicleControl *q;
  RaceStandingsTable *s5;
  RaceVehicle *s1;
  int x8;
  int u1;
  int u2;
  int t8;
  int ab;
  int w1;
  int cnt;
  int flg;
  int n;
  int ab2;
  int th2;
  int c0;
  int e1;
  int e3;
  int s2;
  int s4;
  int s6;
  int a0;
  int v0;
  int v1;
  s6 = 0;
  s4 = -1;
  s5 = RaceContext.standings;
  q = &p->control;
  s2 = FindNearestCarAhead(p);
  if (s2 >= 0)
  {
    s1 = s5->entries18[s2].vehicle08;
    if ((ScoreOvertakeOpportunity(p, s2) > 0) && (q->boostTimer44 == 0))
    {
      if (q->laneActive6A == 0)
      {
        e1 = (u16)q->laneTarget3C;
        q->laneTimer38 = 10;
        q->laneActive6A = 1;
        q->laneMode88 = 0;
        q->laneStart3A = e1;
        ab = s5->gap14;
        if (ab < 0)
        {
          ab = -ab;
        }
        e3 = (short)((u16)p->halfSpan10E);
        e3 = e3 + (e3 / 2);
        if (ab < e3)
        {
          if (s1->lateralOffset172 < p->lateralOffset172)
          {
            q->laneTarget3C = (s1->lateralOffset172 + 0x400) / 2;
          }
          else
          {
            q->laneTarget3C = (s1->lateralOffset172 - 0x400) / 2;
          }
        }
        else
        {
          if (s1->lateralOffset172 > 0)
          {
            q->laneTarget3C = s1->lateralOffset172 - 0x200;
          }
          else
          {
            q->laneTarget3C = s1->lateralOffset172 + 0x200;
          }
        }
        if (q->laneTarget3C < (-0x220))
        {
          q->laneTarget3C = -0x220;
        }
        if (q->laneTarget3C > 0x220)
        {
          q->laneTarget3C = 0x220;
        }
        q->laneDelta6C = ((u16)q->laneTarget3C) - ((u16)q->laneStart3A);
      }
    }
    else
    {
      s2 = -1;
    }
  }
  if (NumPlayers > 0)
  {
    x8 = s5->playerDistance[0];
    if ((x8 < 0) && (q->metric12A < x8))
    {
      s4 = 0;
    }
    if (NumPlayers == 2)
    {
      if ((s5->playerDistance[0] < s5->playerDistance[1]) && (q->metric12A < s5->playerDistance[1]))
      {
        s4 = 1;
      }
    }
    if (s4 >= 0)
    {
      if ((q->laneActive6A == 0) && (q->laneMode88 == 0))
      {
        s1 = &PlayerCars[s4];
        w1 = q->relDist74 + q->relDist78;
        if (w1 < 0)
        {
          if (s1->lateralOffset172 < p->lateralOffset172)
          {
            cnt = 0;
            u1 = (u16)q->laneTarget3C;
            q->laneTimer38 = 10;
            q->laneActive6A = 1;
            q->laneMode88 = 2;
            q->laneStart3A = u1;
          }
        }
        else
        {
          t8 = s1->lateralOffset172;
          if (p->lateralOffset172 < t8)
          {
            u2 = (u16)q->laneTarget3C;
            q->laneTimer38 = 10;
            q->laneActive6A = 1;
            q->laneMode88 = 1;
            q->laneStart3A = u2;
          }
        }
        s2 = 0;
      }
    }
    else
    {
      q->laneMode88 = 0;
    }
  }
  else
  {
    q->laneMode88 = 0;
  }
  if (s2 < 0)
  {
    cnt = 0;
    n = NumCars;
    flg = cnt;
    if (n > 0)
    {
      do
      {
        s1 = s5->entries18[cnt].vehicle08;
        if (s1 != p)
        {
          ab2 = s5->entries18[cnt].totalDist04 - (p->first + p->second);
          if (ab2 < 0)
          {
            ab2 = -ab2;
          }
          th2 = (short)((u16)p->halfSpan10E);
          th2 = th2 + (th2 / 2);
          if (ab2 < th2)
          {
            c0 = q->laneCurrent3E;
            if (c0 < q->laneStart3A)
            {
              if (s1->lateralOffset172 > (c0 - 0x100))
              {
                s6 = 1;
              }
            }
            else
            {
              if (s1->lateralOffset172 < (c0 + 0x100))
              {
                s6 = 1;
              }
            }
          }
        }
        cnt++;
      }
      while (cnt < NumCars);
    }
    switch (q->laneMode88)
    {
      case 1:
        if (q->laneStart3A < q->laneCurrent3E)
      {
        flg = 1;
      }
        break;

      case 2:
        if (q->laneCurrent3E < q->laneStart3A)
      {
        flg = 1;
      }
        break;

    }

    if (((s6 == 0) && (q->laneTimer38 != 0)) && (flg == 0))
    {
      v0 = q->laneCooldown40;
      if (v0 > 0)
      {
        q->laneCooldown40 = v0 - 1;
      }
      else
      {
        q->laneTimer38 = 0;
        q->laneActive6A = 1;
        ComputeCarTrackSurfaceProj(p);
        v1 = (u16)q->laneTarget3C;
        a0 = (u16)q->laneStart3A;
        q->laneMode88 = 3;
        q->laneDelta6C = v1 - a0;
      }
    }
  }
}

static __inline__ s32 extend_track_edge(s32 edge, s32 width, s32 extension)
{
    return edge * (width + extension) / width;
}

/* Interpolate the lateral road position in the horizontal plane. */
void InterpolateLanePointXZ(const TrackSample3C *first, const TrackSample3C *second,
    s32 lateral, s32 fraction, s32 *output)
{
    Vector firstPoint;
    Vector secondPoint;
    s32 offset;
    const TrackSample3C *sample;

    offset = __builtin_abs(lateral);
    sample = first;
    if (lateral < 0) {
        firstPoint.x = extend_track_edge(sample->leftOffsetX04, sample->road.leftWidth, sample->road.leftExtension);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->road.x;
        firstPoint.z = extend_track_edge(sample->leftOffsetZ06, sample->road.leftWidth, sample->road.leftExtension);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->road.z;
        secondPoint.x = extend_track_edge(second->leftOffsetX04, second->road.leftWidth, second->road.leftExtension);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->road.x;
        secondPoint.z = extend_track_edge(second->leftOffsetZ06, second->road.leftWidth, second->road.leftExtension);
        secondPoint.z = (secondPoint.z * offset) / 1024 + second->road.z;
    } else {
        firstPoint.x = extend_track_edge(sample->rightOffsetX00, sample->road.rightWidth, sample->road.rightExtension);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->road.x;
        firstPoint.z = extend_track_edge(sample->rightOffsetZ02, sample->road.rightWidth, sample->road.rightExtension);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->road.z;
        secondPoint.x = extend_track_edge(second->rightOffsetX00, second->road.rightWidth, second->road.rightExtension);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->road.x;
        secondPoint.z = extend_track_edge(second->rightOffsetZ02, second->road.rightWidth, second->road.rightExtension);
        secondPoint.z = (secondPoint.z * offset) / 1024 + second->road.z;
    }

    output[0] = (secondPoint.x - firstPoint.x) * fraction / 1024 + firstPoint.x;
    output[2] = (secondPoint.z - firstPoint.z) * fraction / 1024 + firstPoint.z;
}

/* The camera path also needs the height of the centre line. */
void InterpolateLanePointXYZ(const TrackSample3C *first, const TrackSample3C *second,
    s32 lateral, s32 fraction, s32 *output)
{
    Vector firstPoint;
    Vector secondPoint;
    s32 offset;
    const TrackSample3C *sample;

    offset = __builtin_abs(lateral);
    sample = first;
    if (lateral < 0) {
        firstPoint.x = extend_track_edge(sample->leftOffsetX04, sample->road.leftWidth, sample->road.leftExtension);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->road.x;
        firstPoint.y = sample->road.height;
        firstPoint.z = extend_track_edge(sample->leftOffsetZ06, sample->road.leftWidth, sample->road.leftExtension);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->road.z;
        secondPoint.x = extend_track_edge(second->leftOffsetX04, second->road.leftWidth, second->road.leftExtension);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->road.x;
        secondPoint.y = second->road.height;
        secondPoint.z = extend_track_edge(second->leftOffsetZ06, second->road.leftWidth, second->road.leftExtension);
        secondPoint.z = (secondPoint.z * offset) / 1024 + second->road.z;
    } else {
        firstPoint.x = extend_track_edge(sample->rightOffsetX00, sample->road.rightWidth, sample->road.rightExtension);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->road.x;
        firstPoint.y = sample->road.height;
        firstPoint.z = extend_track_edge(sample->rightOffsetZ02, sample->road.rightWidth, sample->road.rightExtension);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->road.z;
        secondPoint.x = extend_track_edge(second->rightOffsetX00, second->road.rightWidth, second->road.rightExtension);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->road.x;
        secondPoint.y = second->road.height;
        secondPoint.z = extend_track_edge(second->rightOffsetZ02, second->road.rightWidth, second->road.rightExtension);
        secondPoint.z = (secondPoint.z * offset) / 1024 + second->road.z;
    }

    output[0] = (secondPoint.x - firstPoint.x) * fraction / 1024 + firstPoint.x;
    output[1] = (secondPoint.y - firstPoint.y) * fraction / 1024 + firstPoint.y;
    output[2] = (secondPoint.z - firstPoint.z) * fraction / 1024 + firstPoint.z;
}

void UpdateComCarTargetPoint(RaceVehicle *vehicle) {
    Vector firstPoint;
    Vector secondPoint;
    Vector target;
    RaceVehicleControl *control;
    s32 fraction;
    s32 firstNode;
    s32 secondNode;
    TrackSample3C *firstSample;
    TrackSample3C *secondSample;
    TrackLaneOffset8 *firstOffset;
    TrackLaneOffset8 *secondOffset;
    s32 delta;

    control = &vehicle->control;
    fraction = 0x400 - vehicle->trackFraction170;
    firstNode = vehicle->trackNodeF0 - fraction / 1024;
    if (firstNode < 0) {
        firstNode += TrackNodeCount;
    }
    secondNode = firstNode - 1;
    if (secondNode < 0) {
        secondNode += TrackNodeCount;
    }
    firstSample = &TrackNodes[firstNode];
    secondSample = &TrackNodes[secondNode];
    /* ASM_BARRIER: form both sample pointers before selecting the lane state. */
    __asm__ ("");
    fraction &= 0x3FF;
    if (control->laneActive6A == 0) {
        switch (control->laneTimer38) {
        case 0:
            firstOffset = &RaceContext.laneOffsets54C[firstNode];
            firstPoint.x = firstSample->road.x + firstOffset->x;
            firstPoint.z = firstSample->road.z + firstOffset->z;
            secondOffset = &RaceContext.laneOffsets54C[secondNode];
            secondPoint.x = secondSample->road.x + secondOffset->x;
            secondPoint.z = secondSample->road.z + secondOffset->z;
            target.x = (secondPoint.x - firstPoint.x) * fraction / 1024 + firstPoint.x;
            target.z = (secondPoint.z - firstPoint.z) * fraction / 1024 + firstPoint.z;
            break;
        case 10:
            InterpolateLanePointXZ(firstSample, secondSample, control->laneStart3A, fraction, (s32 *)&target);
            break;
        }
    } else {
        delta = control->laneDelta6C >= 0 ? control->laneDelta6C : -control->laneDelta6C;
        if (delta < 0x10) {
            control->laneActive6A = 0;
            control->laneStart3A = control->laneTarget3C;
            if (control->laneMode88 == 3) {
                control->laneMode88 = 0;
            }
        } else {
            if (control->laneDelta6C > 0) {
                control->laneDelta6C -= 14;
                if (control->laneDelta6C < 0) {
                    control->laneDelta6C = 0;
                }
            } else {
                control->laneDelta6C += 14;
                if (control->laneDelta6C > 0) {
                    control->laneDelta6C = 0;
                }
            }
            control->laneStart3A = control->laneTarget3C - control->laneDelta6C;
            if (control->laneStart3A < -800) {
                control->laneStart3A = -800;
            }
            if (control->laneStart3A > 800) {
                control->laneStart3A = 800;
            }
            control->laneDelta6C = control->laneTarget3C - control->laneStart3A;
        }
        InterpolateLanePointXZ(firstSample, secondSample, control->laneStart3A, fraction, (s32 *)&target);
    }
    vehicle->control.pathDeltaX30 = target.x - vehicle->position20.words[0];
    vehicle->control.pathDeltaZ34 = target.z - vehicle->position20.words[2];
}
