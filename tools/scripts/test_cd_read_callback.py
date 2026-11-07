"""Actual CD data callback with an independent transfer/timer/notification model."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class CdReadCallbackTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary=TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)

        def header(name):
            s=(ROOT/'include/rr'/name).read_text()
            s=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',s,flags=re.M)
            return re.sub(r'extern[^;]*;','',s)

        body=(ROOT/'tools/scripts/fixtures/legacy/func_8008E828.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$\d+"\)','',body)
        body=re.sub(r',?\s*"\$\d+"','',body)
        body,n=re.subn(r'\(u8\s*\*\)0x800A0000U','native_page.bytes',body)
        assert n==1, 'native callback page must replace exactly one PAL page'
        decoder=(ROOT/'tools/scripts/fixtures/legacy/func_8008C6A4.c').read_text()
        decoder=re.sub(r'^#include[^\n]*\n','',decoder,flags=re.M)
        decoder=decoder.replace('CdPosToInt','actual_decode')
        code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "m2c_macros.h"
''' + ''.join(header(n) for n in ['library.h','cd_read_request.h','cd_read_callback.h','cd.h']) + r'''
typedef struct {
    u8 guard0[32];
    RrLibReadRequestState state;
    u32 flags;
    RrCdDataNotify notify;
    RrLibCompletionCallback registered[3];
    u8 buffers[2][4096];
    int tick_index;
    u8 guard1[32];
} World;
typedef struct {
    u32 ticks[3],start_after[3];
    s32 remaining_after[3];
    s32 restart_remaining,command_remaining;
    u32 restart_start;
    unsigned mutations;
    u32 header_flags,sync_flags,stop_flags;
    int match,final_notify;
    u8 header[12];
} Plan;
typedef struct {
    int kind,channel;
    s32 argument;
    u32 words;
    const void *pointer;
    RrLibCompletionCallback callback;
    u8 header[12];
    World snapshot;
} Event;
static World actual,expected;
static Plan plan;
static Event actual_log[32],expected_log[32];
static int actual_count,expected_count;
static u8 *input_data;
static union {void *alignment;u8 bytes[0x6400];} native_page;
static volatile u32 completion_effect;
static void completion0(void){completion_effect^=1;}
static void completion1(void){completion_effect^=2;}
static void completion2(void){completion_effect^=4;}
static void notification0(s32,u8*),notification1(s32,u8*);
const char CdReadSectorErrorMsg[]="CdRead: sector mismatch\n";
#define CdReadState actual.state
#define D_800A67B8 (*(RrCdPositionView*)&actual.state.position08)
#define D_800A67D0 actual.state.completed20
#define CD_read_dma_mode actual.flags
''' + body + '\n' + decoder + r'''
enum {SET,READ_IMMEDIATE,READ_DEFERRED,SYNC,DECODE,DIAGNOSTIC,CLOCK,RESTART,COMMAND,NOTIFY};
static s32 bits(u32 word){s32 s;memcpy(&s,&word,4);return s;}
static int in_buffers(const void *pointer) {
    uintptr_t offset=(uintptr_t)pointer-(uintptr_t)actual.buffers;
    return offset<sizeof(actual.buffers);
}
static void *mapped(World *world,const void *pointer) {
    uintptr_t offset=(uintptr_t)pointer-(uintptr_t)&actual;
    assert(offset<sizeof(actual));return (u8*)world+offset;
}
static void record(int model,int kind,int channel,s32 argument,u32 words,
                   const void *pointer,RrLibCompletionCallback callback,const u8 *header) {
    Event event;memset(&event,0,sizeof(event));
    event.kind=kind;event.channel=channel;event.argument=argument;event.words=words;
    event.pointer=pointer;event.callback=callback;
    if(header)memcpy(event.header,header,12);
    memcpy(&event.snapshot,model?&expected:&actual,sizeof(World));
    int index=model?expected_count++:actual_count++;assert(index<32);
    if(model)expected_log[index]=event;else actual_log[index]=event;
}
static void set_notify(int model,RrCdDataNotify callback) {
    World *w=model?&expected:&actual;w->notify=callback;
    if(!model)*(RrCdDataNotify*)(native_page.bytes+0x63F0)=callback;
}
static RrLibCompletionCallback set(int model,int channel,RrLibCompletionCallback callback) {
    World *w=model?&expected:&actual;
    record(model,SET,channel,0,0,0,callback,0);
    RrLibCompletionCallback old=w->registered[channel];w->registered[channel]=callback;
    if(plan.mutations&1) {
        if(channel==2&&callback==cb_data)w->flags=plan.header_flags;
        w->state.restore.notify34=actual.buffers[1]+3000;
    }
    if(plan.mutations&8) {
        if(channel==1) {
            w->flags=plan.stop_flags;
            w->state.restore.saved30=completion2;
        }
        if(channel==0)w->state.finished24=42;
    }
    return old;
}
static s32 read_data(int model,int deferred,u8 *pointer,u32 words) {
    World *w=model?&expected:&actual;
    int payload=in_buffers(pointer);
    assert((uintptr_t)pointer>UINT32_MAX);
    record(model,deferred?READ_DEFERRED:READ_IMMEDIATE,payload,0,words,
           payload?pointer:0,0,0);
    if(!payload) {
        assert(words==3&&((uintptr_t)pointer&3)==0);
        memcpy(pointer,plan.header,12);
        if(plan.mutations&1) {
            w->state.position08=actual.buffers[1]+16;
            w->state.step10=585;
        }
    } else {
        assert(words<=585);
        // Exercise writes to the exact supplied full pointer, not a target handle.
        memset(mapped(w,pointer),deferred?0xA5:0x5A,words*4U);
        if(plan.mutations&2) {
            w->state.position08=actual.buffers[1]+32;
            w->state.step10=4;
            w->state.remaining14=(-2147483647-1);
            w->state.completed20=0xFFFFFFFFU;
        }
    }
    return -17; // Caller deliberately ignores both transfer results.
}
static void sync_transfer(int model,s32 mode) {
    World *w=model?&expected:&actual;
    record(model,SYNC,0,mode,0,0,0,0);assert(mode==0);
    if(plan.mutations&1)w->flags=plan.sync_flags;
}
static s32 decode(int model,void *pointer) {
    World *w=model?&expected:&actual;
    record(model,DECODE,0,0,0,0,0,pointer);
    const u8 *p=pointer;
    s32 value=model?((p[0]/16*10+p[0]%16)*60+p[1]/16*10+p[1]%16)*75
                     +p[2]/16*10+p[2]%16-150:actual_decode(pointer);
    if(plan.mutations&32)w->state.completed20=(u32)value+(plan.match?0:1);
    return value;
}
static s32 diagnostic(int model,const char *text) {
    record(model,DIAGNOSTIC,0,0,0,text,0,0);assert(text==CdReadSectorErrorMsg);return -1;
}
static s32 clock_tick(int model,s32 mode) {
    World *w=model?&expected:&actual;
    record(model,CLOCK,0,mode,0,0,0,0);assert(mode==-1);
    int i=w->tick_index++;assert(i<3);
    if(plan.mutations&4) {
        w->state.start1C=plan.start_after[i];
        w->state.remaining14=plan.remaining_after[i];
        w->state.restore.notify34=actual.buffers[1]+3000;
    }
    return bits(plan.ticks[i]);
}
static s32 restart(int model,s32 mode) {
    World *w=model?&expected:&actual;
    record(model,RESTART,0,mode,0,0,0,0);assert(mode==1);
    w->state.remaining14=plan.restart_remaining;
    w->state.start1C=plan.restart_start;
    return -1; // Only the helper's state effects matter here.
}
static s32 command(int model,s32 command,const u8 *parameters) {
    World *w=model?&expected:&actual;
    record(model,COMMAND,0,command,0,parameters,0,0);assert(command==9&&!parameters);
    if(plan.mutations&16) {
        w->state.remaining14=plan.command_remaining;
        w->state.finished24=42;
        set_notify(model,plan.final_notify==0?0:plan.final_notify==1?notification0:notification1);
    }
    return 0;
}
static void notify(int model,int channel,s32 event,u8 *pointer) {
    World *w=model?&expected:&actual;
    record(model,NOTIFY,channel,event,0,pointer,0,0);
    assert(pointer==input_data);
    assert(w->state.finished24==1);
    w->state.completed20^=0x11223344U;
}
static void notification0(s32 event,u8 *pointer){notify(0,0,event,pointer);}
static void notification1(s32 event,u8 *pointer){notify(0,1,event,pointer);}
RrLibCompletionCallback CdSyncCallback(RrLibCompletionCallback c){return set(0,0,c);}
RrLibCompletionCallback CdReadyCallback(RrLibCompletionCallback c){return set(0,1,c);}
RrLibCompletionCallback CdDataCallback(RrLibCompletionCallback c){return set(0,2,c);}
s32 CdGetSector(u8 *p,u32 n){return read_data(0,0,p,n);}
s32 CdGetSector2(u8 *p,u32 n){return read_data(0,1,p,n);}
void CdDataSync(s32 mode){sync_transfer(0,mode);}
s32 CdPosToInt(void *p){return decode(0,p);}
s32 PsyqPuts(const char *s,...){return diagnostic(0,s);}
s32 VSync(s32 mode){return clock_tick(0,mode);}
s32 cd_read_retry(s32 mode){return restart(0,mode);}
s32 CdControlF(s32 c,const u8 *p){return command(0,c,p);}
void cb_data(void){completion_effect^=8;}
void cd_read_restore_sync(void){completion_effect^=16;}
static void reference(s32 event,u8 *data) {
    expected.state.restore.notify34=data;
    if((u32)event%256!=1)expected.state.remaining14=-1;
    else if(expected.state.remaining14>0) {
        if(expected.state.step10==512) {
            u8 header[12];
            if(expected.flags&1) {
                set(1,2,0);
                read_data(1,1,header,3);
                sync_transfer(1,0);
                set(1,2,cb_data);
            } else read_data(1,0,header,3);
            s32 sector=decode(1,header);
            if((u32)sector!=expected.state.completed20) {
                diagnostic(1,CdReadSectorErrorMsg);
                expected.state.remaining14=-1;
            }
        }
        int deferred=(expected.flags&1)!=0;
        read_data(1,deferred,expected.state.position08,expected.state.step10);
        if(!deferred) {
            expected.state.position08+=expected.state.step10*4U;
            expected.state.remaining14=bits((u32)expected.state.remaining14-1U);
            expected.state.completed20=(expected.state.completed20+1U);
        }
    }
    expected.state.retry18=(u32)clock_tick(1,-1);
    if(expected.state.remaining14<0)restart(1,1);
    s32 now=clock_tick(1,-1);
    if(now>bits(expected.state.start1C+1200U))expected.state.remaining14=-1;
    int finished=expected.state.remaining14==0;
    if(!finished) {
        now=clock_tick(1,-1);
        finished=now>bits(expected.state.start1C+1200U);
    }
    if(finished) {
        set(1,1,expected.state.restore.saved2C);
        if(expected.flags&1)set(1,2,expected.state.restore.saved30);
        set(1,0,cd_read_restore_sync);
        command(1,9,0);
        RrCdDataNotify callback=expected.notify;
        expected.state.finished24=1;
        if(callback)notify(1,callback==notification0?0:1,
                           expected.state.remaining14==0?2:5,data);
    }
}
static void check(s32 event,s32 remaining,u32 step,u32 flags,u32 start,u32 seed) {
    memset(&actual,(u8)seed,sizeof(actual));
    actual.state.position08=actual.buffers[0]+16;
    actual.state.buffer04=actual.buffers[0];
    actual.state.restore.notify34=actual.buffers[1]+3000;
    actual.state.restore.saved28=completion0;actual.state.restore.saved2C=completion1;
    actual.state.restore.saved30=completion0;
    actual.registered[0]=completion0;actual.registered[1]=completion1;actual.registered[2]=completion2;
    actual.state.remaining14=remaining;actual.state.step10=step;actual.state.start1C=start;
    actual.state.completed20=(u32)actual_decode(plan.header)+(plan.match?0:1);
    actual.flags=flags;actual.tick_index=0;
    actual.notify=(seed%3)==0?0:(seed%3)==1?notification0:notification1;
    *(RrCdDataNotify*)(native_page.bytes+0x63F0)=actual.notify;
    memcpy(&expected,&actual,sizeof(actual));actual_count=expected_count=0;
    assert((uintptr_t)actual.buffers>UINT32_MAX&&(uintptr_t)notification0>UINT32_MAX);
    input_data=(seed&4)?0:actual.buffers[0]+3000;
    reference(event,input_data);
    cb_read(event,input_data);
    assert(actual_count==expected_count);
    for(int i=0;i<actual_count;i++)if(memcmp(&actual_log[i],&expected_log[i],sizeof(Event))) {
        fprintf(stderr,"event %d kind %d/%d eventword=%d pending=%d step=%u flags=%u seed=%u mutations=%u\n",
                i,actual_log[i].kind,expected_log[i].kind,event,remaining,step,flags,seed,plan.mutations);
        assert(0);
    }
    assert(!memcmp(&actual,&expected,sizeof(actual)));
}
static void configure(u32 seed) {
    memset(&plan,0,sizeof(plan));plan.match=seed&1;
    for(int i=0;i<12;i++)plan.header[i]=(u8)(seed+i*17);
    for(int i=0;i<3;i++) {
        plan.ticks[i]=seed+i*17U;
        plan.start_after[i]=seed+i*31U;
        plan.remaining_after[i]=(i==0?-1:i==1?1:0);
    }
    plan.restart_remaining=1;plan.restart_start=seed;
    plan.header_flags=seed^1U;plan.sync_flags=seed^3U;plan.stop_flags=seed;
    plan.command_remaining=seed&1?-7:0;plan.final_notify=seed%3;
}
int main(int argc,char **argv) {
    assert(argc==2&&sizeof(void*)>4);
    assert(RR_READ_COMPLETED_OWNER_OFFSET==offsetof(RrLibReadRequestState,completed20));
    assert(RR_READ_POSITION_OWNER_OFFSET==offsetof(RrLibReadRequestState,position08));
    assert(offsetof(RrCdPositionView,step10)==offsetof(RrLibReadRequestState,step10)-offsetof(RrLibReadRequestState,position08));
    const s32 pending[]={(-2147483647-1),-1,0,1,2147483647};
    const u32 steps[]={0,4,512,582,585};
    const u32 times[]={0,1,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU};
    int group=atoi(argv[1]);
    if(group==0) {
        for(u32 event=0;event<256;event++)for(unsigned r=0;r<5;r++)for(unsigned s=0;s<5;s++)for(u32 f=0;f<4;f++) {
            configure(event*31U+r+s);check(bits(event|(event*1717U<<8)),pending[r],steps[s],f,0,event);
        }
    } else if(group==1) {
        for(unsigned start=0;start<5;start++)for(unsigned t0=0;t0<5;t0++)for(int delta=-1;delta<=1;delta++) {
            for(unsigned t2=0;t2<5;t2++)for(unsigned r=0;r<5;r++) {
                configure(start+t0+t2+r);plan.ticks[0]=times[t0];
                plan.ticks[1]=times[start]+1200U+(u32)delta;plan.ticks[2]=times[t2];
                plan.restart_start=times[start];plan.restart_remaining=pending[r];
                check(1,pending[r],4,0,times[start],r+t2);
            }
        }
        // First expiry followed by a non-expired second sample must not stop.
        configure(0);plan.ticks[0]=0;plan.ticks[1]=1201;plan.ticks[2]=1200;
        check(1,0,4,0,0,0);
    } else {
        for(unsigned mutations=0;mutations<64;mutations++)for(u32 seed=0;seed<24;seed++) {
            for(unsigned r=0;r<5;r++)for(unsigned s=2;s<5;s++) {
                configure(seed);plan.mutations=mutations;
                plan.restart_remaining=pending[r];plan.restart_start=seed^0x80000000U;
                plan.start_after[1]=seed&1?0x7FFFFFFFU:0;
                plan.start_after[2]=seed&1?0:0xFFFFFFFFU;
                plan.remaining_after[0]=pending[(r+1)%5];
                plan.ticks[1]=1200U+seed%3;plan.ticks[2]=1200U+seed%5;
                check(seed&2?1:0x80000001U,pending[r],steps[s],seed,0,seed);
            }
        }
    }
    return 0;
}
'''
        source=Path(cls.temporary.name)/'cd_read_callback.c'
        source.write_text(code)
        cls.binary=source.with_suffix('')
        subprocess.run(['cc','-std=gnu99','-O2','-fno-strict-aliasing',
                        '-fsanitize=undefined','-fno-sanitize-recover=all',
                        '-I',str(ROOT/'include'),str(source),'-o',str(cls.binary)],check=True)

    def test_event_low_byte_pending_extremes_and_transfer_modes(self):
        subprocess.run([str(self.binary),'0'],check=True)

    def test_signed_timeout_boundaries_wrap_and_repeated_samples(self):
        subprocess.run([str(self.binary),'1'],check=True)

    def test_fresh_fields_callback_restore_and_full_pointer_notification(self):
        subprocess.run([str(self.binary),'2'],check=True)


if __name__=='__main__':
    unittest.main()
