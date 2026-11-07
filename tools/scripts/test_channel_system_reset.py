"""Actual channel-system reset, native callback mutations and IRQ bus observer."""
import ctypes as C
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest
from test_channel_callbacks import ChannelRecord

Hook=C.CFUNCTYPE(None,C.c_int32)


class Trace(C.Structure):
    _fields_=[('stage',C.c_int32),('enabled',C.c_int32),('arg0',C.c_size_t),('arg1',C.c_size_t),('pending',C.c_uint32),('mask',C.c_uint32),('counter0',C.c_int32),('counter1',C.c_int32)]


class Irq(C.Structure):
    _fields_=[('guard0',C.c_uint32),('pending',C.c_uint32),('mask',C.c_uint32),('guard1',C.c_uint32)]


class ChannelSystemResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        headers=''
        for name in ('channels.h','interrupts.h'):
            h=(root/'include/rr'/name).read_text()
            headers+=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',h,flags=re.M)
        body=(root/'tools/scripts/fixtures/views/func_80098044.c').read_text()
        body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
        body=re.sub(r'\s+__asm__\("\$[0-9]+"\)','',body)
        body=re.sub(r'"\$[0-9]+", ','',body)
        body=re.sub(r' : "\$[0-9]+"','',body)
        assert body.count('(s32 *)0x800B0000U')==1
        body=body.replace('(s32 *)0x800B0000U','(test_state+0x10000/4)')
        observed=body.replace('_padStartCom','observed_reset')
        replacements={'p->pending=0xFFFFFFFEU;':'test_ack(p,0xFFFFFFFEU);','mask=p->mask;':'mask=test_read_mask(p);','*(u32 *)((u8 *)p+RR_FIELD_OFFSET(RrInterruptRegisters,mask))=mask|1;':'test_write_mask(p,mask|1);'}
        for old,new in replacements.items():
            assert observed.count(old)==1
            observed=observed.replace(old,new)
        code='''#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
#define RR_FIELD_OFFSET(type,field) offsetof(type,field)
'''+headers+'''
s32 test_state[0x10000/4+1];
#define PadComActive test_state[0xBCA0/4]
unsigned long record_size=sizeof(RrChannelRecordF0);
RrChannelRecordF0 channels_a[2],channels_b[2];
RrChannelRecordF0 *PadChannelRecords=channels_a;
RrChannelUpdateCallback PadResetChannelFn;
s32 PadPortTimeoutCounts[2];u32 PadIntQueueEntry[4];
struct {u32 guard0;RrInterruptRegisters irq;u32 guard1;} irq_units[2];
RrInterruptRegisters *I_STAT_Pad=&irq_units[0].irq;
struct Trace {s32 stage,enabled;uintptr_t arg0,arg1;u32 pending,mask;s32 counter0,counter1;} trace[64];
s32 trace_count,op_count,op_kind[8],op_unit[8];u32 op_value[8];
void (*on_stage)(s32);
void stage(s32 kind,uintptr_t a,uintptr_t b) {
    struct Trace *t=&trace[trace_count++];
    t->stage=kind;t->enabled=PadComActive;t->arg0=a;t->arg1=b;
    t->pending=I_STAT_Pad->pending;t->mask=I_STAT_Pad->mask;
    t->counter0=PadPortTimeoutCounts[0];t->counter1=PadPortTimeoutCounts[1];
    if(on_stage)on_stage(kind);
}
s32 EnterCriticalSection(void) {stage(0,0,0);return -123;}
void ExitCriticalSection(void) {stage(4,0,0);}
s32 SysDeqIntRP(s32 n,void *q) {stage(1,n,(uintptr_t)q);return -1;}
s32 SysEnqIntRP(s32 n,void *q) {stage(2,n,(uintptr_t)q);return -2;}
s32 ChangeClearRCnt(s32 n,s32 mode) {stage(3,n,mode);return -3;}
void callback_a(void *p) {stage(5,(uintptr_t)p,0);}
void callback_b(void *p) {stage(6,(uintptr_t)p,0);}
void op(s32 kind,RrInterruptRegisters *p,u32 value) {
    s32 i=op_count++;op_kind[i]=kind;op_unit[i]=p==&irq_units[1].irq;op_value[i]=value;
}
void test_ack(RrInterruptRegisters *p,u32 value) {op(0,p,value);p->pending&=value;}
u32 test_read_mask(RrInterruptRegisters *p) {u32 value=p->mask;op(1,p,value);return value;}
void test_write_mask(RrInterruptRegisters *p,u32 value) {op(2,p,value);p->mask=value;}
'''+body+observed
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'))
        for name in ('_padStartCom','observed_reset'):
            fn=getattr(cls.lib,name);fn.argtypes=[];fn.restype=None
        cls.state=(C.c_int32*(0x10000//4+1)).in_dll(cls.lib,'test_state')
        cls.record_size=C.sizeof(ChannelRecord)
        assert C.c_ulong.in_dll(cls.lib,'record_size').value==cls.record_size
        cls.a=(C.c_uint8*(2*cls.record_size)).in_dll(cls.lib,'channels_a');cls.b=(C.c_uint8*(2*cls.record_size)).in_dll(cls.lib,'channels_b')
        cls.units=(Irq*2).in_dll(cls.lib,'irq_units')
        cls.base=C.c_void_p.in_dll(cls.lib,'PadChannelRecords');cls.callback=C.c_void_p.in_dll(cls.lib,'PadResetChannelFn')
        cls.irq=C.c_void_p.in_dll(cls.lib,'I_STAT_Pad');cls.hook=C.c_void_p.in_dll(cls.lib,'on_stage')
        cls.counters=(C.c_int32*2).in_dll(cls.lib,'PadPortTimeoutCounts');cls.queue=(C.c_uint32*4).in_dll(cls.lib,'PadIntQueueEntry')
        cls.trace=(Trace*64).in_dll(cls.lib,'trace')
        cls.cb_a=C.cast(cls.lib.callback_a,C.c_void_p).value;cls.cb_b=C.cast(cls.lib.callback_b,C.c_void_p).value

    def integer(self,name):return C.c_int32.in_dll(self.lib,name)
    def enabled(self):return self.state[0xBCA0//4]
    def stages(self):return [self.trace[i].stage for i in range(self.integer('trace_count').value)]
    def install(self,fn):
        hook=Hook(fn);self.retained.append(hook);self.hook.value=C.cast(hook,C.c_void_p).value

    def setUp(self):
        for i in range(len(self.state)):self.state[i]=0x12345678
        C.memset(C.addressof(self.a),0xA5,C.sizeof(self.a));C.memset(C.addressof(self.b),0x5A,C.sizeof(self.b))
        self.base.value=C.addressof(self.a);self.callback.value=self.cb_a
        for i in range(2):self.units[i]=Irq(0xABCDEF01,0xA5A50001+i,0x12340002+i,0x10FEDCBA)
        self.irq.value=C.addressof(self.units[0])+Irq.pending.offset
        self.counters[:]=[-11,123];self.queue[:]=[1,2,3,4]
        self.integer('trace_count').value=0;self.integer('op_count').value=0;self.hook.value=None;self.retained=[]

    def test_call_order_state_publication_and_ram_body_guards(self):
        state_before=bytearray(self.state);a_before=bytes(self.a);b_before=bytes(self.b);queue_before=bytes(self.queue)
        self.lib._padStartCom()
        self.assertEqual(self.stages(),[0,1,2,3,4,5,5])
        self.assertTrue(all(self.trace[i].enabled==0 for i in range(7)))
        self.assertEqual([(self.trace[i].arg0,self.trace[i].arg1) for i in (1,2)],[(2,C.addressof(self.queue))]*2)
        self.assertEqual((self.trace[3].arg0,self.trace[3].arg1),(3,0))
        self.assertEqual([self.trace[i].arg0 for i in (5,6)],[C.addressof(self.a),C.addressof(self.a)+self.record_size])
        self.assertEqual((self.units[0].pending,self.units[0].mask),(0xFFFFFFFE,0x12340003))
        self.assertEqual((self.enabled(),list(self.counters)),(1,[0,0]))
        state_before[0xBCA0:0xBCA4]=(1).to_bytes(4,'little')
        self.assertEqual(bytes(self.state),bytes(state_before));self.assertEqual(bytes(self.a),a_before);self.assertEqual(bytes(self.b),b_before);self.assertEqual(bytes(self.queue),queue_before)
        for unit in self.units:self.assertEqual((unit.guard0,unit.guard1),(0xABCDEF01,0x10FEDCBA))

    def test_fresh_irq_pointer_and_dynamic_second_callback_and_base(self):
        def mutate(stage):
            if stage==2:self.irq.value=C.addressof(self.units[1])+Irq.pending.offset
            if stage==5:
                self.callback.value=self.cb_b;self.base.value=C.addressof(self.b)
                self.counters[:]=[333,444];self.state[0xBCA0//4]=-7
        self.install(mutate)
        self.lib._padStartCom()
        self.assertEqual(self.stages(),[0,1,2,3,4,5,6])
        self.assertEqual((self.units[0].pending,self.units[0].mask),(0xA5A50001,0x12340002))
        self.assertEqual((self.units[1].pending,self.units[1].mask),(0xFFFFFFFE,0x12340003))
        self.assertEqual((self.trace[5].arg0,self.trace[6].arg0),(C.addressof(self.a),C.addressof(self.b)+self.record_size))
        self.assertEqual((self.trace[6].enabled,self.trace[6].counter0,self.trace[6].counter1),(-7,333,444))
        self.assertEqual((self.enabled(),list(self.counters)),(1,[0,0]))

    def test_mmio_ack_mask_order_and_preservation_of_later_changes(self):
        rng=random.Random(0x98044)
        for _ in range(500):
            self.integer('trace_count').value=0;self.integer('op_count').value=0
            pending=rng.getrandbits(32);mask=rng.getrandbits(32)
            self.units[0].pending=pending;self.units[0].mask=mask
            self.lib.observed_reset()
            self.assertEqual(self.integer('op_count').value,3)
            self.assertEqual(list((C.c_int32*8).in_dll(self.lib,'op_kind'))[:3],[0,1,2])
            self.assertEqual(list((C.c_int32*8).in_dll(self.lib,'op_unit'))[:3],[0,0,0])
            self.assertEqual(list((C.c_uint32*8).in_dll(self.lib,'op_value'))[:3],[0xFFFFFFFE,mask,mask|1])
            self.assertEqual((self.units[0].pending,self.units[0].mask),(pending&0xFFFFFFFE,mask|1))
        self.install(lambda stage:setattr(self.units[0],'mask',0x87654321) if stage==3 else None)
        self.integer('trace_count').value=0;self.integer('op_count').value=0
        self.lib.observed_reset();self.assertEqual(self.units[0].mask,0x87654321)

    def test_reentry_does_not_invent_an_enable_guard(self):
        nested=False
        def reenter(stage):
            nonlocal nested
            if stage==5 and not nested:
                nested=True;self.lib._padStartCom()
        self.install(reenter);self.lib._padStartCom()
        self.assertEqual(self.stages(),[0,1,2,3,4,5,0,1,2,3,4,5,5,5])
        self.assertEqual(self.trace[13].enabled,1)
        self.assertEqual((self.enabled(),list(self.counters)),(1,[0,0]))
