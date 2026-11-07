"""Actual channel pitch setter with canonical row/cache layouts and live callbacks."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class ChannelPitchTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_80084E74.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body)
        body=body.replace(':,"memory"',':"memory"')
        assert body.count('(u8*)0x80110000U')==5
        body=body.replace('(u8*)0x80110000U','ram.bytes+0x2000')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('spu.h') + header('channels.h') + header('channel_pitch.h') + r'''
static union {uint64_t alignment;u8 bytes[0x4000];} ram;
#define _svm_voice (*(RrChannelControlRow36 (*)[24])(ram.bytes+0x1908))
#define _svm_sreg_dirty (*(RrSpuVolumeCache *)(ram.bytes+0x1E18))
#define D_8011010C (*(RrSpuPitchRequest *)(ram.bytes+0x210C))
#define SvmLastChannel (*(s16 *)(ram.bytes+0x2114))
#define SvmVoiceIndex (*(u16 *)(ram.bytes+0x2118))
''' + body + r'''
static int voice,selection_calls,pitch_calls,mutate;
static s32 selection_status,seen_bank,seen_program;
static u32 seen_first,seen_second;
static u16 result_pitch(u32 first,u32 second) {return (u16)(first*257U+second*11U);}
s32 _SsVmVSetUp(s32 bank,s32 program) {
    selection_calls++;seen_bank=bank;seen_program=program;
    if(mutate) {
        _svm_voice[voice].pad16[0]=0xEC;
        _svm_voice[voice].match18=1234;
        _svm_voice[voice].match14=-2468;
        _svm_voice[voice].group0E=777;
    }
    return selection_status;
}
u16 note2pitch2(s32 first,s32 second) {
    assert(selection_calls==1);
    assert(SvmLastChannel==0x21&&SvmVoiceIndex==voice);
    assert(ram.bytes[0x210C]==_svm_voice[voice].pad16[0]);
    assert(first>=0&&first<=65535&&second>=0&&second<=65535);
    pitch_calls++;seen_first=first;seen_second=second;
    if(mutate) {
        _svm_sreg_dirty.flags[voice]=0x80;
        u16 value=0xBEEF;
        memcpy(_svm_sreg_dirty.voices[voice].pad04,&value,2);
    }
    return result_pitch(first,second);
}
static u8 expected[sizeof(ram)];
static void prepare(int selected,s16 bank,s16 program,s16 group,int dynamic) {
    memset(ram.bytes,0xA5,sizeof(ram.bytes));
    voice=selected;mutate=dynamic;selection_calls=pitch_calls=0;
    _svm_voice[voice].match18=bank;
    _svm_voice[voice].match14=program;
    _svm_voice[voice].group0E=group;
    _svm_voice[voice].pad16[0]=0x17;
    _svm_sreg_dirty.flags[voice]=0x42;
}
static void run(s32 rawvoice,s32 bank,s32 program,s32 group,u32 first,u32 second) {
    memcpy(expected,ram.bytes,sizeof(expected));
    int valid=(u16)rawvoice<24;
    if(valid) {
        assert((u16)rawvoice==voice);
        valid=_svm_voice[voice].match18==(s16)bank
            &&_svm_voice[voice].match14==(s16)program
            &&_svm_voice[voice].group0E==(s16)group;
    }
    s32 expected_bank=_svm_voice[voice].match18;
    s32 expected_program=_svm_voice[voice].match14;
    if(valid) {
        RrChannelControlRow36 *rows=(RrChannelControlRow36 *)(expected+0x1908);
        RrSpuVolumeCache *cache=(RrSpuVolumeCache *)(expected+0x1E18);
        if(mutate) {
            rows[voice].pad16[0]=0xEC;rows[voice].match18=1234;
            rows[voice].match14=-2468;rows[voice].group0E=777;
            cache->flags[voice]=0x80;
        }
        s16 mode=0x21;u16 slot=voice,pitch=result_pitch((u16)first,(u16)second);
        memcpy(expected+0x2114,&mode,2);memcpy(expected+0x2118,&slot,2);
        expected[0x210C]=rows[voice].pad16[0];
        memcpy(cache->voices[voice].pad04,&pitch,2);
        cache->flags[voice]|=4;
    }
    assert(SsUtChangePitch(rawvoice,bank,program,group,0xDEADBEEF,first,second)==(valid?0:-1));
    assert(selection_calls==valid&&pitch_calls==valid);
    if(valid) {
        assert(seen_bank==expected_bank&&seen_program==expected_program);
        assert(seen_first==(u16)first&&seen_second==(u16)second);
    }
    assert(memcmp(ram.bytes,expected,sizeof(expected))==0);
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(!strcmp(argv[1],"voices")) {
        for(unsigned v=0;v<65536;v++) {
            int selected=v<24?v:5;
            prepare(selected,-32768,32767,-1,0);selection_status=-1;
            run((s32)(0xA5A50000U|v),(s32)0x12348000U,(s32)0xFFFF7FFFU,
                (s32)0x5678FFFFU,0xABCDFFFFU,0x12348000U);
        }
    } else if(!strcmp(argv[1],"matches")) {
        const s16 values[]={-32768,-129,-1,0,1,127,32767};
        const s32 statuses[]={-1,0,17};
        for(int v=0;v<24;v++)
        for(unsigned b=0;b<sizeof(values)/sizeof(*values);b++)
        for(unsigned p=0;p<sizeof(values)/sizeof(*values);p++)
        for(unsigned g=0;g<sizeof(values)/sizeof(*values);g++)
        for(int failure=0;failure<4;failure++) {
            prepare(v,values[b],values[p],values[g],0);selection_status=statuses[v%3];
            s32 bank=(s32)(0xABCD0000U|(u16)values[b]);
            s32 program=(s32)(0x12340000U|(u16)values[p]);
            s32 group=(s32)(0xFFFF0000U|(u16)values[g]);
            if(failure==1)bank^=1;
            if(failure==2)program^=1;
            if(failure==3)group^=1;
            run(v,bank,program,group,0x8000,0xFFFF);
        }
    } else {
        for(unsigned x=0;x<65536;x++) {
            prepare(x%24,-1,2,-3,1);selection_status=-123;
            run((s32)(0xA5A50000U|(x%24)),-1,2,-3,0x12340000U|x,0xABCD0000U|(65535-x));
        }
    }
    return 0;
}
'''
        directory=Path(cls.temporary.name);source=directory/'pitch.c'
        source.write_text(code);cls.executable=directory/'pitch'
        result=subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                               '-fsanitize=undefined','-fno-strict-aliasing',
                               '-Wno-unused-parameter','-I',str(ROOT/'include'),
                               str(source),'-o',str(cls.executable)],capture_output=True,text=True)
        if result.returncode:raise AssertionError(result.stderr)

    def run_case(self,case):
        result=subprocess.run([str(self.executable),case],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stderr,'')

    def test_all_unsigned_low16_voice_values(self):self.run_case('voices')
    def test_signed_selector_matches_and_rejections(self):self.run_case('matches')
    def test_all_pitch_halfwords_and_live_callback_mutations(self):self.run_case('live')


if __name__=='__main__':unittest.main()
