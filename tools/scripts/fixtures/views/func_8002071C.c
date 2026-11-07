#include "rr/sprite_emit.h"

/* legacy typing of shared globals in this part of the unit */
#define AddDrawModePrim (*(u8 *(*)(u32 *,u8 *,u16))&AddDrawModePrim)

#undef AddDrawModePrim

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)(u32 *, u32 *))AddPrim)

#undef AddPrim

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((int (*)())AddPrim)

#define SetShadeTex ((int (*)())SetShadeTex)

#undef AddPrim

#undef SetShadeTex

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((int (*)())AddPrim)

#define SetShadeTex ((int (*)())SetShadeTex)

#undef AddPrim

#undef SetShadeTex

/* legacy typing of shared globals in this part of the unit */
#define AddPrim ((void (*)(u32 *,u32 *))AddPrim)

#undef AddPrim

extern void SetSprt(void *),SetSemiTrans(void *,s32),AddPrim(u32 *,u32 *);

/* extern */
void SetSemiTrans(void *, s32);

void SetPolyF3(void *);

RrPolyF3Packet *EmitPolyF3(u32 *ordering,RrPolyF3Packet *packet,s32 xy0,s32 xy1,s32 xy2,const u8 *rgb,s32 flags){
 SetPolyF3(packet);
 SetSemiTrans(packet,flags);
 packet->xy0=xy0;
 packet->xy1=xy1;
 packet->xy2=xy2;
 packet->r=rgb[0];
 packet->g=rgb[1];
 packet->b=rgb[2];
 AddPrim(ordering,(u32*)packet++);
 return packet;
}
