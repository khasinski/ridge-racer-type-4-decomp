/* RODATA 0x800105d0 0x30 */
#include "types.h"
#include "game/car/car.h"
#include "scratchpad.h"
#include "m2c_macros.h"
#include "game/system/pad.h"
#include "game/ui/hudmap.h"
#include "game/race/racegrid.h"
#include "rr/menu_selection.h"
#include "rr/matrix_workspace.h"
#include "rr/audio_loop.h"
#include "rr/driver_control.h"
#include "rr/bios_events.h"
#include "rr/serial_handles.h"
#include "game/gfx/gfxotag.h"
#include "game/gfx/drawbuf.h"

typedef struct { int a, b, c, d; } w3g2V;

struct w3g2p {
    int pad0[4]; int f10, f14, f18, f1C; char pad1[0x20]; int f40; int pad44; int f48; int pad4c; int f50, f54, f58, f5C;
    char pad2[0x108 - 0x60]; int f108; char pad3[0x1AA - 0x10C]; short s1AA, s1AC; unsigned short u1AE; char pad4[6]; unsigned short u1B6;
    char pad5[0x1E4 - 0x1B8]; unsigned short u1E4;
};

typedef struct {
    u8 b0;
    u8 b1;
    u8 b2;
    u8 b3;
} Glyph4;

typedef struct { char pad[0x50]; short f; char pad2[0xA]; } S1;

typedef struct {
    short x, y;
} V;

typedef struct 
{
  char pad[0x84];
  int a84;
  int a88;
} T38;

extern int ScaleMatrixRow1();
extern int BuildCameraMatrices();
extern short MenuCarCameraAngles[];
extern short MenuCarCameraOffsets[];
extern struct 
{
  char pad[0x178];
  int v;
  char pad2[0x320 - 0x17C];
} PlayerCars[];
extern struct 
{
  short a;
  unsigned short b;
  char pad[0x58];
} PlayerControllers[];
extern short AngleDiffSigned();
extern int BuildRotMatrixZ();
extern int BuildRotMatrixY();
extern int BuildRotMatrixX();
extern int MulMatrix();
extern int ApplyMatrix();
extern int ViewUsesCarTrackNode;
extern s32 AddDrawAreaPrim();
extern s32 AddDrawModePrim();
extern s32 AddSpritePrim();
extern Glyph4 StartCountdownGlyphs[];
extern u16 StartCountdownLightCluts[];
extern u16 StartCountdownCluts[];
extern u8 StartCountdownDigitX[];
extern u8 StartCountdownDigitX2;
extern u8 StartCountdownDigitX3;
extern s32 RaceState;
extern int SoundQueueEnqueue();
extern int RaceVoiceFlags;
extern short MenuFadeTimer;
extern int PadSetRumbleParams();
extern int PlaySfx();
extern unsigned char PauseCursorMax[];
extern signed char PauseCursor;
extern int FinishRace();
extern int PauseMenuModeTargets[];
extern char SelectionState[];
extern int RankEntries;
extern int TrackLength;
extern short ModeId;
extern int TotalRaceTime;
extern short ModeGroup;
extern short ModeSubState;
extern int GameQueueSprite();
extern int EmitPolyF3();
extern unsigned char GridQuadTriOrders[][4];
extern int SioDeviceName[];
extern int LinkWriteHandle;
extern int LinkReadHandle;
extern s16 LinkState;
extern int LinkHandshakeCountdown;
extern signed char LinkEntryState[];
extern signed char LinkEntryStateSlot2;
extern int RaceTimeRemaining;
extern short LinkRoleSlot;
extern int MenuCancelPressed;

void SetupMenuCarCamera(int a);
void UpdateRaceChaseCamera(struct w3g2p *p);
void DrawStartCountdown(s32 arg0, s32 arg1);
void PlayStartCountdownVoice(int a0);
void InitPlayerRumble(void);
void UpdatePauseMenu(int a0);
void ApplyPauseMenuChoice(int arg0);
void DrawPauseMenu(int a0, int a1);
int LinkTimeoutCallback(int a0, unsigned int a1);
void BeginLinkSync(void);
int ExitOnLinkLost(void);
int UpdateLinkHandshake(int a0);
int PollLinkStatus(void);

