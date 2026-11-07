#include "scratchpad.h"
#include "rr/ui_bar.h"
#include "rr/ui_panel.h"
#include "game/ui/uidraw.h"

typedef struct 
{
  short a;
  short b;
  short c;
  short d;
  short e;
  short f;
  short g;
  short h;
  short i;
  char pad[10];
} E;

void InitBadgeRow();
RrSpritePacket *DrawBadgeTitleBar(u32 *order,RrSpritePacket *cursor,s32 x,s32 y,s32 style);

/* legacy typing of shared globals in this part of the unit */
#define BadgeRows ((E *)BadgeRows)

void InitBadgeRow(a0, a1, a2, a3, a4, a5, a6)
int a0;
int a1;
short a2;
int a3;
unsigned short a4;
unsigned short a5;
unsigned short a6;
{
  int s;
  unsigned int z;
  if (a0 < BadgeRowCount)
  {
    BadgeRows[a0].a = a6;
    BadgeRows[a0].b = 0;
    s = (short) a6;
    if (s < 0)
    {
      a6 = 0;
    }
    else
    {
      z = s == 0;
      a6 = 0x16;
      if (z)
      {
        a4 = 0x80;
      }
    }
    BadgeRows[a0].d = a2 - a6;
    BadgeRows[a0].f = a2 - a6;
    BadgeRows[a0].c = a1;
    BadgeRows[a0].e = a1;
    BadgeRows[a0].g = a3;
    BadgeRows[a0].h = a4 + a6;
    BadgeRows[a0].i = a5;
  }
}

#undef BadgeRows

/* legacy typing of shared globals in this part of the unit */
#define AddSpritePrim (*(RrSpritePacket *(*)(u32*,RrSpritePacket*,s32,s32,RrUiUvRect*,u16,s32))&AddSpritePrim)
#define GameQueueSprite (*(RrSpritePacket *(*)(u32*,RrSpritePacket*,s32,s32,s32,s32,s32,s32,s32))&GameQueueSprite)

RrSpritePacket *DrawBadgeTitleBar(u32 *order,RrSpritePacket *cursor,s32 x,s32 y,s32 style) {
    /* ASM_BARRIER: keep the ordering pointer live across every helper call. */
    register u32 *ordering __asm__("$21")=order;
    s32 xpos,ypos,offset,index;
    /* ASM_BARRIER: retain the captured descriptor across live count reloads. */
    register RrUiBarDescriptor *descriptor __asm__("$18");
    /* ASM_BARRIER: stage each horizontal coordinate in the outgoing argument. */
    register s32 x_stage __asm__("$6")=x;
    /* ASM_BARRIER: stage each vertical coordinate in the outgoing argument. */
    register s32 y_stage __asm__("$7")=y;
    u8 *descriptor_base;
    /* ASM_BARRIER: form the descriptor byte offset in the stack-argument temporary. */
    register s32 selector __asm__("$2");
    s32 width,sampled;
    u32 *out_order;
    RrSpritePacket *out_cursor;
    /* ASM_BARRIER: capture the saved ordering pointer before position setup. */
    __asm__("" : "+r"(x_stage) : "r"(ordering));
    xpos=x_stage;
    x_stage=(s32)((u32)xpos+6U);
    ypos=y_stage;
    y_stage=(s32)((u32)ypos+5U);
    offset=4;
    index=0;
    descriptor_base=(u8*)UiBarDescriptors;
    selector=(u32)style*sizeof(RrUiBarDescriptor);
    descriptor=(RrUiBarDescriptor*)(descriptor_base+selector);
    cursor=AddSpritePrim(ordering,cursor,x_stage,y_stage,
                       &UiBarGlyphRects[descriptor->glyph],0x7FFB,0);
    out_order=ordering;
    out_cursor=cursor;
    x_stage=xpos;
    y_stage=ypos;
    /* ASM_BARRIER: consume the returned cursor before reusing v0 for width. */
    __asm__("" : "+r"(out_order), "+r"(out_cursor), "+r"(x_stage), "+r"(y_stage));
    width=offset;
    cursor=GameQueueSprite(out_order,out_cursor,x_stage,y_stage,width,24,20,72,0x7E3B);
    if(descriptor->count>0) {
        do {
            cursor=GameQueueSprite(ordering,cursor,(s32)((u32)xpos+(u32)offset),
                                ypos,8,24,24,72,0x7E3B);
            sampled=descriptor->count;
            /* ASM_BARRIER: sample the updated count before incrementing the index. */
            __asm__("" : "+r"(sampled), "+r"(index));
            index++;
            offset+=8;
        }while(index<sampled);
    }
    cursor=GameQueueSprite(ordering,cursor,(s32)((u32)xpos+(u32)offset),
                        ypos,8,24,32,72,0x7E3B);
    out_order=ordering;
    out_cursor=cursor;
    x_stage=(s32)((u32)xpos+4U);
    y_stage=(s32)((u32)ypos+24U);
    /* ASM_BARRIER: finish the final helper arguments before computing underline width. */
    __asm__("" : "+r"(out_order), "+r"(out_cursor), "+r"(x_stage), "+r"(y_stage) : : "$2");
    width=offset-4;
    return AddTilePrim(out_order,out_cursor,x_stage,y_stage,width,2,255,255,255);
}

