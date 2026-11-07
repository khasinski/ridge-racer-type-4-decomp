"""Actual two-phase CD stop with native callback records and IRQ-style hooks."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class CdReadStopTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        def header(name):
            text=(ROOT/'include/rr'/name).read_text()
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
        body=(ROOT/'tools/scripts/fixtures/legacy/func_8008ED54.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        assert body.count("extern RrLibCompletionCallback volatile CdReadSavedReadyCallback;\n")==1
        body=body.replace("extern RrLibCompletionCallback volatile CdReadSavedReadyCallback;\n", "")
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header('library.h') + header('cd_read_request.h') + header('cd_read_stop.h') + header('cd.h') + r'''
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
    u32 ticks[16],clear_mask,replace_mask,new_flags;
    int flip_flags,mutate,sync_clears;
} Plan;
typedef struct {
    int kind;
    s32 argument;
    RrLibCompletionCallback callback;
    const u8 *parameters;
    World snapshot;
} Entry;
static World actual,expected;
static Plan plan;
static Entry actual_log[32],expected_log[32];
static int actual_count,expected_count;
static volatile u32 callback_effect;
static void cb0(void){callback_effect^=1;}
static void cb1(void){callback_effect^=2;}
static void cb2(void){callback_effect^=4;}
static void cb3(void){callback_effect^=8;}
void cd_read_restore_sync(void){callback_effect^=16;}
#define CdReadState actual.state
#define CdReadSavedSyncCallback actual.state.restore.saved28
#define CdReadSavedReadyCallback actual.state.restore.saved2C
#define CD_read_dma_mode actual.flags
''' + body + r'''
enum {SYNC,CLOCK,SET0,SET1,SET2,COMMAND};
static s32 bits(u32 value){s32 result;memcpy(&result,&value,4);return result;}
static void record(int model,int kind,s32 argument,RrLibCompletionCallback callback,const u8 *parameters) {
    Entry entry;memset(&entry,0,sizeof(entry));
    entry.kind=kind;entry.argument=argument;entry.callback=callback;entry.parameters=parameters;
    memcpy(&entry.snapshot,model?&expected:&actual,sizeof(World));
    int index=model?expected_count++:actual_count++;assert(index<32);
    if(model)expected_log[index]=entry;else actual_log[index]=entry;
}
static void sync_transfer(int model,s32 argument) {
    World *world=model?&expected:&actual;
    record(model,SYNC,argument,0,0);assert(argument==0);
    if(plan.sync_clears)world->state.finished24=0;
    if(plan.mutate) {
        world->state.remaining14=0x11223344;
        world->flags^=0x80000001U;
    }
}
static s32 tick(int model,s32 argument) {
    World *world=model?&expected:&actual;
    record(model,CLOCK,argument,0,0);assert(argument==-1);
    int index=world->tick_index++;assert(index<16);
    if(plan.clear_mask&(1U<<index))world->state.finished24=0;
    if(plan.replace_mask&(1U<<index))world->state.restore.saved28=cb3;
    return bits(plan.ticks[index]);
}
static RrLibCompletionCallback setter(int model,int channel,RrLibCompletionCallback callback) {
    World *world=model?&expected:&actual;
    record(model,SET0+channel,0,callback,0);
    RrLibCompletionCallback previous=world->registered[channel];world->registered[channel]=callback;
    if(channel==1&&plan.flip_flags)world->flags=plan.new_flags;
    if(plan.mutate) {
        if(channel==0) {
            world->state.finished24=42;
            world->state.remaining14=0x5050;
            world->state.restore.saved2C=cb3;
            world->state.restore.saved30=cb0;
            world->state.buffer04=actual.buffers[1];
        }
        if(channel==1)world->state.restore.saved30=cb3;
    }
    return previous;
}
static s32 command(int model,s32 argument,const u8 *parameters) {
    World *world=model?&expected:&actual;
    record(model,COMMAND,argument,0,parameters);assert(argument==9&&!parameters);
    if(plan.mutate) {
        world->state.finished24=0;
        world->state.remaining14=-9;
        world->state.restore.saved28=cb3;
    }
    return -17;
}
void CdDataSync(s32 arg){sync_transfer(0,arg);}
s32 VSync(s32 arg){return tick(0,arg);}
RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback c){return setter(0,0,c);}
RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback c){return setter(0,1,c);}
RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback c){return setter(0,2,c);}
s32 CdControlF(s32 c,const u8 *p){return command(0,c,p);}
/* Separate algorithm: one generic deadline wait, then the stop specification.
 * A timeout returns true; a natural completion returns false. */
static int wait_model(void) {
    u32 baseline=(u32)tick(1,-1);
    if(!expected.state.finished24)return 0;
    for(;;) {
        u32 elapsed=(u32)tick(1,-1)-baseline;
        if(elapsed>120) {
            setter(1,0,expected.state.restore.saved28);
            expected.state.finished24=0;
            return 1;
        }
        if(!expected.state.finished24)return 0;
    }
}
static void reference(void) {
    if(expected.flags&1)sync_transfer(1,0);
    expected.state.remaining14=0;
    if(expected.state.finished24&&!wait_model())return;
    setter(1,1,expected.state.restore.saved2C);
    if(expected.flags&1)setter(1,2,expected.state.restore.saved30);
    setter(1,0,cd_read_restore_sync);
    command(1,9,0);
    expected.state.finished24=1;
    if(expected.state.finished24)wait_model();
}
static void timing(int scenario,u32 seed) {
    memset(&plan,0,sizeof(plan));
    for(int i=0;i<16;i++)plan.ticks[i]=seed+i*121U;
    switch(scenario) {
    case 0:case 3:plan.clear_mask=1;break;
    case 1:case 4:plan.ticks[1]=seed+120;plan.clear_mask=2;break;
    case 2:plan.ticks[1]=seed+121;plan.clear_mask=2;plan.replace_mask=2;break;
    case 5:plan.ticks[1]=seed+121;plan.clear_mask=4;plan.replace_mask=2;break;
    case 6:case 7:
        plan.ticks[1]=seed+121;plan.ticks[2]=seed+500;
        plan.ticks[3]=plan.ticks[2]+(scenario==6?120:121);
        plan.clear_mask=8;plan.replace_mask=10;break;
    case 8:case 9:
        plan.ticks[1]=seed+60;plan.ticks[2]=seed+(scenario==8?120:121);
        plan.clear_mask=4;plan.replace_mask=4;break;
    case 10:case 11:case 12:case 13:
        plan.ticks[0]=0xFFFFFFF0U;
        plan.ticks[1]=(scenario==10||scenario==12)?0x68:0x69;
        plan.clear_mask=(scenario==11?4:2);plan.replace_mask=2;break;
    case 14:case 15:plan.ticks[0]=100;plan.ticks[1]=99;plan.clear_mask=4;break;
    case 16:
        plan.ticks[0]=plan.ticks[2]=0x80000000U;
        plan.ticks[1]=plan.ticks[3]=0x7FFFFFFFU;plan.replace_mask=10;break;
    }
}
static void check(int scenario,s32 pending,u32 flags,int mutate,int flip,int sync_clear,u32 seed) {
    memset(&actual,(u8)seed,sizeof(actual));actual.tick_index=0;
    actual.state.finished24=pending;
    actual.state.sectors00=bits(seed*2654435761U);
    actual.state.buffer04=actual.buffers[0];
    actual.state.position08=actual.buffers[1];
    actual.state.restore.saved28=cb0;actual.state.restore.saved2C=cb1;actual.state.restore.saved30=cb2;
    actual.registered[0]=cb1;actual.registered[1]=cb2;actual.registered[2]=cb3;
    actual.flags=flags;timing(scenario,seed);
    plan.mutate=mutate;plan.flip_flags=flip;plan.new_flags=flags^1;plan.sync_clears=sync_clear;
    memcpy(&expected,&actual,sizeof(actual));actual_count=expected_count=0;
    reference();CdReadBreak();
    assert(actual_count==expected_count);
    assert(!memcmp(actual_log,expected_log,actual_count*sizeof(Entry)));
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
    assert(argc==2);assert(sizeof(void*)>4);
    size_t pair=offsetof(RrLibReadRequestState,restore)+offsetof(RrLibReadRestoreTail,saved2C);
    assert(pair==sizeof(RrLibReadRequestState)-sizeof(RrLibReadRestoreTail)+sizeof(RrLibCompletionCallback));
    assert(&RR_PAIR_PENDING(&actual.state.restore.saved2C)==&actual.state.finished24);
    assert((const void*)RR_PAIR_TAIL(&actual.state.restore.saved2C)==(const void*)&actual.state.restore);
    assert((uintptr_t)cb0>0xFFFFFFFFU&&(uintptr_t)actual.buffers>0xFFFFFFFFU);
    int mode=atoi(argv[1]);
    if(mode==0) {
        for(u32 i=0;i<65536;i++)check(0,0,i|(i*1717U<<16),0,0,0,i);
    } else if(mode==1) {
        const s32 pending[]={0,1,-1,(-2147483647-1),2147483647};
        for(int scenario=0;scenario<=16;scenario++)for(unsigned p=0;p<5;p++) {
            for(int flags=0;flags<4;flags++)check(scenario,pending[p],flags,0,0,0,0xFFFFFFC0U+flags);
        }
        for(u32 i=1;i<65536;i++)check(4,(s16)i,i*3571U,0,0,0,i);
    } else {
        for(int scenario=0;scenario<=16;scenario++)for(int flags=0;flags<4;flags++) {
            for(int flip=0;flip<2;flip++)for(int sync_clear=0;sync_clear<2;sync_clear++) {
                check(scenario,1,flags,1,flip,sync_clear,flags+7);
                check(scenario,-1,flags,1,flip,sync_clear,flags+17);
                check(scenario,0,flags,1,flip,sync_clear,flags+27);
            }
        }
    }
    return 0;
}
'''
        source=Path(cls.temporary.name)/'cd_read_stop.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_raw_flags_and_idle_stop(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_two_waits_completion_timeout_and_tick_wrap(self):
        subprocess.run([str(self.binary),'1'],check=True)

    def test_callback_mutations_and_command_publication_order(self):
        subprocess.run([str(self.binary),'2'],check=True)


if __name__=='__main__':
    unittest.main()
