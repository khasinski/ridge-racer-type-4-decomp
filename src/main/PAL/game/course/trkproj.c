#include "types.h"
#include "game/course/control.h"
#include "rr/track_projection.h"
#include "rr/geometry.h"
#include "rr/trig.h"
#include "rr/race_front_search.h"
#include "rr/track_interpolation.h"
#include "rr/race.h"
#include "rr/track_lateral.h"

/* Keep the original table-relative cursors, while using the canonical native
 * record stride and array offset (the pointer-bearing row grows on a host). */
#define RR_STANDING_PREFIX_SIZE \
    (sizeof(RrRaceStandingsTable)-sizeof(((RrRaceStandingsTable*)0)->entries18))
#define RR_STANDING_ROW(base) \
    ((RrRaceStandingEntry0C*)((base)+RR_STANDING_PREFIX_SIZE))
#define SIN(x) (((x) & 0x800) ? (((x) & 0x400) ? -SinTable[0x400 - ((x) & 0x3FF)] : -SinTable[(x) & 0x3FF]) : (((x) & 0x400) ? SinTable[0x400 - ((x) & 0x3FF)] : SinTable[(x) & 0x3FF]))

extern RrRaceStandingsTable *RaceStandings;
extern s8 D_80010F2C[][16];
extern s8 D_80010F8C[][16];
extern s16 RaceType;
extern s32 ScanCurveAhead();
extern int NumPlayers;

void UpdateComCarCurrentLane(RrRaceVehicle *vehicle);
void ProjectToTrackSegment(RrTrackOffsetArgument offsetX, RrTrackOffsetArgument offsetZ, s32 node, s16 *fraction, s16 *lateral, s32 *remaining);
void UpdateCarTrackProjection(RrRaceVehicle *vehicle);
s32 FindNearestCarAhead(RrRaceVehicle *vehicle);
s32 ScoreOvertakeOpportunity(RrRaceVehicle *arg0, s32 arg1);
void UpdateComCarOvertake(RrRaceVehicle *p);
void InterpolateLanePointXZ(const RrTrackSample3C *first, const RrTrackSample3C *second, s32 lateral, s32 fraction, s32 *output);
void InterpolateLanePointXYZ(const RrTrackSample3C *first, const RrTrackSample3C *second, s32 lateral, s32 fraction, s32 *output);
void UpdateComCarTargetPoint(RrRaceVehicle *vehicle);

void UpdateComCarCurrentLane(RrRaceVehicle *vehicle)
{
    struct {
        RrTrackOffsetArgument a;
        RrTrackOffsetArgument b;
        RrVector first;
        RrVector second;
        s16 fraction;
        int remaining;
    } line;
    int distance = 0x400 - vehicle->trackFraction170;
    int rounded = distance;
    int firstNode;
    int secondNode;
    int fraction;
    RrTrackLaneOffset8 *offset;
    RrTrackLaneOffset8 *sample;
    RrRaceVehicleControl *control = (RrRaceVehicleControl *)((u8 *)vehicle + RR_RACE_CONTROL_OFFSET);
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
    offset = TrackLaneOffsets + firstNode;
    sample = offset;
    line.first.x = sample->x;
    line.first.z = sample->z;
    sample = TrackLaneOffsets + secondNode;
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
    ProjectToTrackSegment(line.a, line.b, vehicle->trackNodeF0, &line.fraction, &vehicle->laneCurrent232, &line.remaining);
    if (control->laneTimer38 == 0) {
        control->laneTarget3C = control->laneCurrent3E;
    }
}

#define SIN(x) (((x) & 0x800) ? (((x) & 0x400) ? -SinTable[0x400 - ((x) & 0x3FF)] : -SinTable[(x) & 0x3FF]) : (((x) & 0x400) ? SinTable[0x400 - ((x) & 0x3FF)] : SinTable[(x) & 0x3FF]))

void ProjectToTrackSegment(RrTrackOffsetArgument offsetX, RrTrackOffsetArgument offsetZ, s32 node, s16 *fraction, s16 *lateral, s32 *remaining) {
    s32 nextNode;
    RrTrackSample3C *sample;
    RrTrackSample3C *nextSample;
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

    angle = sample->heading12 + 0x800;
    node = sample->heading12 + 0x400;

    across = SIN(angle);
    cosine = SIN(node);

    along = (across * offsetX.value + cosine * offsetZ.value) / 4096;
    across = (-cosine * offsetX.value + across * offsetZ.value) / 4096;

    if (along < 0) {
        along = 0;
    }

    node = nextSample->heading12 - sample->heading12;
    node = sample->segmentLength1C - ((across * SIN(node)) / 4096);
    if (along > node) {
        along = node;
    }

    position = (along << 10) / node;
    *fraction = position;
    *remaining = node - ((node * (s16)position) >> 10);

    if (across < 0) {
        width = sample->leftWidth18 + sample->leftExtension2E;
        nextWidth = nextSample->leftWidth18 + nextSample->leftExtension2E;
        node = width - nextWidth;
        if (node == 0) {
            *lateral = (across << 10) / width;
        } else {
            node = ((node * *fraction) / 1024) + nextWidth;
            *lateral = (across << 10) / node;
        }
    } else {
        width = sample->rightWidth1A + sample->rightExtension2C;
        nextWidth = nextSample->rightWidth1A + nextSample->rightExtension2C;
        node = width - nextWidth;
        if (node == 0) {
            *lateral = (across << 10) / width;
        } else {
            node = ((node * *fraction) / 1024) + nextWidth;
            *lateral = (across << 10) / node;
        }
    }
}