void SetupMenuCarCamera(int a)
{
  int b1 = (int) MenuCarCameraOffsets;
  int o = a * 6;
  short *p = (short *) (o + b1);
  short *q;
  int i;
  /* ASM_BARRIER: pin scratchpad base to $a0 like the original */
  register int *g asm("$4");
  g = (int *) SCRATCHPAD;
  g[2] = p[0];
  g[3] = p[1];
  g[4] = p[2];
  q = (short *) (o + ((int) MenuCarCameraAngles));
  g[6] = q[0];
  g[7] = q[1];
  g[8] = q[2];
  BuildCameraMatrices(g, q);
  {
    int t;
    t = SCRATCH_ROT_MATRIX;
    /* ASM_BARRIER: forces the SCRATCH_ROT_MATRIX li to be scheduled before the li a0,2 (original delay-slot layout) */
    asm volatile("" : "=r"(t) : "0"(t));
    ScaleMatrixRow1(2, t);
  }
  ;
  if (PlayerControllers[SelectionPlayer & 1].b & 4)
  {
    PlayerCars[SelectionPlayer & 1].v += 0xC0;
  }
  else
    if (PlayerControllers[SelectionPlayer & 1].b & 8)
  {
    PlayerCars[SelectionPlayer & 1].v -= 0xC0;
  }
  i = PlayerCars[SelectionPlayer & 1].v;
  if (i > 0xF00)
  {
    PlayerCars[SelectionPlayer & 1].v = 0xF00;
  }
  else
    if (i < (-0xF00))
  {
    PlayerCars[SelectionPlayer & 1].v = -0xF00;
  }
}

void UpdateRaceChaseCamera(struct w3g2p *p) {
    struct { char a[0x20]; char b[0x20]; short v[3]; short pad; int w[3]; } s;
    int *g = (int *)SCRATCHPAD;
    int r;
    short t;
    unsigned short a1, a2;
    ViewUsesCarTrackNode = 1;
    if (p->u1E4 & 0x100) {
        BuildRotMatrixY(s.a, 0x800 - p->f54);
        BuildRotMatrixX(s.b, p->f50);
        MulMatrix(s.a, s.b);
        BuildRotMatrixZ(s.b, p->f58);
        MulMatrix2(s.a, s.b);
        s.v[0] = 0;
        s.v[1] = -0x64;
        if (p->f108 < 0x28) s.v[1] = (unsigned short)p->f108 - 0x8C;
        s.v[2] = 0;
        ApplyMatrix(s.b, s.v, s.w);
        *(w3g2V *)(SCRATCHPAD + 0x8) = *(w3g2V *)&p->f10;
        g[2] -= p->f40;
        g[4] -= p->f48;
        *(w3g2V *)(SCRATCHPAD + 0x18) = *(w3g2V *)&p->f50;
        g[6] += p->s1AC * 2;
        a1 = p->u1B6;
        a2 = p->u1AE;
        g[2] += s.w[0];
        g[3] += s.w[1];
        g[4] += s.w[2];
        g[8] += ((short)a1 / 2) + ((short)a2 / 2);
        return;
    }
    s.v[0] = AngleDiffSigned(g[6], p->f50) / 8;
    r = AngleDiffSigned(g[7], p->f54);
    t = r / 6;
    g[6] += s.v[0];
    s.v[1] = t;
    g[7] += t;
    BuildRotMatrixY(s.a, 0x800 - g[7], g[7]);
    BuildRotMatrixX(s.b, g[6]);
    MulMatrix(s.a, s.b);
    BuildRotMatrixZ(s.b, g[8]);
    MulMatrix2(s.a, s.b);
    s.v[0] = 0;
    s.v[1] = -0xFA;
    if (p->f108 < 0x28) s.v[1] = (unsigned short)p->f108 - 0x122;
    s.v[2] = 0x2D0;
    ApplyMatrix(s.b, s.v, s.w);
    *(w3g2V *)(SCRATCHPAD + 0x8) = *(w3g2V *)&p->f10;
    g[8] = p->f58 - p->s1AA;
    g[2] += s.w[0];
    g[3] += s.w[1];
    g[4] += s.w[2];
}

