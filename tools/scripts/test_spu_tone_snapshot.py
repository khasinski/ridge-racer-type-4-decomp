"""Recovered SPU tone snapshot: signed indexing, live pointers, overlap and guards."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class SpuToneSnapshotTest(unittest.TestCase):
    def test_native_snapshot(self):
        body = (ROOT/'tools/scripts/fixtures/views/func_800850C4.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r',?\s*"\$\d+"', '', body)
        self.assertEqual(body.count('(u8*)0x80110000U'), 1)
        body = body.replace('(u8*)0x80110000U', 'test_flags + 32768')
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16; typedef int32_t s32; typedef uint32_t u32;
typedef struct {u8 bank, padding[4], program, value;} Selection;
Selection SvmCurVabId;
u8 test_flags[65536+0x200];
u8 tone_pool[2097152] __attribute__((aligned(512)));
struct {u8 prefix[16]; union {u8 bytes[32]; u8 *pointer;} state;} holder;
#define selected holder.state
#define _svm_tn selected.pointer
static u8 *next_base;
static u8 next_value;
static s32 selector_result, calls, seen_bank, seen_program;
s32 _SsVmVSetUp(s32 bank,s32 program) {
    calls++; seen_bank=bank; seen_program=program;
    _svm_tn=next_base; SvmCurVabId.value=next_value;
    return selector_result;
}
''' + body + r'''
static const int order[14] = {0,1,2,3,4,5,7,6,8,9,10,11,12,13};
static u8 output[64] __attribute__((aligned(16)));
static void copy_reference(u8 *destination, const u8 *source) {
    for(int i=0;i<14;i++) destination[order[i]]=source[order[i]];
    for(int i=16;i<24;i+=2) {
        uint16_t value; memcpy(&value,source+i,2); memcpy(destination+i,&value,2);
    }
}
static void normal(s32 rawbank,s16 program,s32 tone,u8 flag,u8 value,s32 status) {
    s32 bank=(s16)rawbank;
    test_flags[32768+bank+0x120]=flag;
    memset(output,0xA5,sizeof(output));
    u8 expected[64]; memcpy(expected,output,sizeof(output));
    next_base=tone_pool+1048576; next_value=value; selector_result=status; calls=0;
    s32 index=(s16)((u32)tone+(u32)(s32)(int8_t)value*16U);
    if(flag==1) copy_reference(expected+8,next_base+index*32);
    s32 result=SsUtGetVagAtr(rawbank,program,tone,output+8);
    assert(result==(flag==1?0:-1));
    assert(calls==(flag==1));
    if(flag==1) assert(seen_bank==bank && seen_program==program);
    assert(memcmp(output,expected,sizeof(output))==0);
}
int main(void) {
    for(unsigned i=0;i<sizeof(tone_pool);i++)tone_pool[i]=(u8)(i*73U+(i>>7)+19U);
    const s32 banks[]={-32768,-513,-1,0,1,16,32767};
    for(unsigned k=0;k<sizeof(banks)/sizeof(banks[0]);k++)
        for(unsigned f=0;f<256;f++)
            normal((s32)(0xA5A50000U|(u16)banks[k]),-7,-31,(u8)f,0xFF,-1);
    for(unsigned value=0;value<256;value++) {
        normal(0,(s16)value,32767,1,(u8)value,-1);
        normal(0,(s16)value,-32768,1,(u8)value,0);
    }
    for(unsigned tone=0;tone<65536;tone++) {
        normal(0x12340000,(s16)tone,(s32)(0xA5A50000U|tone),1,0x80,-1);
        normal(0,(s16)tone,(s32)(0xFFFF0000U|tone),1,0x7F,0);
    }
    /* Overlap oracle observes the original byte sequence, including 7 before 6. */
    for(int shift=-8;shift<=8;shift+=2) {
        u8 original[128], expected[128];
        u8 *start=tone_pool+1048576;
        for(int i=0;i<128;i++)original[i]=(u8)(i*29+7);
        memcpy(start-32,original,128); memcpy(expected,original,128);
        copy_reference(expected+32+shift,expected+32);
        next_base=start;next_value=0;selector_result=-1;calls=0;
        test_flags[32768+0x120]=1;
        assert(SsUtGetVagAtr(0,0,0,start+shift)==0 && calls==1);
        assert(memcmp(start-32,expected,128)==0);
    }
    /* Byte zero changes the selected pointer; every later byte must reload it. */
    u8 *initial=tone_pool+1048576, *redirected=initial+64;
    assert(((uintptr_t)initial&255)==0);
    u8 representation[sizeof(redirected)];memcpy(representation,&redirected,sizeof(redirected));
    initial[0]=representation[0];
    memcpy(redirected+1,representation+1,sizeof(redirected)-1);
    for(unsigned i=sizeof(redirected);i<32;i++)redirected[i]=(u8)(0x30+i);
    memset(selected.bytes,0xA5,sizeof(selected.bytes));
    next_base=initial;next_value=0;selector_result=-1;
    u8 expected[32]; memset(expected,0xA5,sizeof(expected));
    memcpy(expected,representation,sizeof(representation));
    for(int i=8;i<14;i++)expected[i]=redirected[i];
    memcpy(expected+16,redirected+16,8);
    assert(SsUtGetVagAtr(0,0,0,selected.bytes)==0);
    assert(_svm_tn==redirected && memcmp(selected.bytes,expected,32)==0);
    /* The final four halfwords share one captured source pointer. */
    memset(&holder,0xA5,sizeof(holder));
    memset(redirected+16,0xCC,8);
    memcpy(initial+16,representation,sizeof(representation));
    u8 tail_expected[sizeof(holder)]; memset(tail_expected,0xA5,sizeof(tail_expected));
    copy_reference(tail_expected,initial);
    next_base=initial; next_value=0; selector_result=-1;
    assert(SsUtGetVagAtr(0,0,0,holder.prefix)==0);
    assert(_svm_tn==redirected);
    assert(memcmp(&holder,tail_expected,sizeof(holder))==0);
    return 0;
}
'''
        with TemporaryDirectory() as temporary:
            directory=Path(temporary); source=directory/'tone.c'; executable=directory/'tone'
            source.write_text(code)
            result=subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                                   '-fsanitize=undefined',str(source),'-o',str(executable)],
                                  capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            result=subprocess.run([str(executable)],capture_output=True,text=True)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual(result.stderr,'')


if __name__=='__main__':unittest.main()