void UpdateCarTrackProjection(RrRaceVehicle *vehicle)
{
    struct { RrTrackOffsetArgument x, z; } offset;
    offset.x.value = vehicle->pos20[0] - TrackNodes[vehicle->trackNodeF0].x08;
    offset.z.value = vehicle->pos20[2] - TrackNodes[vehicle->trackNodeF0].z0C;
    ProjectToTrackSegment(offset.x, offset.z, vehicle->trackNodeF0,
        &vehicle->trackFraction170, &vehicle->lateralOffset172, &vehicle->second);
}

s32 FindNearestCarAhead(RrRaceVehicle *vehicle) {
    s32 best=-1,index=0,selected,limit,count,fresh_count,id;
    s32 first_count,second_count,distance,half,negative_half;
    /* ASM_BARRIER: keep the captured track length in the consumed mode register. */
    register s32 circumference __asm__("$5");
    RrRaceStandingsTable *table;
    /* ASM_BARRIER: retain the second pass's table-relative cursor. */
    register u8 *cursor __asm__("$9");
    u8 *reference;
    RrRaceDistanceContextPrefix *context;
    s32 flags;
    /* ASM_BARRIER: retain context mode until the initial identifier is formed. */
    register s32 mode __asm__("$5");
    s32 input;
    u8 *lookup_cursor;
    context=&RaceContext;
    flags=vehicle->flags1E4;
    mode=context->mode;
    input=vehicle->input;
    table=(RrRaceStandingsTable*)context->result;
    flags=(flags&15)+best;
    input++;
    limit=input*16;
    count=NumCars;
    selected=(s32)((u32)flags+(u32)mode);
    if(count>0) {
        first_count=count;
        lookup_cursor=(u8*)table;
lookup:
        if(selected==RR_STANDING_ROW(lookup_cursor)->unk00)selected=index;
        else {
            index++;
            lookup_cursor+=sizeof(RrRaceStandingEntry0C);
            if(index<first_count)goto lookup;
        }
    }
    /* ASM_BARRIER: preserve the original second count capture after lookup. */
    __asm__("" : : : "memory");
    index=0;
    fresh_count=NumCars;
    if(fresh_count>0) {
        reference=(u8*)table+selected*sizeof(RrRaceStandingEntry0C);
        second_count=fresh_count;
        circumference=TrackLength;
        cursor=(u8*)table;
        half=circumference/2;
        negative_half=(s32)(0U-(u32)circumference)/2;
        do {
            if(index!=selected) {
                id=RR_STANDING_ROW(cursor)->unk00;
                distance=table->entries18[id].totalDist04;
                distance=(s32)((u32)distance-(u32)RR_STANDING_ROW(reference)->totalDist04);
                distance%=circumference;
                if(distance>=half)distance=(s32)((u32)distance-(u32)circumference);
                if(distance<negative_half)distance=(s32)((u32)distance+(u32)circumference);
                if(distance>0&&distance<limit) {
                    best=id;
                    limit=distance;
                    table->gap14=distance;
                }
            }
            index++;
            cursor+=sizeof(RrRaceStandingEntry0C);
        }while(index<second_count);
    }
    return best;
}

/* legacy typing of shared globals in this part of the unit */
#define CalcSteerLimit ((s32 (*)())CalcSteerLimit)

