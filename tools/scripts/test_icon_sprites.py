"""Execute the actual shared TU, including context redirection between packets."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class IconSpritesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/legacy/icon_sprites.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body)
        assert body.count('(RrGpuScratchPointers*)0x1F800000U')==1
        assert body.count('(u8*)0x800B0000U')==1
        body=body.replace('(RrGpuScratchPointers*)0x1F800000U','&actual->scratch')
        # The translated page stays inside one byte array; subtracting33EC is
        # defined and lands on an eight-byte-aligned full native pointer slot.
        body=body.replace('(u8*)0x800B0000U','actual->ram+0x346C')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('gpu_scratch_types.h')+header('gpu.h') + header('animated_sprite_grid.h') + header('icon_sprites.h') + r'''
typedef struct {
    RrGpuScratchPointers scratch;
    u8 guard[32];
    u8 ram[0x4000] __attribute__((aligned(8)));
    RrIconSpriteContext contexts[2];
    u8 packets[192];
    u16 cluts[6];
    u8 suffix[64];
} World;
static World *actual,*expected;
#define D_8001121C actual->cluts
''' + body + r'''
static u8 *locate(const void *address,size_t size) {
    uintptr_t offset=(uintptr_t)address-(uintptr_t)actual;
    assert(offset<=sizeof(*actual)-size);return (u8*)expected+offset;
}
static u32 read(const void *address,int size) {
    const u8 *bytes=locate(address,size);u32 value=0;
    for(int i=0;i<size;i++)value|=(u32)bytes[i]<<(8*i);return value;
}
static void store(void *address,u32 value,int size) {
    u8 *bytes=locate(address,size);for(int i=0;i<size;i++)bytes[i]=value>>(8*i);
}
static RrIconSpriteContext *context(void) {
    RrIconSpriteContext *pointer;memcpy(&pointer,expected->ram+0x80,sizeof(pointer));return pointer;
}
static void reference(int variant,s32 index,s32 color) {
    u8 *p;memcpy(&p,&expected->scratch.cursor,sizeof(p));
    store(p+3,4,1);store(p+7,0x66,1);
    store(p+8,148,2);store(p+10,variant?128:120,2);
    store(p+16,24,2);store(p+18,16,2);
    u32 clut=read(&actual->cluts[index],2);
    store(p+4,color,1);store(p+5,color,1);store(p+6,color,1);
    store(p+12,(u32)index*24U+112U,1);store(p+13,172,1);
    RrIconSpriteContext *captured=context();
    store(p+14,clut,2);
    store(p,(read(p,4)&0xFF000000U)|(read(&captured->head70,4)&0xFFFFFFU),4);
    store(&captured->head70,(read(&captured->head70,4)&0xFF000000U)|((u32)(uintptr_t)p&0xFFFFFFU),4);
    p+=20;memcpy(&expected->scratch.cursor,&p,sizeof(p));
    store(p+3,1,1);
    captured=context();
    store(p+4,0xE100023E,4);
    store(p,(read(p,4)&0xFF000000U)|(read(&captured->head70,4)&0xFFFFFFU),4);
    store(&captured->head70,(read(&captured->head70,4)&0xFF000000U)|((u32)(uintptr_t)p&0xFFFFFFU),4);
    p+=8;memcpy(&expected->scratch.cursor,&p,sizeof(p));
}
static void slot(RrIconSpriteContext *pointer) {
    memcpy(actual->ram+0x80,&pointer,sizeof(pointer));
}
static void initialize(u32 seed) {
    memset(actual,(u8)(seed^(seed>>8)),sizeof(*actual));
    actual->scratch.cursor=actual->packets;
    actual->scratch.order=0; /* This emitter uses the context, not scratch.order. */
    actual->contexts[0].head70=seed*2654435761U;
    actual->contexts[1].head70=~seed;
    for(int i=0;i<6;i++)actual->cluts[i]=(u16)(seed*1717U+i*0x1234U);
    slot(&actual->contexts[0]);
}
static void check(int variant,int index,s32 color) {
    memcpy(expected,actual,sizeof(*actual));reference(variant,index,color);
    if(variant)func_80059D34(index,color);else EmitIconSpriteRow120(index,color);
    assert(!memcmp(actual,expected,sizeof(*actual)));
}
int main(int argc,char **argv) {
    assert(argc==2);
    assert(sizeof(*actual)<65536);
    assert(!posix_memalign((void**)&actual,65536,sizeof(*actual)));
    expected=malloc(sizeof(*expected));assert(expected);
    const s32 extremes[]={(-2147483647-1),-65536,-257,-256,-255,-1,0,1,127,128,255,256,257,65535,65536,2147483647};
    int mode=atoi(argv[1]);
    if(mode==0) {
        for(int variant=0;variant<2;variant++)for(u32 i=0;i<65536;i++) {
            u32 raw=i|(i*1717U<<16);s32 color;memcpy(&color,&raw,4);
            initialize(raw);check(variant,(i/256)%6,color);
        }
        for(int variant=0;variant<2;variant++)for(int index=0;index<6;index++) {
            for(unsigned i=0;i<sizeof(extremes)/sizeof(*extremes);i++) {
                initialize(i*71U+index);check(variant,index,extremes[i]);
                check(1-variant,index,extremes[i]);
            }
        }
    } else if(mode==1) {
        for(int variant=0;variant<2;variant++)for(int index=0;index<6;index++) {
            for(int delta=-16;delta<=4;delta+=4) {
                initialize(index*71U+delta);
                actual->scratch.cursor=(u8*)actual+offsetof(World,cluts)+delta;
                check(variant,index,0x12345678);
            }
            for(int offset=0;offset<28;offset+=4) {
                initialize(index*17U+offset);
                actual->scratch.cursor=actual->ram+0x500;
                slot((RrIconSpriteContext*)(actual->ram+0x500+offset-0x70));
                check(variant,index,-1);
            }
        }
    } else {
        for(int variant=0;variant<2;variant++)for(int index=0;index<6;index++) {
            initialize(index);
            /* First head aliases the native context-pointer low word. The
             * head insertion preserves its upper byte and redirects the second
             * context capture to the first packet, whose head is112 bytes on. */
            slot((RrIconSpriteContext*)(actual->ram+0x10));
            check(variant,index,0x12345678);
        }
    }
    free(expected);free(actual);return 0;
}
'''
        source=Path(cls.temporary.name)/'icon_sprites.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_both_entries_color_words_and_repeated_emission(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_clut_and_packet_head_aliases(self):
        subprocess.run([str(self.binary),'1'],check=True)

    def test_context_redirect_between_packets(self):
        subprocess.run([str(self.binary),'2'],check=True)


if __name__=='__main__':
    unittest.main()
