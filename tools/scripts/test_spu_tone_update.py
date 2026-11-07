"""Actual SPU tone writer: signed selection, live targets, capture and overlap."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class SpuToneUpdateTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        header=(ROOT/'include/rr/spu.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_800867E4.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body)
        assert body.count('(u8*)0x80110000U')==1
        body=body.replace('(u8*)0x80110000U','flags+32768')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header + r'''
RrSpuSelectionPrefix SvmCurVabId;
static u8 flags[65536+0x200];
static u8 tones[2097280] __attribute__((aligned(512)));
static u8 input[32] __attribute__((aligned(16)));
static struct {
    u8 prefix[64];
    union {u8 bytes[256];u8 *pointer;} state;
} holder __attribute__((aligned(65536)));
#define _svm_tn holder.state.pointer
static u8 *selected_base;
static u8 selected_value;
static s32 selection_result,calls,seen_bank,seen_program;
s32 _SsVmVSetUp(s32 bank,s32 program) {
    calls++;seen_bank=bank;seen_program=program;
    _svm_tn=selected_base;SvmCurVabId.value=selected_value;
    return selection_result;
}
''' + body + r'''
static const int order[14]={0,1,2,3,4,5,7,6,8,9,10,11,12,13};
static void copy_reference(u8 *destination,const u8 *source) {
    for(int i=0;i<14;i++)destination[order[i]]=source[order[i]];
    for(int i=16;i<24;i+=2) {
        u16 value;memcpy(&value,source+i,2);memcpy(destination+i,&value,2);
    }
}
static void normal(s32 rawbank,s32 program,s32 tone,u8 flag,u8 value,s32 status) {
    int bank=(s16)rawbank;
    flags[32768+bank+0x120]=flag;
    selected_base=tones+1048640;selected_value=value;selection_result=status;calls=0;
    int index=(s16)((u32)tone+(u32)(s32)(s8)value*16U);
    u8 *destination=selected_base+index*32;
    memset(destination-8,0xA5,48);
    u8 want[48],saved_input[32];memcpy(want,destination-8,48);
    memcpy(saved_input,input,sizeof(input));
    if(flag==1)copy_reference(want+8,input);
    assert(SsUtSetVagAtr(rawbank,program,tone,input)==(flag==1?0:-1));
    assert(calls==(flag==1));
    if(flag==1)assert(seen_bank==bank&&seen_program==(s16)program);
    assert(memcmp(destination-8,want,48)==0);
    assert(memcmp(saved_input,input,sizeof(input))==0);
}
static void reset_alias(void) {
    memset(&holder,0xA5,sizeof(holder));
    for(unsigned i=0;i<sizeof(input);i++)input[i]=(u8)(i*29+7);
    flags[32768+0x120]=1;selected_value=0;selection_result=-1;calls=0;
}
int main(int argc,char **argv) {
    assert(argc==2);
    for(unsigned i=0;i<sizeof(input);i++)input[i]=(u8)(i*73+19);
    if(!strcmp(argv[1],"selectors")) {
        const s32 banks[]={-32768,-513,-1,0,1,16,32767};
        for(unsigned b=0;b<sizeof(banks)/sizeof(*banks);b++)
            for(unsigned f=0;f<256;f++)
                normal((s32)(0xA5A50000U|(u16)banks[b]),(s32)0x87658001U,-31,f,0xFF,-1);
        for(unsigned value=0;value<256;value++) {
            normal(0,(s32)(0xABCD0000U|value),32767,1,value,-1);
            normal(0,-32768,-32768,1,value,0);
        }
        for(unsigned tone=0;tone<65536;tone++) {
            normal(0x12340000,(s32)(0xBEEF0000U|tone),(s32)(0xA5A50000U|tone),1,0x80,-1);
            normal(0,(s16)tone,(s32)(0xFFFF0000U|tone),1,0x7F,0);
        }
    } else if(!strcmp(argv[1],"overlap")) {
        for(int shift=-16;shift<=16;shift+=2) {
            u8 want[128];
            u8 *start=tones+1048640;
            for(int i=0;i<128;i++)start[i-32]=(u8)(i*29+7);
            memcpy(want,start-32,128);
            copy_reference(want+32,want+32+shift);
            selected_base=start;selected_value=0;selection_result=-1;
            flags[32768+0x120]=1;
            assert(SsUtSetVagAtr(0,0,0,start+shift)==0);
            assert(memcmp(start-32,want,128)==0);
        }
    } else {
        /* Byte zero changes the live selected target. */
        reset_alias();
        u8 *initial=holder.state.bytes,*redirected=initial+64;
        selected_base=initial;
        u8 representation[sizeof(redirected)];
        memcpy(representation,&redirected,sizeof(redirected));
        input[0]=representation[0];
        _svm_tn=initial;
        u8 want[sizeof(holder)];memcpy(want,&holder,sizeof(holder));
        want[64]=input[0];
        for(int i=1;i<14;i++)want[128+order[i]]=input[order[i]];
        for(int i=16;i<24;i++)want[128+i]=input[i];
        assert(SsUtSetVagAtr(0,0,0,input)==0);
        assert(_svm_tn==redirected);
        assert(memcmp(&holder,want,sizeof(holder))==0);

        /* First halfword changes the global pointer; later halfwords retain
         * the destination captured before that write. */
        reset_alias();
        initial=(u8*)&holder+48;redirected=holder.state.bytes+64;
        selected_base=initial;_svm_tn=initial;
        memcpy(input+16,&redirected,sizeof(redirected));
        memcpy(want,&holder,sizeof(holder));
        copy_reference(want+48,input);
        assert(SsUtSetVagAtr(0,0,0,input)==0);
        assert(_svm_tn==redirected);
        assert(memcmp(&holder,want,sizeof(holder))==0);
    }
    return 0;
}
'''
        directory=Path(cls.temporary.name);source=directory/'update.c'
        source.write_text(code);cls.executable=directory/'update'
        result=subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                               '-fsanitize=undefined','-fno-strict-aliasing',
                               '-I',str(ROOT/'include'),str(source),'-o',str(cls.executable)],
                              capture_output=True,text=True)
        if result.returncode:raise AssertionError(result.stderr)

    def run_case(self,case):
        result=subprocess.run([str(self.executable),case],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stderr,'')

    def test_signed_selectors_tones_flags_and_ignored_status(self):self.run_case('selectors')
    def test_sequential_overlap_and_guards(self):self.run_case('overlap')
    def test_live_byte_target_and_captured_halfword_target(self):self.run_case('alias')


if __name__=='__main__':unittest.main()