void DrawStartCountdown(s32 arg0, s32 arg1) {
    Glyph4 sp20;
    s32 s0;
    s32 s1;
    s32 s2;
    s32 s3;
    s32 s4;
    s32 s5;
    s32 s7;
    s32 fp;
    s32 a1;

    if (RaceState != 0 && (arg1 - 0x4B) < 0xAF) {
        s5 = arg1 % 25;
        s3 = (arg1 - 0x4B) / 25;
        if ((CurrentDrawBuf + 0x70) == arg0) {
            s4 = 0;
            if (s3 == 6) {
                s4 = -(s5 << 7) / 25;
            }
        } else {
            *(s32 *)SCRATCHPAD = AddDrawAreaPrim(arg0, *(s32 *)SCRATCHPAD, 0, 0, 0x140, 0x100);
            s4 = 0x40;
        }
        a1 = AddDrawModePrim(arg0, *(s32 *)SCRATCHPAD, 5);
        if (s3 == 1) {
            fp = ((s5 << 6) / 25) + 0x40;
        } else if (s3 >= 4) {
            fp = 0xA4;
            if (s3 >= 5) {
                fp = 0x80;
            }
        } else {
            fp = 0x80;
            if (s3 <= 0) {
                fp = 1;
            }
        }
        if (RaceType == 1) {
            if (s3 >= 4) {
                s7 = ((arg1 >> 2) & 1) + 2;
                sp20 = StartCountdownGlyphs[7];
                a1 = AddSpritePrim(arg0, a1, StartCountdownDigitX3 + 0x60, s4 + 0x39, &sp20, StartCountdownCluts[s7], 0);
            } else if (s3 > 0) {
                s2 = 0;
                for (s1 = 0; s1 < 0x10;) {
                    s32 b, c;

                    if (s1 > s5 - 7) {
                        break;
                    }
                    b = s1 & 1;
                    c = b << 1;
                    a1 = AddSpritePrim(arg0, a1, s2 + 0x78, s4 + 0x39, &StartCountdownGlyphs[2 + b], StartCountdownCluts[b * 2], 0);
                    s1++;
                    s2 += 1 << (c + 1);
                }
                s7 = 0;
                sp20 = StartCountdownGlyphs[5];
                sp20 = StartCountdownGlyphs[6];
                sp20.b0 += (s3 - 1) * 12;
                a1 = AddSpritePrim(arg0, a1, StartCountdownDigitX2 + 0x60, s4 + 0x39, &sp20, StartCountdownCluts[0], 0);
            }
        } else {
            if (s3 == 0) {
                s2 = 0;
                for (s1 = 0; s1 < 0x10;) {
                    s32 b, c;

                    if (s1 > s5 - 6) {
                        break;
                    }
                    b = s1 & 1;
                    c = b << 1;
                    a1 = AddSpritePrim(arg0, a1, s2 + 0x78, s4 + 0x39, &StartCountdownGlyphs[2 + b], StartCountdownCluts[b * 2], 0);
                    s1++;
                    s2 += 1 << (c + 1);
                }
            } else if (s3 >= 4) {
                s7 = ((arg1 >> 2) & 1) + 2;
                sp20 = StartCountdownGlyphs[5];
                sp20.b0 += 0x60;
                for (s1 = 0; s1 < 3; s1++) {
                    if (s1 == 2) {
                        sp20 = StartCountdownGlyphs[7];
                        s1 = 3;
                    }
                    a1 = AddSpritePrim(arg0, a1, StartCountdownDigitX[s1] + 0x60, s4 + 0x39, &sp20, StartCountdownCluts[s7], 0);
                }
            } else {
                s7 = 0;
                sp20 = StartCountdownGlyphs[5];
                for (s1 = 0; s1 < 3; s1++) {
                    if (s1 == 0) {
                        sp20.b0 += ((s5 * 9) / 25) * 12;
                    } else if (s1 == 1) {
                        sp20.b0 = StartCountdownGlyphs[5].b0 + s3 * 0x18;
                    } else {
                        sp20 = StartCountdownGlyphs[6];
                        sp20.b0 += (s3 - 1) * 12;
                    }
                    a1 = AddSpritePrim(arg0, a1, StartCountdownDigitX[s1] + 0x60, s4 + 0x39, &sp20, StartCountdownCluts[s7], 0);
                }
            }
        }
        if (s3 > 0) {
            s0 = s4 + 0x39;
            s2 = 0;
            for (s1 = 0; s1 < 10; s1++) {
                a1 = AddSpritePrim(arg0, a1, s2 + 0x78, s0, &StartCountdownGlyphs[4], StartCountdownCluts[(u32)(s1 - 3) < 4 ? 3 : s7], 0);
                s2 += 8;
            }
        }
        a1 = AddSpritePrim(arg0, a1, 0x68, s4 + 0x1A, &StartCountdownGlyphs[1], StartCountdownLightCluts[s3 >= 4], (fp << 1) | 1);
        a1 = AddSpritePrim(arg0, a1, 0x60, s4, &StartCountdownGlyphs[0], 0x797C, 0);
        *(s32 *)SCRATCHPAD = AddDrawModePrim(arg0, a1, 0x3F);
        if (arg0 == CurrentDrawBuf + 0xB70) {
            s32 y;
            if (s3 == 6) {
                fp = ((0x19 - s5) * 0x18) / 25;
            } else {
                fp = 0x18;
            }
            y = (0x18 - fp) / 2 + 0x34;
            *(s32 *)SCRATCHPAD = AddDrawAreaPrim(arg0, *(s32 *)SCRATCHPAD, 0x70, s4 + y, 0x60, fp);
        }
    }
}

