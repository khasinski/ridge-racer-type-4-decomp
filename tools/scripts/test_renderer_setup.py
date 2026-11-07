"""Execute the actual renderer initializer and verify all untouched storage."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class RendererSetupTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_80013890.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        assert body.count('*(s32*)0x1F800060U')==1
        assert body.count('(RrRendererScratch*)0x1F800000U')==1
        body=body.replace('*(s32*)0x1F800060U','actual.scratch.depthShift60')
        body=body.replace('(RrRendererScratch*)0x1F800000U','&actual.scratch')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('gpu.h') + header('renderer_setup.h') + r'''
typedef struct {
    u8 prefix[16];
    RrRendererScratch scratch;
    u8 middle[16];
    s32 flags;
    u8 gap[12];
    RrGpuColorCommand gouraud;
    u32 untouched1C;
    RrGpuColorCommand flat;
    u32 untouched24;
    u8 suffix[16];
} World;
static World actual,expected;
#define MirrorMode actual.flags
#define PolyGT4WhiteColor actual.gouraud
#define DefaultFT4Color actual.flat
''' + body + r'''
static void store(size_t offset,u32 value,int size) {
    for(int i=0;i<size;i++)((u8*)&expected)[offset+i]=value>>(8*i);
}
static void check(s32 shift,u32 flagbits,u32 seed) {
    for(size_t i=0;i<sizeof(actual);i++) {
        seed=seed*1664525U+1013904223U;((u8*)&actual)[i]=seed>>24;
    }
    memcpy(&actual.flags,&flagbits,4);
    memcpy(&expected,&actual,sizeof(actual));
    /* Independent memory specification, rather than the source field names. */
    size_t scratch=offsetof(World,scratch);
    store(scratch+0x60,(u32)shift,4);
    store(scratch+0x64,flagbits,4);
    store(offsetof(World,flat),0x2C808080U,4);
    store(offsetof(World,gouraud),0x3CFFFFFFU,4);
    store(scratch+0x70,320,2);
    store(scratch+0x6C,0,2);
    store(scratch+0x6E,0,2);
    store(scratch+0x72,256,2);
    SetupRenderer(shift);
    assert(!memcmp(&actual,&expected,sizeof(actual)));
    /* A second call must replace stale parameters and templates again. */
    SetupRenderer(shift);
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
    assert(argc==2);
    const s32 extremes[]={(-2147483647-1),-65536,-32768,-1,0,1,31,32,32767,65535,65536,2147483647};
    if(atoi(argv[1])==0) {
        for(u32 i=0;i<65536;i++) {
            s32 shift=(s32)i-32768;
            check(shift,i*2654435761U,i*1717U);
        }
    } else {
        for(u32 i=0;i<65536;i++) {
            u32 flags=i|(i<<16);
            for(unsigned j=0;j<sizeof(extremes)/sizeof(*extremes);j++)check(extremes[j],flags,i+j*712U);
        }
        for(u32 i=0;i<65536;i++) {
            u32 raw=i*2654435761U;s32 shift;memcpy(&shift,&raw,4);
            check(shift,raw^0x80000001U,i);
        }
    }
    return 0;
}
'''
        source=Path(cls.temporary.name)/'renderer_setup.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_all_signed_halfword_shifts(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_flags_and_full_word_shift_extremes(self):
        subprocess.run([str(self.binary),'1'],check=True)


if __name__=='__main__':
    unittest.main()
