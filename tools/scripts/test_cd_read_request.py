"""Actual CD request constructor with callback snapshots and wrapping tick plans."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class CdReadRequestTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/legacy/func_8008EEF4.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body)
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('library.h') + header('cd_read_request.h') + r'''
typedef struct {
    u8 prefix[32];
    RrLibReadRequestState state;
    u32 flags;
    RrLibCompletionCallback registered[3];
    u8 buffers[2][64];
    int tick_index;
    u8 suffix[32];
} World;
typedef struct {
    u32 ticks[8];
    int clear_on_tick,replace_on_tick,flip_flags,mutate;
    u32 new_flags;
    u8 status;
    s32 begin_result;
} Plan;
typedef struct {
    int kind;
    s32 argument;
    RrLibCompletionCallback callback;
    const u8 *parameters;
    u8 *result;
    World snapshot;
} Entry;
static World actual,expected;
static Plan plan;
static Entry actual_log[24],expected_log[24];
static int actual_count,expected_count;
static volatile u32 callback_effect;
static void cb0(void){callback_effect^=1;}
static void cb1(void){callback_effect^=2;}
static void cb2(void){callback_effect^=4;}
static void cb3(void){callback_effect^=8;}
#define CdReadState actual.state
#define CdReadStatePending actual.state.finished24
#define CdReadSavedSyncCallback actual.state.restore.saved28
#define CD_read_dma_mode actual.flags
''' + body + r'''
enum {CLOCK,SET0,SET1,SET2,STATUS,COMMAND,BEGIN};
static s32 bits(u32 value){s32 result;memcpy(&result,&value,4);return result;}
static void log_event(int model,int kind,s32 argument,RrLibCompletionCallback callback,
                      const u8 *parameters,u8 *result) {
    Entry entry;memset(&entry,0,sizeof(entry));
    entry.kind=kind;entry.argument=argument;entry.callback=callback;
    entry.parameters=parameters;entry.result=result;
    memcpy(&entry.snapshot,model?&expected:&actual,sizeof(World));
    int index=model?expected_count++:actual_count++;assert(index<24);
    if(model)expected_log[index]=entry;else actual_log[index]=entry;
}
static s32 tick(int model,s32 argument) {
    World *world=model?&expected:&actual;
    log_event(model,CLOCK,argument,0,0,0);assert(argument==-1);
    int index=world->tick_index++;assert(index<8);
    if(index==plan.clear_on_tick)world->state.finished24=0;
    if(index==plan.replace_on_tick)world->state.restore.saved28=cb3;
    return bits(plan.ticks[index]);
}
static RrLibCompletionCallback setter(int model,int channel,RrLibCompletionCallback callback) {
    World *world=model?&expected:&actual;
    log_event(model,SET0+channel,0,callback,0,0);
    RrLibCompletionCallback previous=world->registered[channel];
    world->registered[channel]=callback;
    if(channel==1&&plan.flip_flags)world->flags=plan.new_flags;
    if(plan.mutate) {
        if(channel==0) {
            world->state.buffer04=actual.buffers[1];
            world->state.sectors00=0x12345678;
            world->state.mode0C^=0x8040U;
            world->state.finished24=42;
            world->state.restore.saved28=cb2;
        }
        if(channel==1)world->state.restore.saved28=cb3;
        if(channel==2)world->state.start1C=0xDEADBEEFU;
    }
    return previous;
}
static u8 status(int model) {
    World *world=model?&expected:&actual;
    log_event(model,STATUS,0,0,0,0);
    if(plan.mutate)world->state.start1C=0xCAFEBABEU;
    return plan.status;
}
static s32 command(int model,s32 command,const u8 *parameters,u8 *result) {
    log_event(model,COMMAND,command,0,parameters,result);
    assert(command==9&&!parameters&&!result);return -17;
}
static s32 begin(int model,s32 argument) {
    World *world=model?&expected:&actual;
    log_event(model,BEGIN,argument,0,0,0);assert(argument==0);
    world->state.remaining14=plan.begin_result;
    if(plan.mutate)world->state.completed20^=0x87654321U;
    return plan.begin_result;
}
s32 VSync(s32 arg){return tick(0,arg);}
RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback c){return setter(0,0,c);}
RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback c){return setter(0,1,c);}
RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback c){return setter(0,2,c);}
u8 CdStatus(void){return status(0);}
s32 CdControlB(s32 c,const u8 *p,u8 *r){return command(0,c,p,r);}
s32 cd_read_retry(s32 arg){return begin(0,arg);}
static s32 reference(s32 sectors,u8 *buffer,u32 mode) {
    if(expected.state.finished24) {
        u32 started=(u32)tick(1,-1);
        if(expected.state.finished24) {
            for(;;) {
                u32 elapsed=(u32)tick(1,-1)-started;
                if(elapsed>120) {
                    setter(1,0,expected.state.restore.saved28);
                    expected.state.finished24=0;
                    break;
                }
                if(!expected.state.finished24)break;
            }
        }
    }
    expected.state.mode0C=mode;
    unsigned kind=expected.state.mode0C&0x30;
    expected.state.step10=kind==0?512:kind==32?585:582;
    expected.state.mode0C|=32;
    expected.state.buffer04=buffer;
    expected.state.sectors00=sectors;
    expected.state.restore.saved28=setter(1,0,0);
    expected.state.restore.saved2C=setter(1,1,0);
    if(expected.flags&1)expected.state.restore.saved30=setter(1,2,0);
    expected.state.start1C=(u32)tick(1,-1);
    if(status(1)&224)command(1,9,0,0);
    return begin(1,0)>0;
}
static void timing(int scenario,u32 seed) {
    memset(&plan,0,sizeof(plan));plan.clear_on_tick=-1;plan.replace_on_tick=-1;
    for(int i=0;i<8;i++)plan.ticks[i]=seed+i*60U;
    if(scenario==1)plan.clear_on_tick=0;
    if(scenario==2){plan.ticks[1]=seed+120U;plan.clear_on_tick=1;}
    if(scenario==3){plan.ticks[1]=seed+121U;plan.clear_on_tick=1;plan.replace_on_tick=1;}
    if(scenario==4){plan.clear_on_tick=2;}
    if(scenario==5){plan.ticks[0]=0xFFFFFFF0U;plan.ticks[1]=0x68;plan.clear_on_tick=1;}
    if(scenario==6){plan.ticks[0]=0xFFFFFFF0U;plan.ticks[1]=0x69;plan.replace_on_tick=1;}
    if(scenario==7){plan.ticks[0]=100;plan.ticks[1]=99;}
    if(scenario==8){plan.ticks[0]=0x80000000U;plan.ticks[1]=0x7FFFFFFFU;}
    if(scenario==9){plan.ticks[1]=seed;plan.ticks[2]=seed+121U;plan.replace_on_tick=2;}
}
static void check(int scenario,s32 pending,s32 sectors,u32 mode,u32 flags,u8 condition,
                  s32 begin_result,int mutate,int flip,u32 seed) {
    memset(&actual,(u8)seed,sizeof(actual));actual.tick_index=0;
    actual.state.finished24=scenario?pending:0;
    actual.state.buffer04=actual.buffers[1];
    actual.state.position08=actual.buffers[0];
    actual.state.restore.saved28=cb0;actual.state.restore.saved2C=cb1;
    actual.state.restore.saved30=cb2;
    actual.registered[0]=cb1;actual.registered[1]=cb2;actual.registered[2]=cb3;
    actual.flags=flags;
    timing(scenario,seed);
    plan.status=condition;plan.begin_result=begin_result;plan.mutate=mutate;
    plan.flip_flags=flip;plan.new_flags=flags^1U;
    memcpy(&expected,&actual,sizeof(actual));actual_count=expected_count=0;
    u8 *buffer=actual.buffers[seed&1];
    assert((uintptr_t)buffer>0xFFFFFFFFU);
    assert((uintptr_t)cb0>0xFFFFFFFFU);
    int wanted=reference(sectors,buffer,mode);
    int got=CdRead(sectors,buffer,mode);
    assert(got==wanted&&actual_count==expected_count);
    assert(!memcmp(actual_log,expected_log,actual_count*sizeof(Entry)));
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
    assert(argc==2);assert(sizeof(void*)>4);
    assert(offsetof(RrLibReadRequestState,restore)==sizeof(RrLibReadRequestState)-sizeof(RrLibReadRestoreTail));
    assert(offsetof(RrLibReadRequestState,restore)>0x28);
    int mode=atoi(argv[1]);
    if(mode==0) {
        for(u32 i=0;i<65536;i++) {
            check(0,0,bits(i*2654435761U),i|(i*1717U<<16),i*3571U,i,((i%3)-1),0,0,i);
        }
    } else if(mode==1) {
        const s32 pending[]={1,-1,(-2147483647-1),2147483647};
        const s32 results[]={(-2147483647-1),-1,0,1,2147483647};
        for(int scenario=1;scenario<=9;scenario++)for(unsigned p=0;p<4;p++) {
            for(unsigned r=0;r<5;r++)for(int shape=0;shape<4;shape++) {
                check(scenario,pending[p],-17,0x80000000U+shape*16,shape,0xE0,results[r],0,0,0xFFFFFFC0U+r);
            }
        }
        for(u32 i=1;i<65536;i++)check(2,(s16)i,7,0x80,1,0,1,0,0,i);
    } else {
        for(int scenario=0;scenario<=9;scenario++)for(int flags=0;flags<4;flags++) {
            for(int flip=0;flip<2;flip++)for(int condition=0;condition<256;condition++) {
                check(scenario,1,2147483647,0xFFFFFFFFU,flags,condition,-1,1,flip,condition);
            }
        }
    }
    return 0;
}
'''
        source=Path(cls.temporary.name)/'cd_read_request.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_modes_sector_words_and_status_bits(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_completion_timeout_boundaries_and_tick_wrap(self):
        subprocess.run([str(self.binary),'1'],check=True)

    def test_callback_snapshots_mutation_and_live_flag_selection(self):
        subprocess.run([str(self.binary),'2'],check=True)


if __name__=='__main__':
    unittest.main()
