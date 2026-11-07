#include "rr/overlay662_image_scroll.h"

void garage_shift_image_wrap(u32 *source,u32 *destination,s32 x,s32 y)
{
    u32 buffer[512];
    struct {s16 column,row;} cursor;
    s32 i,j,shift,left;
    u32 first,second;
    {
        register u32 *input asm("t1")=source;
        register u32 *temporary asm("t0")=buffer;
        if(y>=0)cursor.row=y*8;
        else cursor.row=(y+64)*8;
        for(i=0;i<64;i++) {
            if(cursor.row>=505)cursor.row=0;
            for(j=0;j<8;) {
                u32 bytes=j++*4;
                s32 at=cursor.row;
                (temporary+at)[bytes/4]=*input++;
            }
            cursor.row+=8;
        }
    }
    {
        register u32 *temporary asm("t1")=buffer;
        register u32 *output asm("t0")=destination;
        x=-x;
        if(x<0)x+=64;
        shift=(x%8)*4;
        left=32-shift;
        for(i=0;i<64;i++) {
            cursor.column=x/8;
            first=(temporary+cursor.column)[i*8];
            for(j=0;j<8;j++) {
                cursor.column++;
                if(cursor.column>=8)cursor.column=0;
                second=first;
                first=(temporary+cursor.column)[i*8];
                if(left==32)*output=second;
                else *output=(second>>shift)|(first<<left);
                output++;
            }
        }
    }
}