s32 ScoreOvertakeOpportunity(RrRaceVehicle *arg0, s32 arg1) {
    s32 sp18;
    s32 sp1C;
    s32 s0;
    Control *s1;
    RrRaceStandingsTable *t;
    RrRaceVehicle *other;
    s32 i;
    s32 v1;
    s32 a2;
    s32 a1;
    s16 temp_v0;
    s32 v1_2;

    s0 = 0;
    t = RaceStandings;
    s1 = (Control *)((char *)arg0 + 0x1F4);
    other = t->entries18[arg1].vehicle08;

    i = -other->heading1D4 + 0x400;
    i -= TrackNodes[other->trackNodeF0].heading12;
    v1 = -((SIN(i) * other->input) / 4096);
    a1 = t->gap14;
    a2 = s1->unk58 - v1;
    a1 /= 100;
    if (a1 >= 16) {
        a1 = 15;
    }

    for (i = 0; i < 2; i++) {
        s0 += D_80010F2C[i][a1];
    }

    a2 = (a2 * 160) / 1168;
    if (a2 > 0) {
        if (a2 >= 80) {
            a2 = 79;
        }
        a2 /= 5;
        if (RaceType != 5) {
            s0 += D_80010F2C[2][a2];
        } else {
            s0 += D_80010F8C[2][a2];
        }
    } else {
        s0 -= 8;
    }

    s1->unk70 = ScanCurveAhead(arg0, 0, &s1->unk80, &s1->unk74, &s1->unk78);
    s1->unk7C = ScanCurveAhead(arg0, 10, &s1->unk84, &sp18, &sp1C);
    temp_v0 = CalcSteerLimit(arg0);
    s1->unk6E = temp_v0;
    v1_2 = temp_v0 << 12;

    if (s0 > 0) {
        if (v1_2 < s1->unk7C) {
            s0 = 0;
        }
    }
    return s0;
}

#undef CalcSteerLimit

