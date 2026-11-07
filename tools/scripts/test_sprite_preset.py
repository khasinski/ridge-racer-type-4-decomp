"""Actual sprite emitter, including live table and ordering-page aliases."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class SpritePresetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_8001BAF8.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        assert body.count('(RrGpuScratchPointers*)0x1F800000U')==1
        body=body.replace('(RrGpuScratchPointers*)0x1F800000U','&actual.scratch')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('gpu_scratch_types.h')+header('gpu.h') + header('animated_sprite_grid.h') + header('sprite_preset.h') + r'''
typedef struct {
    u8 guard[32];
    RrGpuScratchPointers scratch;
    u32 ordering[4];
    u8 pad[32];
    RrSpritePreset presets[32];
    u8 packets[64];
    u8 suffix[32];
} World;
static World actual __attribute__((aligned(65536))),expected;
#define SpritePresets actual.presets
''' + body + r'''
static u8 *locate(const void *address,size_t size) {
    uintptr_t offset=(uintptr_t)address-(uintptr_t)&actual;
    assert(offset<=sizeof(actual)-size);
    return (u8*)&expected+offset;
}
static u32 read32(const void *address) {
    u32 value;memcpy(&value,locate(address,4),4);return value;
}
static void word(void *address,u32 value) {memcpy(locate(address,4),&value,4);}
static void byte(void *address,u8 value) {*locate(address,1)=value;}
static u32 *order(void) {
    u32 *pointer;memcpy(&pointer,&expected.scratch.order,sizeof(pointer));return pointer;
}
static void reference(s32 intensity,int index) {
    if(!intensity)return;
    u8 *packet;memcpy(&packet,&expected.scratch.cursor,sizeof(packet));
    u32 value=intensity<0?0:intensity>255?255:(u32)intensity;
    word(packet+4,0x66000000U|value|(value<<8)|(value<<16));
    word(packet+8,read32(&actual.presets[index].position));
    word(packet+16,read32(&actual.presets[index].size));
    word(packet+12,read32(&actual.presets[index].uv));
    word(packet,read32((u8*)order()+4));
    byte(packet+3,4);
    word((u8*)order()+4,(u32)(uintptr_t)packet);
    byte((u8*)order()+7,0);
    packet+=20;memcpy(&expected.scratch.cursor,&packet,sizeof(packet));
}
static void initialize(u32 seed) {
    u8 *bytes=(u8*)&actual;
    for(size_t i=0;i<sizeof(actual);i++) {
        seed=seed*1664525U+1013904223U;bytes[i]=seed>>24;
    }
    actual.scratch.cursor=actual.packets;
    actual.scratch.order=actual.ordering;
}
static void check(s32 intensity,int index) {
    memcpy(&expected,&actual,sizeof(actual));
    reference(intensity,index);
    EmitPresetSprite(intensity,index);
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
    assert(argc==2);int mode=atoi(argv[1]);
    const s32 extremes[]={(-2147483647-1),-65536,-257,-256,-255,-1,0,1,254,255,256,257,65535,65536,2147483647};
    if(mode==0) {
        for(int index=0;index<32;index++) {
            for(int intensity=-512;intensity<=512;intensity++) {
                initialize((u32)intensity*17U+index);check(intensity,index);
            }
            for(unsigned i=0;i<sizeof(extremes)/sizeof(*extremes);i++) {
                initialize(i*381U+index);check(extremes[i],index);
            }
        }
        for(u32 i=0;i<65536;i++) {
            u32 bits=i*2654435761U; s32 intensity;memcpy(&intensity,&bits,4);
            initialize(bits);check(intensity,i%32);
        }
    } else if(mode==1) {
        for(int index=1;index<31;index++)for(int delta=-12;delta<=8;delta+=4) {
            for(unsigned i=0;i<sizeof(extremes)/sizeof(*extremes);i++) {
                initialize(i*137U+index);
                actual.scratch.cursor=(u8*)&actual+offsetof(World,presets)+index*sizeof(RrSpritePreset)+delta;
                check(extremes[i],index);
            }
        }
        for(unsigned i=0;i<sizeof(extremes)/sizeof(*extremes);i++) {
            /* The first head write redirects the full native page pointer to
             * the packet. Its next reload must clear packet byte 7. */
            initialize(i);
            actual.scratch.order=(u32*)((u8*)&actual+offsetof(World,scratch)+offsetof(RrGpuScratchPointers,order)-4);
            check(extremes[i],i%32);
            initialize(i);
            actual.scratch.order=(u32*)((u8*)&actual+offsetof(World,packets)-4);
            check(extremes[i],i%32);
            initialize(i);
            actual.scratch.order=(u32*)((u8*)&actual+offsetof(World,scratch)+offsetof(RrGpuScratchPointers,cursor)-4);
            check(extremes[i],i%32);
        }
    } else {
        /* Neither invalid table index nor inaccessible cursor/order is used. */
        initialize(0);actual.scratch.cursor=0;actual.scratch.order=0;
        check(0,(-2147483647-1));check(0,2147483647);
    }
    return 0;
}
'''
        source=Path(cls.temporary.name)/'sprite_preset.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_clamp_and_all_table_entries(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_live_table_and_order_aliases(self):
        subprocess.run([str(self.binary),'1'],check=True)

    def test_zero_intensity_skips_all_accesses(self):
        subprocess.run([str(self.binary),'2'],check=True)


if __name__=='__main__':
    unittest.main()
