"""Actual guarded channel reset: dispatch, signed selectors, reentry and cleanup."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class ChannelResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        header=(ROOT/'include/rr/channels.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_8008564C.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body).replace(':,"memory"',':"memory"')
        assert body.count('(u8*)0x80110000U')==6
        body=body.replace('(u8*)0x80110000U','ram.bytes+0x2000')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header + r'''
static union {uint64_t alignment;u8 bytes[0x4000];} ram;
#define _svm_voice (*(RrChannelControlRow36 (*)[24])(ram.bytes+0x1908))
#define _snd_ev_flag (*(s32 *)(ram.bytes+0x105C))
#define SvmVoiceIndex (*(u16 *)(ram.bytes+0x2118))
''' + body + r'''
static int voice,fast_calls,ordinary_calls,dynamic;
static u8 snapshot[sizeof(ram)],expected[sizeof(ram)];
static void halfword(u8 *pointer,u16 value) {memcpy(pointer,&value,2);}
static void mutation(u8 *memory) {
    RrChannelControlRow36 *rows=(RrChannelControlRow36 *)(memory+0x1908);
    int other=(voice+1)%24;
    rows[voice].value00=-123;
    rows[voice].match18=17;rows[voice].match14=-29;rows[voice].group0E=31;
    halfword(rows[voice].pad16,0x8123);
    halfword(rows[voice].pad1E,0x1234);halfword(rows[voice].pad1E+12,0xABCD);
    halfword(rows[other].pad1E,0x5678);halfword(rows[other].pad1E+12,0x9876);
    halfword(memory+0x2118,other);
    s32 changed_lock=-7;memcpy(memory+0x105C,&changed_lock,4);
}
static void callback(void) {
    assert(_snd_ev_flag==1);
    if(dynamic) {
        memcpy(snapshot,ram.bytes,sizeof(snapshot));
        assert(SsUtKeyOff((voice+1)%24,0,0,0,0)==-1);
        assert(memcmp(snapshot,ram.bytes,sizeof(snapshot))==0);
        mutation(ram.bytes);
    }
}
void vmNoiseOff(u8 index) {assert(index==voice);fast_calls++;callback();}
void _SsVmKeyOffNow(s32 unused) {
    assert(unused==0&&SvmVoiceIndex==voice);ordinary_calls++;callback();
}
static void prepare(int selected,int lock,s16 bank,s16 program,s16 instrument,s16 group,s16 kind,int live) {
    memset(ram.bytes,0xA5,sizeof(ram.bytes));
    voice=selected;fast_calls=ordinary_calls=0;dynamic=live;_snd_ev_flag=lock;
    _svm_voice[voice].match18=bank;_svm_voice[voice].match14=program;
    _svm_voice[voice].group0E=group;_svm_voice[voice].value00=kind;
    halfword(_svm_voice[voice].pad16,(u16)instrument);
}
static void run(s32 rawvoice,s32 bank,s32 program,s32 instrument,s32 group) {
    memcpy(expected,ram.bytes,sizeof(expected));
    int lock=_snd_ev_flag,valid=(u16)rawvoice<24,fast=0;
    if(valid)assert((u16)rawvoice==voice);
    if(lock!=1) {
        if(valid) {
            s16 row_instrument;
            memcpy(&row_instrument,_svm_voice[voice].pad16,2);
            fast=_svm_voice[voice].match18==(s16)bank
              &&_svm_voice[voice].match14==(s16)program
              &&row_instrument==(s16)instrument
              &&_svm_voice[voice].group0E==(s16)group
              &&_svm_voice[voice].value00==255;
            if(!fast)halfword(expected+0x2118,voice);
            if(dynamic)mutation(expected);
            RrChannelControlRow36 *rows=(RrChannelControlRow36 *)(expected+0x1908);
            halfword(rows[voice].pad1E,0);halfword(rows[voice].pad1E+12,0);
        }
        s32 zero=0;memcpy(expected+0x105C,&zero,4);
    }
    int result=SsUtKeyOff(rawvoice,bank,program,instrument,group);
    assert(result==(lock!=1&&valid?0:-1));
    assert(fast_calls==(lock!=1&&valid&&fast));
    assert(ordinary_calls==(lock!=1&&valid&&!fast));
    assert(memcmp(expected,ram.bytes,sizeof(expected))==0);
}
static unsigned random_state=0x72A683E1;
static unsigned random_value(void) {
    random_state=random_state*1664525U+1013904223U;return random_state;
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(!strcmp(argv[1],"voices")) {
        const int locks[]={0,1,-1,2,INT_MIN};
        for(unsigned i=0;i<sizeof(locks)/sizeof(*locks);i++)
        for(unsigned v=0;v<65536;v++) {
            prepare(v<24?v:5,locks[i],-32768,32767,-1,17,255,0);
            run((s32)(0xA5A50000U|v),(s32)0x12348000U,(s32)0xABCD7FFFU,
                (s32)0x9876FFFFU,(s32)0x56780011U);
        }
    } else if(!strcmp(argv[1],"dispatch")) {
        const s16 kinds[]={-32768,-1,0,255,256,32767};
        for(unsigned i=0;i<12000;i++) {
            s16 bank=random_value(),program=random_value(),instrument=random_value(),group=random_value();
            int v=random_value()%24;
            for(int failure=0;failure<5;failure++)
            for(unsigned k=0;k<sizeof(kinds)/sizeof(*kinds);k++) {
                prepare(v,0,bank,program,instrument,group,kinds[k],0);
                s32 b=(s32)(0xABCD0000U|(u16)bank),p=(s32)(0x12340000U|(u16)program);
                s32 t=(s32)(0xFFFF0000U|(u16)instrument),g=(s32)(0x87650000U|(u16)group);
                if(failure==1)b^=1;if(failure==2)p^=1;
                if(failure==3)t^=1;if(failure==4)g^=1;
                run((s32)(0x12340000U|v),b,p,t,g);
            }
        }
    } else {
        for(int v=0;v<24;v++)
        for(int fast=0;fast<2;fast++) {
            prepare(v,2,-1,2,-32768,32767,fast?255:0,1);
            run((s32)(0xA5A50000U|v),-1,2,-32768,32767);
        }
    }
    return 0;
}
'''
        directory=Path(cls.temporary.name);source=directory/'reset.c'
        source.write_text(code);cls.executable=directory/'reset'
        result=subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                               '-fsanitize=undefined','-fno-strict-aliasing',
                               '-I',str(ROOT/'include'),str(source),'-o',str(cls.executable)],
                              capture_output=True,text=True)
        if result.returncode:raise AssertionError(result.stderr)

    def run_case(self,case):
        result=subprocess.run([str(self.executable),case],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stderr,'')

    def test_all_low16_voices_and_lock_values(self):self.run_case('voices')
    def test_signed_field_dispatch_and_kind255(self):self.run_case('dispatch')
    def test_callback_mutation_reentry_and_original_row_cleanup(self):self.run_case('live')


if __name__=='__main__':unittest.main()
