"""Actual UI bar caller with full-pointer helper traces and live descriptor hooks."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class UiBarTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/legacy/func_8003D2C4.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body)
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('gpu.h') + header('ui_bar.h') + r'''
typedef struct {
    u8 prefix[32];
    RrUiBarDescriptor descriptors[8];
    RrUiUvRect glyphs[8];
    u32 order;
    RrSpritePacket pool[64];
    u8 suffix[32];
} World;
typedef struct {
    int kind;
    u32 *order;
    RrSpritePacket *cursor;
    RrUiUvRect *glyph;
    s32 args[7];
} Entry;
static World actual,expected;
static Entry actual_log[32780],expected_log[32780];
static int actual_count,expected_count,selected_style,mutation;
#define UiBarDescriptors actual.descriptors
#define UiBarGlyphRects actual.glyphs
''' + body + r'''
static s32 wrap(u32 value) {s32 result;memcpy(&result,&value,4);return result;}
static RrSpritePacket *next_cursor(int sequence) {
    return &actual.pool[(sequence*7+selected_style*3+11)%64];
}
static void effect(World *world,int sequence) {
    RrUiBarDescriptor *d=&world->descriptors[selected_style];
    if(mutation==1&&sequence==1)d->count=3;
    if(mutation==2&&sequence==2)d->count=0;
    if(mutation==3&&sequence==2)d->count=5;
    if(mutation==3&&sequence==4)d->count=3;
    if(mutation==4&&sequence==0)d->count=-1;
    if(mutation==5&&sequence==0)d->glyph=(d->glyph+1)%8;
    if(mutation==6&&sequence==1)d->count=32767;
    if(mutation==6&&sequence==2)d->count=2;
}
static RrSpritePacket *append(int model,int kind,u32 *order,RrSpritePacket *cursor,
                            RrUiUvRect *glyph,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g) {
    Entry entry;memset(&entry,0,sizeof(entry));
    entry.kind=kind;entry.order=order;entry.cursor=cursor;entry.glyph=glyph;
    entry.args[0]=a;entry.args[1]=b;entry.args[2]=c;entry.args[3]=d;
    entry.args[4]=e;entry.args[5]=f;entry.args[6]=g;
    int sequence=model?expected_count++:actual_count++;assert(sequence<32780);
    if(model)expected_log[sequence]=entry;else actual_log[sequence]=entry;
    effect(model?&expected:&actual,sequence);return next_cursor(sequence);
}
RrSpritePacket *AddSpritePrim(u32 *order,RrSpritePacket *cursor,s32 x,s32 y,
                            RrUiUvRect *glyph,u16 clut,s32 mode) {
    return append(0,0,order,cursor,glyph,x,y,clut,mode,0,0,0);
}
RrSpritePacket *GameQueueSprite(u32 *order,RrSpritePacket *cursor,s32 x,s32 y,
                            s32 width,s32 height,s32 u,s32 v,s32 clut) {
    return append(0,1,order,cursor,0,x,y,width,height,u,v,clut);
}
void *AddTilePrim(u32 *order,void *cursor,s32 x,s32 y,s32 width,s32 height,
                   s32 r,s32 g,s32 b) {
    return append(0,2,order,cursor,0,x,y,width,height,r,g,b);
}
static RrSpritePacket *reference(u32 *order,RrSpritePacket *cursor,s32 x,s32 y,int style) {
    RrUiBarDescriptor *record=&expected.descriptors[style];
    RrUiUvRect *glyph=&actual.glyphs[record->glyph];
    cursor=append(1,0,order,cursor,glyph,wrap((u32)x+6),wrap((u32)y+5),0x7FFB,0,0,0,0);
    cursor=append(1,1,order,cursor,0,x,y,4,24,20,72,0x7E3B);
    int pieces=0;
    if(record->count>0) {
        do {
            cursor=append(1,1,order,cursor,0,wrap((u32)x+4U+8U*pieces),y,8,24,24,72,0x7E3B);
            pieces++;
        }while(pieces<record->count);
    }
    cursor=append(1,1,order,cursor,0,wrap((u32)x+4U+8U*pieces),y,8,24,32,72,0x7E3B);
    return append(1,2,order,cursor,0,wrap((u32)x+4),wrap((u32)y+24),8*pieces,2,255,255,255);
}
static void check(int style,s16 count,s32 x,s32 y,int hook,u32 seed,int order_alias) {
    memset(&actual,(u8)seed,sizeof(actual));
    for(int i=0;i<8;i++) {
        actual.descriptors[i].glyph=(seed+i)%8;
        actual.descriptors[i].count=(s16)(i+1);
    }
    actual.descriptors[style].count=count;
    selected_style=style;mutation=hook;actual_count=expected_count=0;
    memcpy(&expected,&actual,sizeof(actual));
    u32 *order=order_alias?(u32*)&actual.descriptors[style]:&actual.order;
    RrSpritePacket *cursor=&actual.pool[seed%64];
    assert((uintptr_t)cursor>0xFFFFFFFFU);
    RrSpritePacket *want=reference(order,cursor,x,y,style);
    RrSpritePacket *got=DrawBadgeTitleBar(order,cursor,x,y,style);
    assert(got==want);
    assert(actual_count==expected_count);
    assert(!memcmp(actual_log,expected_log,actual_count*sizeof(Entry)));
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
    assert(argc==2);assert(sizeof(void*)>4);
    const s32 extremes[]={(-2147483647-1),-2147483647,-65536,-32769,-1,0,1,32767,65535,2147483646,2147483647};
    int mode=atoi(argv[1]);
    if(mode==0) {
        for(int style=0;style<8;style++)for(int count=-32;count<=64;count++) {
            check(style,count,17,-9,0,count+style,0);
        }
        for(int count=-32768;count<0;count++)check(count&7,count,0,0,0,(u32)count,0);
        check(0,32767,2147483647,(-2147483647-1),0,0,0);
        for(int style=0;style<8;style++)for(unsigned x=0;x<11;x++)for(unsigned y=0;y<11;y++) {
            check(style,3,extremes[x],extremes[y],0,style+x+y,0);
        }
        for(u32 i=0;i<4096;i++)check(i%8,i%17,wrap(i*2654435761U),wrap(i*1779033703U),0,i,0);
    } else {
        for(int style=0;style<8;style++)for(int hook=1;hook<=6;hook++) {
            for(int count=-2;count<=8;count++) {
                check(style,count,2147483647,(-2147483647-1),hook,style+count,hook&1);
            }
        }
    }
    return 0;
}
'''
        source=Path(cls.temporary.name)/'ui_bar.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_counts_coordinates_and_full_pointer_cursor_chain(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_live_descriptor_changes_from_helpers(self):
        subprocess.run([str(self.binary),'1'],check=True)


if __name__=='__main__':
    unittest.main()
