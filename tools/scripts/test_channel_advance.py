"""Actual channel advance: first-pass modes, inclusive bound and live callback state."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ChannelAdvanceTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        header=(ROOT/'include/rr/channels.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_80099E9C.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        prelude=r'''
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header + r'''
RrChannelRecordF0 PadChannelRecordArray[2];
s32 D_800ABCC0[2], PadActiveChannel, PadRecvStage, PadLastChannel;
u16 packet_a[12], packet_b[12], *D_800ABD00;
static int scenario, trace[32], trace_count, ready_count, ready_result;
static int id(void *p) {
    if(p==&PadChannelRecordArray[0])return 0;
    assert(p==&PadChannelRecordArray[1]);return 1;
}
static void event(int value) {assert(trace_count<32);trace[trace_count++]=value;}
void _dirFailAuto(u8 *channel) {
    int index=id(channel);event(10+index);
    if(scenario==1) {assert(index==0);PadActiveChannel=1;D_800ABD00=packet_b;}
    if(scenario==4) PadLastChannel=0;
}
void _padSaveCommand(RrChannelTransferView *channel) {
    int index=id(channel);event(20+index);
    if(scenario==3) {assert(index==0);PadActiveChannel=1;D_800ABD00=packet_b;}
}
s32 _padInitSioMode(RrChannelRecordF0 *channel) {
    int index=id(channel);event(30+index);ready_count++;
    if(scenario==2 && ready_count==1) {
        PadActiveChannel=0;D_800ABD00=packet_b;return 0;
    }
    return ready_result;
}
''' + body
        harness=r'''
static void reset(int start,int last,int result) {
    scenario=0;trace_count=0;ready_count=0;ready_result=result;
    PadActiveChannel=start;PadLastChannel=last;PadRecvStage=0x12345678;
    D_800ABCC0[0]=0x13579;D_800ABCC0[1]=0x24680;
    memset(PadChannelRecordArray,0x5A,sizeof(PadChannelRecordArray));
    for(int i=0;i<12;i++){packet_a[i]=0xA5A5;packet_b[i]=0x5A5A;}
    D_800ABD00=packet_a;
}
static void expect_trace(const int *expected,int size) {
    assert(trace_count==size);
    assert(memcmp(trace,expected,size*sizeof(int))==0);
    const u8 *bytes=(const u8 *)PadChannelRecordArray;
    for(unsigned i=0;i<sizeof(PadChannelRecordArray);i++)assert(bytes[i]==0x5A);
    assert(PadRecvStage==0);
}
static void packets(int changed_a,int changed_b) {
    for(int i=0;i<12;i++) {
        assert(packet_a[i]==(i==5&&changed_a?0:0xA5A5));
        assert(packet_b[i]==(i==5&&changed_b?0:0x5A5A));
    }
}
static void basic(s32 mode,int start,int last,int result) {
    reset(start,last,result);
    int expected[8],n=0;
    if(mode!=0 && mode!=-9){expected[n++]=10+start;expected[n++]=20+start;}
    if(start==0 && last==1) {
        expected[n++]=31;
        if(result==0){expected[n++]=11;expected[n++]=21;}
    }
    _padAdvanceChannel(mode);
    expect_trace(expected,n);
    assert(PadActiveChannel==(start==0&&last==1&&result==0?2:start+1));
    assert(D_800ABCC0[0]==(mode==0&&start==0?0:0x13579));
    assert(D_800ABCC0[1]==(mode==0&&start==1?0:0x24680));
    assert(D_800ABD00==packet_a);packets(1,0);
}
static void basic_cases(void) {
    for(unsigned bits=0;bits<65536;bits++) {
        basic((s16)bits,0,1,0);
        basic((s32)(0x12340000U|bits),0,0,1);
    }
    const s32 modes[]={-9,0,1,-1,INT_MIN,INT_MAX,65535,(s32)0xFFFF0000U};
    const s32 results[]={0,1,7,-1,INT_MIN};
    for(unsigned m=0;m<sizeof(modes)/sizeof(modes[0]);m++)
        for(int start=0;start<2;start++)for(int last=0;last<2;last++)
            for(unsigned r=0;r<sizeof(results)/sizeof(results[0]);r++)
                basic(modes[m],start,last,results[r]);
}
static void live_cases(void) {
    const int captured[]={10,20};
    for(int which=1;which<=3;which+=2) {
        reset(0,1,0);scenario=which;
        _padAdvanceChannel(17);
        expect_trace(captured,2);assert(PadActiveChannel==2&&ready_count==0);
        assert(D_800ABD00==packet_b);packets(0,1);
    }
    reset(0,1,1);scenario=2;
    const int restart[]={31,10,20,31};
    _padAdvanceChannel(-9);
    expect_trace(restart,4);assert(PadActiveChannel==1&&ready_count==2);
    assert(D_800ABD00==packet_b);packets(1,1);
    reset(0,1,0);scenario=4;
    _padAdvanceChannel(1);
    expect_trace(captured,2);assert(PadActiveChannel==1&&ready_count==0);
    assert(PadLastChannel==0);packets(1,0);
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(argv[1][0]=='b')basic_cases();else live_cases();
    return 0;
}
'''
        directory=Path(cls.temporary.name);source=directory/'advance.c'
        source.write_text(prelude+harness);cls.executable=directory/'advance'
        result=subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                               '-fsanitize=undefined','-I',str(ROOT/'include'),
                               str(source),'-o',str(cls.executable)],capture_output=True,text=True)
        if result.returncode:raise AssertionError(result.stderr)

    def run_case(self,case):
        result=subprocess.run([str(self.executable),case],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stderr,'')

    def test_first_modes_bounds_and_ready_status(self):self.run_case('basic')
    def test_live_handler_mutations_and_captured_record(self):self.run_case('live')


if __name__=='__main__':unittest.main()
