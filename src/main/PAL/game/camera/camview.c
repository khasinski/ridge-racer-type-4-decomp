#include "types.h"
#include "scratchpad.h"
#include "rr/geometry.h"
#include "game/gfx/gfxdraw.h"
#include "game/gfx/meshquad.h"
#include "game/gfx/scene1.h"
#include "game/camera/camview.h"
#include "rr/matrix_workspace.h"
#include "game/gfx/drawbuf.h"

struct E { s16 a, h; };

extern struct { char pad[0x24C]; char *p24c; char pad2[0x560 - 0x250]; int v560; } RaceContext;
extern int PlayerCars;
extern int ModelBankFormat;
extern void DrawPreviewCar();
extern void PrepareCarDrawPose();
extern void SetupObjectLightMatrices();
extern void SubmitModel();
extern void BuildRotMatrixY();
extern void BuildRotMatrixX();
extern void SetGteObjectTransform();
extern void SelectModelBank();
extern short CamViewEaseCurve[];

int FindCurveSegment(struct E *p, int a1);
s32 InterpCurveLinear(RrPairS16 *t, s32 idx, s32 x);

void DrawCarPreviewScene(void)
{
    struct { char m18[0x20]; char m38[0x20]; char m58[0x28]; char v80[0x10]; char c90[4]; char pad[24]; } L;
    int *g = (int *)SCRATCHPAD;
    int s6;
    int a1;
    int *a3;
    register char *a0 __asm__("$4");
    register unsigned m __asm__("$6");
    unsigned hm;
    int d;
    int t;

    s6 = g[0x18];
    g[0x18] = 3;
    SetupObjectLightMatrices(RaceContext.p24c, RaceContext.p24c + 0x78, L.m18, L.m38);
    SelectModelBank(8);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), RaceContext.p24c, L.m18);
    g[0x1E] = 0;
    SubmitModel(SCRATCHPAD, ModelBankFormat < 1, 0, 0);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), RaceContext.p24c, L.m18);
    DrawMeshPrimList(RaceContext.v560, 0, 0);
    TransformPoint(L.m38, RaceContext.p24c, RaceContext.p24c + 0x18, L.v80, L.m58);
    BuildRotMatrixY(L.m58, -*(short *)(RaceContext.p24c + 0x2A));
    MulMatrix2(L.m18, L.m58);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), L.v80, L.m58);
    g[0x1E] = 0;
    SubmitModel(SCRATCHPAD, 1, 0, 0);
    TransformPoint(L.m38, RaceContext.p24c, RaceContext.p24c + 0x30, L.v80, L.m58);
    BuildRotMatrixY(L.m58, -*(short *)(RaceContext.p24c + 0x42));
    MulMatrix2(L.m18, L.m58);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), L.v80, L.m58);
    a1 = 1;
    g[0x1E] = 0;
    if (ModelBankFormat >= 3)
        a1 = 2;
    SubmitModel(SCRATCHPAD, a1, 0, 0);
    TransformPoint(L.m38, RaceContext.p24c, RaceContext.p24c + 0x60, L.v80, L.m58);
    BuildRotMatrixX(L.m58, -*(short *)(RaceContext.p24c + 0x70));
    MulMatrix2(L.m18, L.m58);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), L.v80, L.m58);
    DrawMeshPrimList(RaceContext.v560, 2, 0);
    TransformPoint(L.m38, RaceContext.p24c, RaceContext.p24c + 0x48, L.v80, L.m58);
    BuildRotMatrixY(L.m58, -*(short *)(RaceContext.p24c + 0x5A));
    MulMatrix2(L.m18, L.m58);
    SetGteObjectTransform((SCRATCHPAD + 0x7C), L.v80, L.m58);
    DrawMeshPrimList(RaceContext.v560, *(int *)(RaceContext.p24c + 0xA4) * 2 | 1, 0);
    g[0x18] = s6;
    L.c90[0] = 0x80;
    L.c90[1] = 0x80;
    L.c90[2] = 0x80;
    g[1] = CurrentDrawBuf + 0xB70;
    PrepareCarDrawPose(&PlayerCars);
    DrawPreviewCar(&PlayerCars, *(int *)L.c90, 0x80, 1);
    t = RotTransPersOrigin(L.m18, (char *)&PlayerCars + 0x20);
    t = (t >> 3) + 0x64;
    m = 0xFFFFFF;
    t = t << 2;
    a0 = (char *)t;
    hm = 0xFF000000;
    d = CurrentDrawBuf;
    a3 = (int *)g[1];
    a0 = a0 + d;
    *a3 = (*a3 & hm) | (*(unsigned *)(a0 + 0x70) & m);
    *(unsigned *)(a0 + 0x70) = (*(unsigned *)(a0 + 0x70) & hm) | ((g[1] + 0xAFC) & m);
    g[1] = d + 0x70;
}

int FindCurveSegment(struct E *p, int a1) {
    int t = a1 / 10;
    int i = 0;
    while (p[i].h >= 0) {
        if (t < p[i].h) break;
        i++;
    }
    return i;
}

s32 InterpCurveLinear(RrPairS16 *t, s32 idx, s32 x) {
    RrPairS16 *p;
    if (idx == 0) return t[0].a;
    p = (RrPairS16 *)(idx * 4 + (s32)t);
    if (p->b < 0) return p->a;
    return (p->a - p[-1].a) * (x - p[-1].b * 10) / ((p->b - p[-1].b) * 10) + p[-1].a;
}

int InterpCurveEase(int a0)
{
  int pad[2];
  int i;
  int r;
  int q;
  short *p;
  int v1;
  int a1;
  int a2;
  int a3;
  int d;
  int h;
  i = FindCurveSegment(CamViewEaseCurve, a0);
  if (i == 0)
  {
    return CamViewEaseCurve[0];
  }
  p = (short *) ((i * 4) + ((int) CamViewEaseCurve));
  if (p[1] < 0)
  {
    return p[0];
  }
  v1 = p[-1];
  a3 = p[0];
  a1 = p[1] - v1;
  a1 = a1 * 10;
  d = a0 - (v1 * 10);
  a2 = a3 - p[-2];
  h = a1 >> 1;
  if (d < h)
  {
    r = (((d * d) * a2) / 2) / (h * h);
  }
  else
  {
    d = a1 - d;
    q = (((d * d) * a2) / 2) / (h * h);
    r = a3 - q;
  }
  q = r;
  return q;
}