void UpdateComCarOvertake(RrRaceVehicle *p)
{
  RrRaceVehicleControl *q;
  RrRaceStandingsTable *s5;
  RrRaceVehicle *s1;
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
  int e2;
  int e3;
  int s2;
  int s4;
  int s6;
  int a0;
  int v0;
  int v1;
  int k;
  int tt;
  s6 = 0;
  s4 = -1;
  s5 = RaceStandings;
  q = (RrRaceVehicleControl *)((char *)p + 0x1F4);
  s2 = FindNearestCarAhead(p);
  if (s2 >= 0)
  {
    k = s2 * 12;
    s1 = ((RrRaceStandingsTable *)((char *)s5 + k))->entries18[0].vehicle08;
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
          e1 = s1->lateralOffset172;
          if (e1 < p->lateralOffset172)
          {
            e2 = (e1 + 0x400) / 2;
          }
          else
          {
            tt = e1 - 0x400; tt = tt + ((unsigned)tt >> 31);
            __asm__ volatile("" : "=r"(tt) : "0"(tt)); /* ASM_BARRIER: keep the two /2 arms from being cross-jumped */
            e2 = tt >> 1;
          }
        }
        else
        {
          e2 = s1->lateralOffset172;
          e1 = (u16)s1->lateralOffset172;
          if (e2 > 0)
          {
            e2 = e1 - 0x200;
          }
          else
          {
            e2 = e1 + 0x200;
          }
        }
        *(volatile s16 *)&q->laneTarget3C = e2;
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
    x8 = s5->dist08;
    if ((x8 < 0) && (q->metric12A < x8))
    {
      s4 = 0;
    }
    if (NumPlayers == 2)
    {
      if ((s5->dist08 < s5->dist0C) && (q->metric12A < s5->dist0C))
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
        UpdateComCarCurrentLane(p);
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
void InterpolateLanePointXZ(const RrTrackSample3C *first, const RrTrackSample3C *second,
    s32 lateral, s32 fraction, s32 *output)
{
    RrVector firstPoint;
    RrVector secondPoint;
    /* ASM_BARRIER: retain the lateral magnitude in t1. */
    register s32 offset __asm__("$9");
    /* ASM_BARRIER: retain the shared Z edge component in v1. */
    register s32 edgeZ __asm__("$3");
    const RrTrackSample3C *sample;

    offset = lateral;
    if (lateral < 0) offset = -offset;
    sample = first;
    if (lateral < 0) {
        firstPoint.x = extend_track_edge(sample->leftOffsetX04, sample->leftWidth18, sample->leftExtension2E);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->x08;
        firstPoint.z = extend_track_edge(sample->leftOffsetZ06, sample->leftWidth18, sample->leftExtension2E);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->z0C;
        secondPoint.x = extend_track_edge(second->leftOffsetX04, second->leftWidth18, second->leftExtension2E);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->x08;
        edgeZ = extend_track_edge(second->leftOffsetZ06, second->leftWidth18, second->leftExtension2E);
    } else {
        /* ASM_BARRIER: retain a separate base for the first sample. */
        __asm__ volatile ("" : "=r"(sample) : "0"(sample));
        firstPoint.x = extend_track_edge(sample->rightOffsetX00, sample->rightWidth1A, sample->rightExtension2C);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->x08;
        firstPoint.z = extend_track_edge(sample->rightOffsetZ02, sample->rightWidth1A, sample->rightExtension2C);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->z0C;
        secondPoint.x = extend_track_edge(second->rightOffsetX00, second->rightWidth1A, second->rightExtension2C);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->x08;
        edgeZ = extend_track_edge(second->rightOffsetZ02, second->rightWidth1A, second->rightExtension2C);
    }

    secondPoint.z = edgeZ;
    secondPoint.z = (secondPoint.z * offset) / 1024 + second->z0C;
    /* ASM_BARRIER: keep final firstPoint/secondPoint reloads after the secondPoint.z store */
    __asm__ volatile ("");
    output[0] = (secondPoint.x - firstPoint.x) * fraction / 1024 + firstPoint.x;
    output[2] = (secondPoint.z - firstPoint.z) * fraction / 1024 + firstPoint.z;
}

/* The camera path also needs the height of the centre line. */
void InterpolateLanePointXYZ(const RrTrackSample3C *first, const RrTrackSample3C *second,
    s32 lateral, s32 fraction, s32 *output)
{
    RrVector firstPoint;
    RrVector secondPoint;
    /* ASM_BARRIER: retain the lateral magnitude in t1. */
    register s32 offset __asm__("$9");
    /* ASM_BARRIER: retain the shared Z edge component in v1. */
    register s32 edgeZ __asm__("$3");
    const RrTrackSample3C *sample;

    offset = lateral;
    if (lateral < 0) offset = -offset;
    sample = first;
    if (lateral < 0) {
        firstPoint.x = extend_track_edge(sample->leftOffsetX04, sample->leftWidth18, sample->leftExtension2E);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->x08;
        firstPoint.y = sample->y10;
        firstPoint.z = extend_track_edge(sample->leftOffsetZ06, sample->leftWidth18, sample->leftExtension2E);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->z0C;
        secondPoint.x = extend_track_edge(second->leftOffsetX04, second->leftWidth18, second->leftExtension2E);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->x08;
        secondPoint.y = second->y10;
        edgeZ = extend_track_edge(second->leftOffsetZ06, second->leftWidth18, second->leftExtension2E);
    } else {
        /* ASM_BARRIER: retain a separate base for the first sample. */
        __asm__ volatile ("" : "=r"(sample) : "0"(sample));
        firstPoint.x = extend_track_edge(sample->rightOffsetX00, sample->rightWidth1A, sample->rightExtension2C);
        firstPoint.x = (firstPoint.x * offset) / 1024 + sample->x08;
        firstPoint.y = sample->y10;
        firstPoint.z = extend_track_edge(sample->rightOffsetZ02, sample->rightWidth1A, sample->rightExtension2C);
        firstPoint.z = (firstPoint.z * offset) / 1024 + sample->z0C;
        secondPoint.x = extend_track_edge(second->rightOffsetX00, second->rightWidth1A, second->rightExtension2C);
        secondPoint.x = (secondPoint.x * offset) / 1024 + second->x08;
        secondPoint.y = second->y10;
        edgeZ = extend_track_edge(second->rightOffsetZ02, second->rightWidth1A, second->rightExtension2C);
    }

    secondPoint.z = edgeZ;
    secondPoint.z = (secondPoint.z * offset) / 1024 + second->z0C;
    /* ASM_BARRIER: keep final firstPoint/secondPoint reloads after the secondPoint.z store */
    __asm__ volatile ("");
    output[0] = (secondPoint.x - firstPoint.x) * fraction / 1024 + firstPoint.x;
    output[1] = (secondPoint.y - firstPoint.y) * fraction / 1024 + firstPoint.y;
    output[2] = (secondPoint.z - firstPoint.z) * fraction / 1024 + firstPoint.z;
}

void UpdateComCarTargetPoint(RrRaceVehicle *vehicle) {
    RrVector firstPoint;
    RrVector secondPoint;
    RrVector target;
    RrRaceVehicleControl *control;
    s32 fraction;
    s32 firstNode;
    s32 secondNode;
    RrTrackSample3C *firstSample;
    RrTrackSample3C *secondSample;
    RrTrackLaneOffset8 *firstOffset;
    RrTrackLaneOffset8 *secondOffset;
    s32 delta;

    control = (RrRaceVehicleControl *)((u8 *)vehicle + RR_RACE_CONTROL_OFFSET);
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
    __asm__ volatile("");
    fraction &= 0x3FF;
    if (control->laneActive6A == 0) {
        switch (control->laneTimer38) {
        case 0:
            firstOffset = &TrackLaneOffsets[firstNode];
            firstPoint.x = firstSample->x08 + firstOffset->x;
            firstPoint.z = firstSample->z0C + firstOffset->z;
            secondOffset = &TrackLaneOffsets[secondNode];
            secondPoint.x = secondSample->x08 + secondOffset->x;
            secondPoint.z = secondSample->z0C + secondOffset->z;
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
    vehicle->pathDeltaX224 = target.x - vehicle->pos20[0];
    vehicle->pathDeltaZ228 = target.z - vehicle->pos20[2];
}
