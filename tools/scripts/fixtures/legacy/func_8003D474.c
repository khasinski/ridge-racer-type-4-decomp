#include "rr/ui_panel.h"
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
        RrGpuScratchPointers *scratch=(RrGpuScratchPointers *)0x1F800000U;
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