#undef AddSpritePrim
#undef GameQueueSprite

void DrawBadgeRow(s32 index) {
    /* ASM_BARRIER: retain the selected index across helper and callback calls. */
    register s32 which asm("$23")=index;
    s32 high_raw;
    s32 offset=which*sizeof(RrBadgeRow1C);
    RrBadgeRow1C *row=(RrBadgeRow1C *)((unsigned long)offset+(unsigned long)BadgeRows);
    u32 raw;
    s32 status;
    /* ASM_BARRIER: retain the byte offset for the two fresh table captures. */
    __asm__("" : "+r"(offset));
    raw=row->status00;

    high_raw=(u32)raw<<16;

    status=high_raw>>16;
    if(status!=-1) {
        s32 y;
        s32 x;
        s32 width;
        u32 *order;
        RrPanelContext *context;
        s32 zero6;
        s32 zero7;
        s32 extent;
        s32 height;
        s32 callback;
        void *cursor;
        s32 white=255;
        RrGpuScratchPointers *scratch=(RrGpuScratchPointers *)SCRATCHPAD;
        zero6=0;

        zero7=zero6;
        y=row->y06;x=row->x04;width=row->width0C;extent=row->extent0E;
        
        context=(RrPanelContext *)CurrentDrawBuf;
        
        order=&context->head74;
        height=extent-22;
        cursor=AddDrawAreaPrim(order,scratch->cursor,zero6,zero7,320,512);
        cursor=AddDrawModePrim(order,cursor,37);
        callback=high_raw>>24;
        {
            void *call_cursor=cursor;
            
            if(callback)call_cursor=PanelDrawCallbacks[callback-1](order,call_cursor,which);
            cursor=call_cursor;
        }
        cursor=DrawBadgeTitleBar(order,cursor,x,y,status&255);
        y=y+22;
        row=(RrBadgeRow1C *)((unsigned long)offset+(unsigned long)BadgeRows);
        cursor=AddTilePrim(order,cursor,x,row->y0A+height+20,width,2,white,white,white);
        cursor=AddTilePrim(order,cursor,x,y,width,2,white,white,white);
        cursor=AddTilePrim(order,cursor,x,y,1,height,white,white,white);
        cursor=AddTilePrim(order,cursor,x+width-1,y,1,height,white,white,white);
        white=74;
        cursor=AddTilePrim(order,cursor,x,y,width,height,white,white,white);
        cursor=AddDrawModePrim(order,cursor,37);
        row=(RrBadgeRow1C *)((unsigned long)offset+(unsigned long)BadgeRows);
        scratch->cursor=AddDrawAreaPrim(order,cursor,row->x14,row->y16,row->w18,row->h1A);
    }
}
