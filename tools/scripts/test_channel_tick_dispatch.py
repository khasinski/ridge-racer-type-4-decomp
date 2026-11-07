"""Actual channel tick dispatcher with real native records and mutable callbacks."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest
from test_cd_disc_probe import native_body
from test_channel_callbacks import ChannelRecord

Hook=C.CFUNCTYPE(None,C.c_void_p)
ErrorHook=C.CFUNCTYPE(None)


class Port(C.Structure):
    _fields_=[('data',C.c_uint8),('pad01',C.c_uint8*3),('status',C.c_uint16),('pad06',C.c_uint8*2),('mode',C.c_uint16),('control',C.c_uint16),('pad0C',C.c_uint8*2),('baud',C.c_uint16)]


class ChannelTickDispatchTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        header=''
        for name in ('channels.h','serial.h'):
            h=(root/'include/rr'/name).read_text()
            header+=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',h,flags=re.M)+'\n'
        body=native_body((root/'tools/scripts/fixtures/views/func_80097EA0.c').read_text())
        assert body.count('(s32 *)0x800B0000U')==1
        body=body.replace('(s32 *)0x800B0000U','ram+0x4330/4')
        observed=body.replace('_padIntPad','observed_tick')
        for old,new in {'port->control&2':'test_read(port)&2','*(u16 *)&port->control=0;':'test_write(port,0,0);','PadSio0Base->baud=0x88;':'test_write(PadSio0Base,1,0x88);'}.items():
            assert observed.count(old)==1
            observed=observed.replace(old,new)
        code='''#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8;typedef int8_t s8;typedef uint16_t u16;typedef int16_t s16;typedef uint32_t u32;typedef int32_t s32;
#define RR_FIELD_OFFSET(type,field) offsetof(type,field)
'''+header+'''
s32 ram[0x4330/4+2],PadFirstChannel,PadLastChannel,PadComActive,PadActiveChannel,PadRecvStage,PadActuatorPowerUsed,PadPortTimeoutCounts[2];
unsigned long record_size=sizeof(RrChannelRecordF0);
RrChannelRecordF0 records[16];RrChannelRecordF0 *PadChannelRecords=&records[4];
RrSerialPortRegisters ports[2];RrSerialPortRegisters *PadSio0Base=&ports[0];
s32 begin_result,events,event_kind[32],event_index[32],event_stage[32],event_counter[32],error_codes[32];void *event_pointer[32];
void (*on_begin)(void *),(*on_step)(void *);void (*on_error)(void),(*on_read)(void);
s32 io_count,io_kind[4],io_port[4];u16 io_value[4];
void record(s32 kind,void *p){s32 i=events++;event_kind[i]=kind;event_pointer[i]=p;event_index[i]=PadActiveChannel;event_stage[i]=PadRecvStage;event_counter[i]=PadActuatorPowerUsed;}
void error_a(s32 code){s32 i=events;record(1,0);error_codes[i]=code;if(on_error)on_error();}
void error_b(s32 code){s32 i=events;record(2,0);error_codes[i]=code;if(on_error)on_error();}
RrChannelErrorCallback PadAdvanceChannelFn=error_a;
s32 _padInitSioMode(RrChannelRecordF0 *p){record(0,p);if(on_begin)on_begin(p);return begin_result;}
void _padSioStageStep(RrChannelRecordF0 *p){record(3,p);if(on_step)on_step(p);else PadActiveChannel=(u32)PadActiveChannel+1;}
u16 test_read(RrSerialPortRegisters *p){s32 i=io_count++;u16 value=p->control;io_kind[i]=0;io_port[i]=p==&ports[1];io_value[i]=value;if(on_read)on_read();return value;}
void test_write(RrSerialPortRegisters *p,s32 baud,u16 value){s32 i=io_count++;io_kind[i]=baud?2:1;io_port[i]=p==&ports[1];io_value[i]=value;if(baud)p->baud=value;else p->control=value;}
'''+body+observed
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'));cls.f=cls.lib._padIntPad;cls.observed=cls.lib.observed_tick
        for f in (cls.f,cls.observed):f.argtypes=[];f.restype=C.c_int32
        cls.values={n:C.c_int32.in_dll(cls.lib,n) for n in ('PadFirstChannel','PadLastChannel','PadComActive','PadActiveChannel','PadRecvStage','PadActuatorPowerUsed','begin_result','events','io_count')}
        cls.ram=(C.c_int32*(0x4330//4+2)).in_dll(cls.lib,'ram');cls.counters=(C.c_int32*2).in_dll(cls.lib,'PadPortTimeoutCounts')
        cls.record_size=C.sizeof(ChannelRecord)
        assert C.c_ulong.in_dll(cls.lib,'record_size').value==cls.record_size
        cls.records=(C.c_uint8*(16*cls.record_size)).in_dll(cls.lib,'records');cls.base=C.c_void_p.in_dll(cls.lib,'PadChannelRecords')
        cls.ports=(Port*2).in_dll(cls.lib,'ports');cls.port=C.c_void_p.in_dll(cls.lib,'PadSio0Base')
        cls.error=C.c_void_p.in_dll(cls.lib,'PadAdvanceChannelFn')
        cls.arrays={n:(C.c_int32*32).in_dll(cls.lib,n) for n in ('event_kind','event_index','event_stage','event_counter','error_codes')}
        cls.pointers=(C.c_void_p*32).in_dll(cls.lib,'event_pointer')

    def setUp(self):self.reset()

    def reset(self):
        for v in self.values.values():v.value=0
        self.values['PadLastChannel'].value=1;self.values['PadActiveChannel'].value=91
        self.values['PadRecvStage'].value=92;self.values['PadActuatorPowerUsed'].value=93
        self.values['begin_result'].value=1
        self.counters[:]=[100,101]
        C.memset(self.ram,0xA5,C.sizeof(self.ram));C.memset(self.records,0x5A,C.sizeof(self.records));C.memset(self.ports,0xC3,C.sizeof(self.ports))
        for p in self.ports:p.control=0;p.baud=0xBEEF
        self.base.value=C.addressof(self.records)+4*self.record_size;self.port.value=C.addressof(self.ports)
        self.error.value=C.cast(self.lib.error_a,C.c_void_p).value
        for n in ('on_begin','on_step','on_error','on_read'):C.c_void_p.in_dll(self.lib,n).value=None
        self.retained=[];self.errors=[]

    def hook(self,name,fn,with_pointer=True):
        def safe(*args):
            try:fn(*args)
            except BaseException as error:
                self.errors.append(error)
                self.values['PadActiveChannel'].value=2147483647
                self.values['PadLastChannel'].value=-1
        callback=(Hook if with_pointer else ErrorHook)(safe)
        self.retained.append(callback);C.c_void_p.in_dll(self.lib,name).value=C.cast(callback,C.c_void_p).value

    def invoke(self,observed=False):
        result=(self.observed if observed else self.f)()
        if self.errors:raise self.errors.pop(0)
        self.assertEqual(result,0)

    def test_every_control_halfword_and_early_gate(self):
        untouched=bytes(self.ports);guards=bytes(self.ram)[4:]
        for control in range(65536):
            self.ports[0].control=control;self.ram[0]=-123
            self.invoke()
            self.assertEqual(self.ports[0].control,0 if control&2 else control)
            self.assertEqual(self.ram[0],-123 if control&2 else 1)
            self.assertEqual(bytes(self.ports)[:10],untouched[:10]);self.assertEqual(bytes(self.ports)[12:],untouched[12:])
        self.assertEqual(bytes(self.ram)[4:],guards);self.assertEqual(list(self.counters),[100,101]);self.assertEqual(self.values['events'].value,0)

    def test_signed_counter_saturation_and_disabled_dispatch(self):
        for first in (0,1,-1):
            for last in (0,1,-1):
                for value in (-2147483648,-1,0,149,150,151,2147483647):
                    self.reset();self.values['PadFirstChannel'].value=first;self.values['PadLastChannel'].value=last
                    self.counters[:]=[value,value];self.invoke()
                    self.assertEqual(list(self.counters),[value+int(bool(first) and value<150),value+int(last==0 and value<150)])
                    self.assertEqual([self.values[n].value for n in ('PadActiveChannel','PadRecvStage','PadActuatorPowerUsed','events')],[91,92,93,0])
                    self.assertEqual(self.ports[0].baud,0xBEEF)

    def test_ranges_initializer_result_and_native_record_addresses(self):
        for first,last in [(-1,1),(0,0),(0,2),(1,1),(2,1)]:
            for initialized in (-1,0,1):
                self.reset();self.values['PadComActive'].value=1
                self.values['PadFirstChannel'].value=first;self.values['PadLastChannel'].value=last
                self.values['begin_result'].value=initialized;before=bytes(self.records)
                self.invoke()
                if last<first:
                    self.assertEqual(self.values['events'].value,0);self.assertEqual(self.ports[0].baud,0xBEEF);continue
                kinds=[0]+([1] if initialized==0 else [])+[3]*(last-first+1)
                self.assertEqual(list(self.arrays['event_kind'])[:len(kinds)],kinds)
                self.assertEqual(self.pointers[0],self.base.value+first*self.record_size)
                step_start=2 if initialized==0 else 1
                self.assertEqual(list(self.pointers)[step_start:len(kinds)],[self.base.value+i*self.record_size for i in range(first,last+1)])
                if initialized==0:self.assertEqual(self.arrays['error_codes'][1],65535)
                self.assertEqual((self.arrays['event_index'][0],self.arrays['event_stage'][0],self.arrays['event_counter'][0]),(first,0,93))
                self.assertEqual(self.values['PadActiveChannel'].value,last+1);self.assertEqual(self.values['PadActuatorPowerUsed'].value,0)
                self.assertEqual(self.ports[0].baud,0x88);self.assertEqual(bytes(self.records),before)

    def test_mutating_initializer_error_callback_and_step_reload_all_state(self):
        self.values['PadComActive'].value=1;self.values['PadLastChannel'].value=0;self.values['begin_result'].value=0
        original_base=self.base.value;steps=[]
        def begin(p):
            self.assertEqual(p,original_base)
            self.error.value=C.cast(self.lib.error_b,C.c_void_p).value
            self.base.value=C.addressof(self.records)+8*self.record_size
        def error():
            self.values['PadActiveChannel'].value=-1;self.values['PadLastChannel'].value=0
            self.port.value=C.addressof(self.ports)+C.sizeof(Port)
        def step(p):
            steps.append(p)
            self.assertEqual(self.values['PadActuatorPowerUsed'].value,0)
            if len(steps)==1:
                self.base.value=C.addressof(self.records)+10*self.record_size
                self.values['PadActiveChannel'].value=1;self.values['PadLastChannel'].value=1
            else:self.values['PadActiveChannel'].value=2
        self.hook('on_begin',begin);self.hook('on_error',error,False);self.hook('on_step',step)
        self.invoke(True)
        self.assertEqual(list(self.arrays['event_kind'])[:4],[0,2,3,3])
        self.assertEqual(steps,[C.addressof(self.records)+7*self.record_size,C.addressof(self.records)+11*self.record_size])
        self.assertEqual((self.arrays['error_codes'][1],self.arrays['event_counter'][1]),(65535,93))
        self.assertEqual((self.ports[0].baud,self.ports[1].baud),(0xBEEF,0x88))
        self.assertEqual(self.values['io_count'].value,2)
        self.assertEqual(list((C.c_int32*4).in_dll(self.lib,'io_kind'))[:2],[0,2])
        self.assertEqual(list((C.c_int32*4).in_dll(self.lib,'io_port'))[:2],[0,1])

    def test_early_clear_uses_captured_port_after_read_pointer_mutation(self):
        self.ports[0].control=0xFFFE;self.ports[1].control=0xABCD;before=bytes(self.ram)
        self.hook('on_read',lambda:setattr(self.port,'value',C.addressof(self.ports)+C.sizeof(Port)),False)
        self.invoke(True)
        self.assertEqual((self.ports[0].control,self.ports[1].control),(0,0xABCD))
        self.assertEqual(bytes(self.ram),before);self.assertEqual(self.values['events'].value,0)
        self.assertEqual(self.values['io_count'].value,2)
        self.assertEqual(list((C.c_int32*4).in_dll(self.lib,'io_kind'))[:2],[0,1])
        self.assertEqual(list((C.c_int32*4).in_dll(self.lib,'io_port'))[:2],[0,0])