void PlayStartCountdownVoice(int a0) {
    if (RaceState != 0 && a0 < 0x115) {
        if (a0 == 0xC3) {
            MenuFadeTimer = 0x1E;
        }
        if (a0 == 0x65) {
            if (!(RaceVoiceFlags & 1)) {
                SoundQueueEnqueue(0, 0);
                RaceVoiceFlags |= 1;
                return;
            }
        }
        if (a0 == 0x7E) {
            if (!(RaceVoiceFlags & 2)) {
                SoundQueueEnqueue(1, 0);
                RaceVoiceFlags |= 2;
                return;
            }
        }
        if (a0 == 0x97) {
            SoundQueueEnqueue(2, 0);
            return;
        }
        if (a0 == 0xB0) {
            if (!(RaceVoiceFlags & 4)) {
                SoundQueueEnqueue(3, 0);
                RaceVoiceFlags |= 4;
                return;
            }
        }
        if (a0 == 0xE2) {
            if (!(RaceVoiceFlags & 8)) {
                SoundQueueEnqueue(0x17, 0);
                RaceVoiceFlags |= 8;
                return;
            }
        }
        if (a0 == 0x114) {
            if (!(RaceVoiceFlags & 0x10)) {
                SoundQueueEnqueue(0x18, 0);
                RaceVoiceFlags |= 0x10;
            }
        }
    }
}

/* legacy typing of shared globals in this part of the unit */
#define PlayerCars (*(Car (*)[])&PlayerCars)
#define PlayerControllers (*(S1 (*)[])&PlayerControllers)

void InitPlayerRumble(void) {
    int i;
    PadClearPulse(0);
    PadClearPulse(1);
    for (i = 0; i < 2; i++) {
        if (PlayerCars[i].unk1E4 != 0) {
            PlayerControllers[i].f = 1;
            PadSetRumbleParams(i, 0, 0xF);
        }
    }
}

#undef PlayerCars
#undef PlayerControllers

void UpdatePauseMenu(int a0) {
    if (Pad0Buttons & 0x1000) {
        if (PauseCursor > 0) {
            PauseCursor = PauseCursor - 1;
            PlaySfx(0);
        }
    }
    if (Pad0Buttons & 0x4000) {
        if (PauseCursor < PauseCursorMax[a0]) {
            PauseCursor = PauseCursor + 1;
            PlaySfx(0);
        }
    }
    InitPlayerRumble();
}

void ApplyPauseMenuChoice(int arg0) {
    int *t;
    int b;
    if (PauseCursor == 0) {
        MenuFadeTimer = 0x1E;
        return;
    }
    SelectionState[0x19] = 0;
    TotalRaceTime = -1;
    if (arg0 == 1) {
        SelectionState[0x19] = 9;
        RaceState = 6;
        RankEntries = -TrackLength;
        if (PauseCursor == 2) {
            ModeSubState = 0;
            ModeGroup = PauseMenuModeTargets[3] >> 8;
            ModeId = *(unsigned char *)&PauseMenuModeTargets[3];
            return;
        }
        FinishRace(SelectionState, PauseCursor);
        return;
    }
    b = (int)PauseMenuModeTargets;
    RaceState = 6;
    t = (int *)((PauseCursor - 1) * 4 + arg0 * 8 + b);
    ModeGroup = *t >> 8;
    {
        int v = *(unsigned char *)t;
        ModeSubState = 0;
        ModeId = v;
    }
}

