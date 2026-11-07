"""Execute the actual CD begin body against an independent command/state model."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class CdReadBeginTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)

        def header(name):
            return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '',
                          (ROOT/'include/rr'/name).read_text(), flags=re.M)

        body = (ROOT/'tools/scripts/fixtures/legacy/func_8008EB6C.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$\d+"\)', '', body)
        body = re.sub(r',?\s*"\$\d+"', '', body)
        # Global PAL aliases address members of the same full native record.
        body = body.replace('extern volatile u32 DsSavedCdMode;', '')
        decoder = (ROOT/'tools/scripts/fixtures/legacy/func_8008C6A4.c').read_text()
        decoder = re.sub(r'^#include[^\n]*\n', '', decoder, flags=re.M)
        decoder = decoder.replace('CdPosToInt', 'actual_decode')
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "m2c_macros.h"
''' + header('library.h') + header('cd_read_request.h') + header('cd.h') + r'''
typedef struct {
    u8 prefix[32];
    RrLibReadRequestState state;
    u32 flags;
    RrLibCompletionCallback registered[3];
    u8 buffers[3][64];
    u8 packets[2][4];
    int tick_index, packet_index;
    u8 suffix[32];
} World;
typedef struct {
    u8 status;
    s32 mode, command2, command14;
    u32 ticks[2];
    int mutate;
    u32 clear_flags, install_flags;
} Plan;
typedef struct {
    int kind;
    s32 arg;
    RrLibCompletionCallback callback;
    const void *pointer;
    int mode_byte;
    World snapshot;
} Event;
static World actual, expected;
static Plan plan;
static Event actual_log[32], expected_log[32];
static int actual_count, expected_count;
static volatile u32 callback_effect;
static void cb0(void){callback_effect^=1;}
static void cb1(void){callback_effect^=2;}
static void cb2(void){callback_effect^=4;}
const char StrCdReadShellOpen[]="CdRead: Shell open...\n";
const char StrCdReadRetry[]="CdRead: retry...\n";
#define CdReadState actual.state
#define DsSavedCdMode actual.state.mode0C
#define CD_read_dma_mode actual.flags
''' + body + '\n' + decoder + r'''
enum {SET0,SET1,SET2,STATUS,CLOCK,LOG,CONTROL,RESET,MODE,PACKET,DECODE,TRANSFER};
static s32 bits(u32 word){s32 value;memcpy(&value,&word,4);return value;}
static void record(int model,int kind,s32 arg,RrLibCompletionCallback callback,
                   const void *pointer,int mode_byte) {
    Event event;memset(&event,0,sizeof(event));
    event.kind=kind;event.arg=arg;event.callback=callback;
    event.pointer=pointer;event.mode_byte=mode_byte;
    memcpy(&event.snapshot,model?&expected:&actual,sizeof(World));
    int index=model?expected_count++:actual_count++;assert(index<32);
    if(model)expected_log[index]=event;else actual_log[index]=event;
}
static RrLibCompletionCallback set(int model,int slot,RrLibCompletionCallback callback) {
    World *world=model?&expected:&actual;
    record(model,SET0+slot,0,callback,0,-1);
    RrLibCompletionCallback previous=world->registered[slot];
    world->registered[slot]=callback;
    if(plan.mutate) {
        if(slot==1&&!callback)world->flags=plan.clear_flags;
        if(slot==1&&callback) {
            world->flags=plan.install_flags;
            world->state.buffer04=actual.buffers[1];
            world->state.sectors00=123;
            world->state.finished24=42;
        }
        if(slot==2&&callback) {
            world->state.buffer04=actual.buffers[2];
            world->state.sectors00=-17;
        }
        world->state.start1C^=0x11223344U+slot;
    }
    return previous;
}
static u8 status(int model) {
    World *world=model?&expected:&actual;
    record(model,STATUS,0,0,0,-1);
    if(plan.mutate)world->state.mode0C^=0x12345678U;
    return plan.status;
}
static s32 clock_tick(int model,s32 argument) {
    World *world=model?&expected:&actual;
    record(model,CLOCK,argument,0,0,-1);assert(argument==-1);
    int index=world->tick_index++;assert(index<2);
    if(plan.mutate) {
        world->state.remaining14=bits(0x80000011U+index);
        world->state.retry18=0xABCDEF00U+index;
        world->state.start1C=0x76543210U+index;
    }
    return bits(plan.ticks[index]);
}
static s32 diagnostic(int model,const char *text) {
    record(model,LOG,0,0,text,-1);
    assert(text==StrCdReadShellOpen||text==StrCdReadRetry);return -17;
}
static u8 *packet(int model) {
    World *world=model?&expected:&actual;
    record(model,PACKET,0,0,0,-1);
    int index=world->packet_index++;assert(index<2);
    return actual.packets[index];
}
static s32 control(int model,s32 command,const u8 *parameters,u8 *result) {
    World *world=model?&expected:&actual;
    assert(!result);
    int byte=-1;const void *pointer=parameters;
    if(command==14) {
        assert(parameters&&(uintptr_t)parameters>UINT32_MAX);
        byte=*parameters;pointer=0; // Distinct stack locals carry the same byte.
    }
    record(model,CONTROL,command,0,pointer,byte);
    if(plan.mutate)world->state.completed20^=0xC0DEC0DEU;
    switch(command) {
        case 9: assert(!parameters);return -123;
        case 2: assert(parameters==actual.packets[0]);return plan.command2;
        case 14: return plan.command14;
        default: assert(0);return 0;
    }
}
static void reset(int model) {
    World *world=model?&expected:&actual;
    record(model,RESET,0,0,0,-1);
    if(plan.mutate)world->state.mode0C^=0x89ABCDEFU;
}
static s32 mode_query(int model) {
    World *world=model?&expected:&actual;
    record(model,MODE,0,0,0,-1);
    // The mode command must still use the byte captured before this helper.
    if(plan.mutate)world->state.mode0C^=0x000000FFU;
    return plan.mode;
}
static s32 decode(int model,void *pointer) {
    World *world=model?&expected:&actual;
    record(model,DECODE,0,0,pointer,-1);
    assert(pointer==actual.packets[world->packet_index-1]);
    const u8 *p=pointer;
    s32 value=model?((p[0]/16*10+p[0]%16)*60+p[1]/16*10+p[1]%16)*75
                    +p[2]/16*10+p[2]%16-150:actual_decode(pointer);
    if(plan.mutate)world->state.completed20=0xDEADBEEFU;
    return value;
}
static s32 transfer(int model,s32 command,const u8 *parameters) {
    World *world=model?&expected:&actual;
    record(model,TRANSFER,command,0,parameters,-1);
    assert(!parameters&&(command==1||command==6));
    if(plan.mutate) {
        world->state.buffer04=actual.buffers[0];
        world->state.sectors00=bits(0xFFFFFF03U);
        world->state.remaining14=0x1234;
    }
    return 0; // This result is deliberately ignored, including zero.
}
RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback c){return set(0,0,c);}
RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback c){return set(0,1,c);}
RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback c){return set(0,2,c);}
u8 CdStatus(void){return status(0);}
s32 VSync(s32 a){return clock_tick(0,a);}
s32 PsyqPuts(const char *s,...){return diagnostic(0,s);}
u8 *CdLastPos(void){return packet(0);}
s32 CdControl(s32 c,const u8 *p,u8 *r){return control(0,c,p,r);}
void CdFlush(void){reset(0);}
s32 CdMode(void){return mode_query(0);}
s32 CdPosToInt(void *p){return decode(0,p);}
s32 CdControlF(s32 c,const u8 *p){return transfer(0,c,p);}
void cb_read(s32 a,u8 *p){callback_effect^=(u32)a+(p!=0);}
void cb_data(void){callback_effect^=8;}
static s32 reference(s32 restart) {
    set(1,0,0);set(1,1,0);
    if(expected.flags&1)set(1,2,0);
    if(status(1)&16) {
        if(((u32)clock_tick(1,-1)%64)==0)diagnostic(1,StrCdReadShellOpen);
        transfer(1,1,0);
        u32 now=(u32)clock_tick(1,-1);
        expected.state.start1C=now;
        expected.state.remaining14=-1;
        return expected.state.remaining14;
    }
    if(restart) {
        diagnostic(1,StrCdReadRetry);
        control(1,9,0,0);
        if(!control(1,2,packet(1),0)) {
            expected.state.remaining14=-1;
            return expected.state.remaining14;
        }
    }
    reset(1);
    u8 mode=(u8)expected.state.mode0C;
    s32 current=mode_query(1);
    if(current!=(s32)mode||restart) {
        if(!control(1,14,&mode,0)) {
            expected.state.remaining14=-1;
            return expected.state.remaining14;
        }
    }
    expected.state.completed20=(u32)decode(1,packet(1));
    set(1,1,(RrLibCompletionCallback)cb_read);
    if(expected.flags&1)set(1,2,cb_data);
    expected.state.position08=expected.state.buffer04;
    transfer(1,6,0);
    expected.state.remaining14=expected.state.sectors00;
    u32 now=(u32)clock_tick(1,-1);
    expected.state.retry18=now;
    return expected.state.remaining14;
}
static void check(s32 restart,u8 status_byte,u32 mode,s32 queried,s32 cmd2,s32 cmd14,
                  u32 flags,int mutate,u32 seed) {
    memset(&actual,(u8)seed,sizeof(actual));
    actual.tick_index=actual.packet_index=0;
    actual.state.buffer04=actual.buffers[0];
    actual.state.position08=actual.buffers[2];
    actual.state.mode0C=mode;actual.state.sectors00=bits(seed*2654435761U);
    actual.state.restore.saved28=cb0;actual.state.restore.saved2C=cb1;
    actual.state.restore.saved30=cb2;
    actual.registered[0]=cb0;actual.registered[1]=cb1;actual.registered[2]=cb2;
    actual.flags=flags;
    for(int i=0;i<2;i++)for(int j=0;j<4;j++)actual.packets[i][j]=(u8)(seed+j*17+i*101);
    memset(&plan,0,sizeof(plan));plan.status=status_byte;plan.mode=queried;
    plan.command2=cmd2;plan.command14=cmd14;plan.mutate=mutate;
    plan.clear_flags=flags^1U;plan.install_flags=flags^3U;
    plan.ticks[0]=seed;plan.ticks[1]=seed^0xFFFFFFFFU;
    memcpy(&expected,&actual,sizeof(actual));actual_count=expected_count=0;
    assert((uintptr_t)actual.state.buffer04>UINT32_MAX);
    assert((uintptr_t)cb0>UINT32_MAX);
    s32 wanted=reference(restart);s32 got=cd_read_retry(restart);
    assert(got==wanted&&actual_count==expected_count);
    if(memcmp(actual_log,expected_log,actual_count*sizeof(Event))) {
        for(int i=0;i<actual_count;i++)if(memcmp(&actual_log[i],&expected_log[i],sizeof(Event))) {
            fprintf(stderr,"event %d kind=%d expected=%d restart=%d status=%u seed=%u\n",
                    i,actual_log[i].kind,expected_log[i].kind,restart,status_byte,seed);break;
        }
        assert(0);
    }
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
int main(int argc,char **argv) {
    assert(argc==2&&sizeof(void*)>4);
    const s32 extremes[]={0,1,-1,(-2147483647-1),2147483647};
    int mode=atoi(argv[1]);
    if(mode==0) {
        // All status bytes, raw mode bytes and both restart choices.
        for(u32 i=0;i<65536;i++)for(int retry=0;retry<2;retry++) {
            check(retry,i>>8,i|(i*1717U<<16),i&255,1,1,i*3571U,0,i);
        }
    } else if(mode==1) {
        for(unsigned r=0;r<5;r++)for(unsigned c2=0;c2<5;c2++)for(unsigned c14=0;c14<5;c14++) {
            for(unsigned q=0;q<5;q++)for(u32 flag=0;flag<4;flag++) {
                check(extremes[r],0,0xFFFF0080U,extremes[q],extremes[c2],extremes[c14],flag,0,flag*64);
            }
        }
        // Full ABI getter result must not be silently narrowed to a byte.
        for(u32 i=0;i<65536;i++)check(0,0,i,bits((i&255)|0x80000100U),1,1,1,0,i);
    } else {
        for(u32 i=0;i<65536;i++) {
            check(bits(i*2654435761U),i>>8,i*1717U,bits(i*3571U),
                  i%3-1,(i/3)%3-1,i,1,i);
            check(0,0,i*1717U,i&255,1,1,i,1,i);
        }
        for(unsigned r=0;r<5;r++)for(unsigned q=0;q<5;q++)for(u32 i=0;i<256;i++) {
            check(extremes[r],i,0x80000000U|i,extremes[q],1,1,i,1,i*64);
        }
    }
    return 0;
}
'''
        source = Path(cls.temporary.name)/'cd_read_begin.c'
        source.write_text(code)
        cls.binary = source.with_suffix('')
        subprocess.run(['cc', '-std=gnu99', '-O2', '-fno-strict-aliasing',
                        '-fsanitize=undefined', '-fno-sanitize-recover=all',
                        '-I', str(ROOT/'include'), str(source), '-o', str(cls.binary)], check=True)

    def test_status_and_mode_words(self):
        subprocess.run([str(self.binary), '0'], check=True)

    def test_restart_command_results_and_full_mode_abi(self):
        subprocess.run([str(self.binary), '1'], check=True)

    def test_live_callback_buffer_sector_and_clock_effects(self):
        subprocess.run([str(self.binary), '2'], check=True)


if __name__ == '__main__':
    unittest.main()
