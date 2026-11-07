"""Execute the installed queue source against an independent state model."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class SequenceQueueTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        header=(ROOT/'include/rr/sequence_queue.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/views/func_8004F2D8.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        assert body.count('(u8*)0x800F0000U')==1
        body=body.replace('(u8*)0x800F0000U','state_page.bytes')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header + r'''
static union {uint64_t alignment;u8 bytes[0x8000];} state_page;
#define SoundState (*(RrSequenceQueueState *)(state_page.bytes+0x4DB8))
''' + body + r'''
s16 *SoundSequenceTable[RR_SEQUENCE_TABLE_COUNT][2];
s32 SoundSequenceSet;
static u8 expected[0x8000];
static s16 values[20],other[2]={123,0};
static int model(int id,int restart) {
    RrSequenceQueueState *state=(RrSequenceQueueState *)(expected+0x4DB8);
    const s16 *input=SoundSequenceTable[id][SoundSequenceSet!=0];
    if(input==(s16*)-1)return -1;
    int used=0;
    if(!state->active19D0 || restart) {
        if(!state->active19D0)state->active19D0=1;
        state->timer19D8=state->timer19DC=0;
        state->mode19E0=restart;state->x19E4=state->y19E6=80;
    } else {
        int length=0;
        while(input[length]>0)length++;
        used=state->count19D4;
        if(used+length>=8)return -1;
    }
    int copied=0;
    while(*input>0){state->items19E8[used++]=(u16)*input++;copied++;}
    state->items19E8[used]=65535;
    state->count19D4=used;
    if(!copied)state->active19D0=0;
    return used*2;
}
static void prepare(int active,int used,int length,int terminal) {
    memset(state_page.bytes,0xA5,sizeof(state_page.bytes));
    SoundState.active19D0=active;SoundState.count19D4=used;
    for(int i=0;i<8;i++)SoundState.items19E8[i]=100+i;
    for(int i=0;i<length;i++)values[i]=(i&1)?32767:1+i;
    values[length]=terminal;
    for(int i=length+1;i<20;i++)values[i]=-222;
    for(int i=0;i<RR_SEQUENCE_TABLE_COUNT;i++){SoundSequenceTable[i][0]=other;SoundSequenceTable[i][1]=other;}
}
static void run(int id,int restart) {
    memcpy(expected,state_page.bytes,sizeof(expected));
    s16 saved[20];memcpy(saved,values,sizeof(saved));
    int offset=model(id,restart);
    void *result=SoundQueueEnqueue(id,restart);
    void *want=offset<0?(void*)-1:(void *)(state_page.bytes+0x4DB8+offset);
    assert(result==want);
    assert(memcmp(state_page.bytes,expected,sizeof(expected))==0);
    assert(memcmp(values,saved,sizeof(saved))==0);
}
int main(int argc,char **argv) {
    assert(argc==2);
    const int directions[]={0,1,-1,INT_MIN,INT_MAX};
    const int active_values[]={0,1,-7,INT_MAX};
    const int restarts[]={0,1,-1,INT_MIN,INT_MAX};
    const int terminals[]={0,-1,-32768};
    if(!strcmp(argv[1],"queue")) {
        for(unsigned a=0;a<sizeof(active_values)/sizeof(*active_values);a++)
        for(int used=0;used<8;used++)
        for(unsigned r=0;r<sizeof(restarts)/sizeof(*restarts);r++)
        for(int length=0;length<16;length++) {
            if((!active_values[a]||restarts[r])&&length>7)continue;
            for(unsigned t=0;t<sizeof(terminals)/sizeof(*terminals);t++)
            for(unsigned d=0;d<sizeof(directions)/sizeof(*directions);d++) {
                prepare(active_values[a],used,length,terminals[t]);
                SoundSequenceSet=directions[d];
                SoundSequenceTable[2][SoundSequenceSet!=0]=values;
                run(2,restarts[r]);
            }
        }
    } else if(!strcmp(argv[1],"sentinel")) {
        for(unsigned d=0;d<sizeof(directions)/sizeof(*directions);d++)
        for(unsigned r=0;r<sizeof(restarts)/sizeof(*restarts);r++) {
            prepare(0,-100,4,0);SoundSequenceSet=directions[d];
            SoundSequenceTable[1][SoundSequenceSet!=0]=(s16*)-1;
            run(1,restarts[r]);
        }
    } else {
        /* Repeated appends must preserve older entries, and a rejection is inert. */
        prepare(0,7,2,0);SoundSequenceSet=0;SoundSequenceTable[0][0]=values;
        run(0,0);assert(SoundState.count19D4==2);
        run(0,0);assert(SoundState.count19D4==4);
        run(0,0);assert(SoundState.count19D4==6);
        run(0,0);assert(SoundState.count19D4==6);
        values[0]=0;run(0,0);
        assert(SoundState.active19D0==0&&SoundState.count19D4==6);
        values[0]=32767;values[1]=-1;run(0,0);
        assert(SoundState.active19D0==1&&SoundState.count19D4==1);
        run(0,-7);assert(SoundState.count19D4==1&&SoundState.mode19E0==-7);
    }
    return 0;
}
'''
        directory=Path(cls.temporary.name);source=directory/'queue.c'
        source.write_text(code);cls.executable=directory/'queue'
        result=subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                               '-fsanitize=undefined','-fno-strict-aliasing',
                               '-I',str(ROOT/'include'),str(source),'-o',str(cls.executable)],
                              capture_output=True,text=True)
        if result.returncode:raise AssertionError(result.stderr)

    def run_case(self,case):
        result=subprocess.run([str(self.executable),case],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stderr,'')

    def test_capacity_initialization_append_and_restart(self):self.run_case('queue')
    def test_missing_pointer_sentinel_preserves_state(self):self.run_case('sentinel')
    def test_repeated_appends_rejection_and_reactivation(self):self.run_case('repeat')


if __name__=='__main__':unittest.main()