void DrawPauseMenu(int a0, int a1) {
    V v[4];
    unsigned char rgb[4];
    int p;
    int i, j;
    unsigned char (*base)[4];
    unsigned char *t;

    p = GameQueueSprite(a0, *(int *)SCRATCHPAD, 0x90, 0x5C, 0x1C, 8, 0x60, 0xA0, 0x7FF8);
    p = GameQueueSprite(a0, p, 0x90, 0x6A, 0x20, 8, 0xC4, 0xA0, 0x7FF8);
    if (RaceType == 1) {
        p = GameQueueSprite(a0, p, 0x90, 0x74, 0x1C, 8, 0x7C, 0xA0, 0x7FF8);
    } else {
        p = GameQueueSprite(a0, p, 0x90, 0x74, 0x24, 8, 0x9C, 0xA0, 0x7FF8);
        p = GameQueueSprite(a0, p, 0x90, 0x7E, 0x1C, 8, 0x7C, 0xA0, 0x7FF8);
    }
    p = GameQueueSprite(a0, p, 0x88, a1 * 10 + 0x6A, 8, 8, 0xE4, 0xA0, 0x7FF8);

    for (i = 0; i < 9; i++) {
        for (j = 0; j < 4; j++) {
            int kx = ((j & 1) << 5) + 0x70;
            v[j].x = ((i % 3) << 5) + kx;
            {
                int ky = ((j >> 1) << 4) + 0x56;
                v[j].y = ((i / 3) << 4) + ky;
            }
        }
        rgb[0] = 0xFF;
        rgb[2] = 0x20;
        rgb[1] = 0x20;
        j = i / 3;
        j = j == 1;
        base = GridQuadTriOrders;
        t = base[j];
        p = EmitPolyF3(a0, p, v[t[0]], v[t[1]], v[t[2]], rgb, 1);
        rgb[0] = 0xFF;
        rgb[2] = 0xFF;
        rgb[1] = 0xFF;
        p = EmitPolyF3(a0, p, v[t[1]], v[t[2]], v[t[3]], rgb, 1);
    }
    *(int *)SCRATCHPAD = AddDrawModePrim(a0, p, 5);

    for (j = 0; j < 4; j++) {
        if (j == 1 || j == 2) {
            v[j].x = 0xD0;
        } else {
            v[j].x = 0x70;
        }
        if (j >= 2) {
            v[j].y = 0x86;
        } else {
            v[j].y = 0x56;
        }
    }

    for (i = 0; i < 3; i++) {
        j = 0x40;
        AddPartialLine(a0, v[i].x, v[i].y, v[i + 1].x, v[i + 1].y, 0x100, 0, j, j, j, -1);
    }
}

void EnableLinkSioTx(void) {
    _sio_control(1, 1, 1);
}

int IsLinkPeerPresent(void) {
    return (unsigned)(((unsigned)_sio_control(0, 0, 0) >> 7) & 3) - 1 < 2;
}

int LinkTimeoutCallback(int a0, unsigned int a1) {
    return a1 <= 0x1869E;
}

void OpenLinkSerial(void) {
    _sio_control(1, 3, 0x7E900);
    _sio_control(1, 4, 1);
    LinkWriteHandle = FileOpen(SioDeviceName, 2);
    LinkReadHandle = FileOpen(SioDeviceName, 1);
    _sio_control(4, 0, (int)LinkTimeoutCallback);
    _sio_control(2, 0, 0);
}

void CloseLinkSerial(void) {
    _sio_control(2, 3, 0);
    _sio_control(2, 0, 0);
    FileClose(LinkWriteHandle);
    FileClose(LinkReadHandle);
    _sio_control(4, 0, 0);
}

void BeginLinkSync(void) {
    if (LinkState == 2) {
        LinkState = 3;
        _sio_control(1, 1, 2);
    }
}

int ExitOnLinkLost(void) {
    if (LinkState == -1) {
        ModeGroup = 1;
        ModeId = 0x2D;
        ModeSubState = 0;
        _sio_control(1, 1, 0);
        return 1;
    }
    return 0;
}

/* legacy typing of shared globals in this part of the unit */
#define _sio_control ((unsigned int (*)())_sio_control)
#define PlayerControllers (*(T38 (*))&PlayerControllers)
#define LinkState (*(unsigned short (*))&LinkState)

