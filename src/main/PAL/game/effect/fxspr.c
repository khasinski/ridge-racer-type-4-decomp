#include "game/gfx/fontdraw.h"
#include "rr/gpu_address.h"
#include "scratchpad.h"
#include "rr/frame_storage_types.h"
#include "game/effect/fxspr.h"
#include "game/gfx/fadespr.h"
#include "rr/frame_loop.h"
#include "rr/trig.h"
#include "psyq/libc/rand.h"


extern int SparkParticleInitVel[];
extern const char HudPauseMenuItemCounts[13][16];
extern unsigned int OverlayTextSpriteUv;
#include "rr/scene_camera_state.h"
void DrawIntroRippleSprites(void)
{
  int l[2];
  short h[2];
  int i;
  int j;
  int x;
  int y;
  int ax;
  int ay;
  int w;
  unsigned char *p;

    u32 *head;
  x = (RaceContext.fade3E0.rippleIntensity60 / 4) + (rsin(FrameCounter * 128U) / 128);
  l[0] = (x - 16) / 4;
  y = (RaceContext.fade3E0.rippleIntensity60 / 2) + (rcos(FrameCounter * 128U) / 128);
  l[1] = (y - 16) / 4;
  for (i = 0; i < 2; i++)
  {
    if (l[i] <= 0)
    {
      l[i] = 1;
    }
  }

  h[0] = (PsyqRand() % 16) - 8;
  h[1] = (PsyqRand() % 6) - 3;
  p = *((unsigned char **) SCRATCHPAD);
  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < 4; j++)
    {
      ax = (j / 2) + 0x9E;
      ay = ((j % 2) << 1) + 0xE4;
      p = AddTextSprites(&CurrentDrawBuf->buffer.ordering70.words[10], p, ax - SparkParticleInitVel[RaceContext.fade3E0.ripplePreset58], h[i] + ay, HudPauseMenuItemCounts[RaceContext.fade3E0.ripplePreset58], 0x7F38, 1, (l[i] << 1) | 1);
    }

  }

  w = OverlayTextSpriteUv;
  *((unsigned char **) SCRATCHPAD) = p;
  p[3] = 1;
    head = &CurrentDrawBuf->buffer.ordering70.words[10];
  ((GpuCommandPacket *)p)->command = (w & 0x9FF) | 0xE1000200;
  *((unsigned int *) p) = ((*((unsigned int *) p)) & 0xFF000000) | (*head & 0xFFFFFF);
  *head = (*head & 0xFF000000) | (GpuDmaEncodeAddress(p) & 0xFFFFFF);
  *((unsigned char **) SCRATCHPAD) = p + 8;
}