int UpdateLinkHandshake(int a0)
{
  int r = 0;
  long new_var2;
  int st;
  int mm;
  signed char *t;
  signed char *bp;
  short z;
  int xx;
  int q3;
  T38 *e;
  signed char *new_var;
  unsigned int k = (_sio_control(0, 0, 0) >> 7) & 3;
  short new_var3;
  st = (short) (LinkState + 1);
  switch (st)
  {
    case 1:
      if (_sio_control(3, 1, 0) != 0)
    {
      _sio_control(1, 1, 2);
      if (a0 == 0)
      {
        LinkRoleSlot = 0;
        LinkEntryStateSlot2 = 1;
        LinkHandshakeCountdown = 3;
        LinkState = 10;
      }
      else
      {
        goto s2;
      }
      break;
    }
      _sio_control(1, 1, 2);
      if (a0 == 0)
    {
      LinkRoleSlot = 1;
      LinkEntryState[0] = 1;
      LinkState = 1;
    }
    else
      if (k != 1)
    {
      LinkState = 1;
    }
    else
    {
      goto s2;
    }
      break;

    case 11:
      if (LinkHandshakeCountdown > 0)
    {
      _sio_control(1, 1, 2);
      LinkHandshakeCountdown -= 1;
      if (k == 1)
      {
        LinkState = 2;
        LinkHandshakeCountdown = 0;
      }
    }
    else
    {
      goto s2;
    }
      break;
      s2:
    LinkState = 2;

      break;

    case 2:
      if ((k == 0) || ((k == 3) && (a0 == 0)))
    {
      goto m1;
    }
    else
      if (k == 2)
    {
      LinkState = k;
      LinkHandshakeCountdown = 4;
    }
      break;

    case 3:
      _sio_control(1, 1, 1);
      if (LinkHandshakeCountdown > 0)
    {
      if (a0 == 0)
      {
        LinkHandshakeCountdown -= 1;
        break;
      }
    }
    else
      if (a0 == 0)
    {
      new_var = LinkEntryState;
      xx = LinkRoleSlot;
      z = 3;
      new_var2 = xx * 2;
      bp = new_var;
      t = &bp[z - new_var2];
      if ((*t) == 0)
      {
        e = &PlayerControllers;
        new_var3 = z;
        if (e->a84 != 0)
        {
          PlaySfx(1);
          bp[new_var3 - (LinkRoleSlot * 2)] = 1;
        }
        else
          if (e->a88 != 0)
        {
          *t = -1;
        }
        if (MenuCancelPressed != 0)
        {
          LinkEntryState[1 - (q3 = LinkRoleSlot)] = -1;
          _sio_control(1, 1, 0, q3);
        }
      }
    }
      if ((k == 0) || ((k == 3) && (a0 == 0)))
    {
      goto m1;
    }
    else
      if (a0 == 0)
    {
      int v = RaceTimeRemaining - 1;
      RaceTimeRemaining = v;
      if (v <= 0)
      {
        LinkState = 4;
        _sio_control(1, 1, 2);
      }
      else
        if (LinkEntryState[3 - (LinkRoleSlot * 2)] != 0)
      {
        LinkState = 3;
        _sio_control(1, 1, 2);
      }
    }
      break;

    case 4:
      if (a0 == 0)
    {
      if (RaceTimeRemaining > 0)
      {
        RaceTimeRemaining -= 1;
      }
    }
      if ((k == 0) || ((k == 3) && (a0 == 0)))
    {
      goto m1;
    }
    else
      if (k != 1)
    {
      goto c5;
    }
      break;

    case 5:
      c5:
    if (a0 != 2)
    {
      LinkState = 0;
    }

      r = 1;
      break;

    case 0:
      r = -1;
      break;
      m1:
    *((short *) (&LinkState)) = -1;

      break;

  }

  return r;
}

#undef _sio_control
#undef PlayerControllers
#undef LinkState

/* legacy typing of shared globals in this part of the unit */
#define _sio_control ((unsigned (*)())_sio_control)

int PollLinkStatus(void) {
    int r = 0;
    int t = (_sio_control(0, 0, 0) >> 7) & 3;
    if (t == 0 || t == 3) {
        LinkState = -1;
        r = -1;
    } else if (t == 1) {
        LinkState = t;
    } else if (t == 2) {
        if (LinkState == 3) r = 1;
    }
    return r;
}

#undef _sio_control
