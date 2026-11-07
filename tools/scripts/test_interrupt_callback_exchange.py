"""Actual IRQ callback exchange and driver wrapper with native pointer-valued slots."""
import ctypes as C
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest
from test_library_saved_state import State

Hook=C.CFUNCTYPE(None,C.c_int32,C.c_int32,C.c_int32)


class InterruptCallbackExchangeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        header=(root/'include/rr/library.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(root/'tools/scripts/fixtures/views/func_8008B7A0.c').read_text().replace('#include "rr/library.h"','')
        body=re.sub(r'\s+__asm__\("\$[0-9]+"\)','',body)
        observed=body.replace('setIntr','observed_exchange')
        sites={'raw=*I_MASK;':'raw=test_read(I_MASK);','*I_MASK=0;':'test_write(I_MASK,0);','*I_MASK=saved;':'test_write(I_MASK,saved);'}
        for old,new in sites.items():
            assert observed.count(old)==1
            observed=observed.replace(old,new)
        wrapper=(root/'tools/scripts/fixtures/views/func_8008B394.c').read_text().replace('#include "rr/library.h"','')
        code='''#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
#define RR_FIELD_OFFSET(type,field) offsetof(type,field)
'''+header+'''
RrLibSavedState IntrEnv;
u16 ports[6];volatile u16 *I_MASK=&ports[1];
s32 called[3],io_count,io_kind[32],io_port[32];u32 io_value[32];
void (*on_bios)(s32,s32,s32);
void callback_a(void) {called[0]++;}
void callback_b(void) {called[1]++;}
void callback_c(void) {called[2]++;}
void invoke(RrLibInterruptCallback callback) {callback();}
s32 ChangeClearPAD(s32 disabled) {if(on_bios)on_bios(0,disabled,0);return -123;}
s32 ChangeClearRCnt(s32 counter,s32 disabled) {if(on_bios)on_bios(1,counter,disabled);return -234;}
void io(s32 kind,volatile u16 *p,u16 value) {
    s32 i=io_count++;io_kind[i]=kind;io_port[i]=p==&ports[4];io_value[i]=value;
}
u16 test_read(volatile u16 *p) {u16 value=*p;io(0,p,value);return value;}
void test_write(volatile u16 *p,u16 value) {io(1,p,value);*p=value;}
RrLibInterruptDriverPrefix driver={0,0,setIntr,0};
RrLibInterruptDriverPrefix *IntrDriverTablePtr=&driver;
unsigned long native_size=sizeof(RrLibSavedState);
unsigned long native_offsets[]={offsetof(RrLibSavedState,callbacks),offsetof(RrLibSavedState,enabled_mask),offsetof(RrLibSavedState,callback_data)};
'''+body+observed+wrapper
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'))
        assert C.c_ulong.in_dll(cls.lib,'native_size').value==C.sizeof(State)
        assert list((C.c_ulong*3).in_dll(cls.lib,'native_offsets'))==[State.callbacks.offset,State.enabled_mask.offset,State.callback_data.offset]
        cls.state=State.in_dll(cls.lib,'IntrEnv');cls.ports=(C.c_uint16*6).in_dll(cls.lib,'ports')
        cls.port=C.c_void_p.in_dll(cls.lib,'I_MASK');cls.hook=C.c_void_p.in_dll(cls.lib,'on_bios')
        cls.callbacks=[C.cast(getattr(cls.lib,name),C.c_void_p).value for name in ('callback_a','callback_b','callback_c')]
        cls.called=(C.c_int32*3).in_dll(cls.lib,'called');cls.io_count=C.c_int32.in_dll(cls.lib,'io_count')
        cls.io_kind=(C.c_int32*32).in_dll(cls.lib,'io_kind');cls.io_port=(C.c_int32*32).in_dll(cls.lib,'io_port');cls.io_value=(C.c_uint32*32).in_dll(cls.lib,'io_value')
        for name in ('setIntr','observed_exchange','InterruptCallback'):
            f=getattr(cls.lib,name);f.argtypes=[C.c_int32,C.c_void_p];f.restype=C.c_void_p
        cls.lib.invoke.argtypes=[C.c_void_p];cls.lib.invoke.restype=None

    def setUp(self):
        self.hook.value=None;self.retained=[];self.events=[]
        C.memset(C.addressof(self.state),0xA5,C.sizeof(self.state));self.state.active=1
        self.state.callbacks[:]=[self.callbacks[0]]*11
        self.ports[:]=[0xCAFE,0xABCC,0xBEEF,0xFACE,0x1111,0x1234]
        self.port.value=C.addressof(self.ports)+2
        self.io_count.value=0;self.called[:]=[0,0,0]

    def install_hook(self,fn):
        callback=Hook(fn);self.retained.append(callback);self.hook.value=C.cast(callback,C.c_void_p).value

    def trace(self):return [(self.io_kind[i],self.io_port[i],self.io_value[i]) for i in range(self.io_count.value)]

    def test_equal_callback_and_inactive_gates_do_not_access_null_port(self):
        self.port.value=None
        for active in (0,1,2,65535):
            self.state.active=active
            for index in range(11):
                for previous in (None,*self.callbacks):
                    self.state.callbacks[index]=previous;before=bytes(self.state)
                    self.assertEqual(self.lib.observed_exchange(index,previous),previous)
                    self.assertEqual(bytes(self.state),before)
        self.state.active=0
        for index in range(11):
            self.state.callbacks[index]=self.callbacks[0];before=bytes(self.state)
            for requested in (None,self.callbacks[1]):
                self.assertEqual(self.lib.observed_exchange(index,requested),self.callbacks[0])
                self.assertEqual(bytes(self.state),before)
        self.assertEqual(self.trace(),[]);self.assertEqual(list(self.called),[0,0,0])

    def test_all_selectors_masks_and_callback_values(self):
        rng=random.Random(0x8B7A0)
        cases=[(index,requested,mask,mask^65535) for index in range(11) for requested in (None,self.callbacks[1],self.callbacks[2]) for mask in (0,1,0x8000,0x55AA,0xAA55,65535)]
        cases += [(rng.randrange(11),rng.choice((None,self.callbacks[1],self.callbacks[2])),rng.randrange(65536),rng.randrange(65536)) for _ in range(1000)]
        for index,requested,mask,ram_mask in cases:
            self.state.active=rng.choice((1,2,256,65535));self.state.callbacks[index]=self.callbacks[0]
            self.state.enabled_mask=ram_mask;self.ports[1]=mask;self.io_count.value=0
            expected=State.from_buffer_copy(bytes(self.state));expected.callbacks[index]=requested
            bit=1<<index;expected.enabled_mask=(ram_mask|bit) if requested else (ram_mask&~bit)
            published=mask|bit if requested else mask&~bit
            events=[]
            self.install_hook(lambda k,a,b:events.append((k,a,b,self.ports[1],self.state.callbacks[index],self.state.enabled_mask)))
            self.assertEqual(self.lib.observed_exchange(index,requested),self.callbacks[0])
            self.assertEqual(bytes(self.state),bytes(expected));self.assertEqual(self.ports[1],published)
            disabled=int(requested is None)
            calls=[(0,disabled,0),(1,3,disabled)] if index==0 else ([(1,index-4,disabled)] if index in (4,5,6) else [])
            self.assertEqual(events,[(k,a,b,0,requested,expected.enabled_mask) for k,a,b in calls])
            self.assertEqual(self.trace(),[(0,0,mask),(1,0,0),(1,0,published)])
            self.assertEqual((self.ports[0],self.ports[2],self.ports[3],self.ports[4],self.ports[5]),(0xCAFE,0xBEEF,0xFACE,0x1111,0x1234))
        self.assertEqual(list(self.called),[0,0,0])

    def test_bios_changes_survive_and_final_write_uses_new_port(self):
        self.state.enabled_mask=0x1234
        events=[]
        def mutate(kind,a,b):
            events.append((kind,a,b,self.ports[1],self.ports[4]))
            if kind==0:
                self.port.value=C.addressof(self.ports)+8;self.ports[1]=0x8888
                self.state.enabled_mask=0xA5A5;self.state.callbacks[0]=self.callbacks[2]
        self.install_hook(mutate)
        self.assertEqual(self.lib.observed_exchange(0,self.callbacks[1]),self.callbacks[0])
        self.assertEqual(self.trace(),[(0,0,0xABCC),(1,0,0),(1,1,0xABCD)])
        self.assertEqual(events,[(0,0,0,0,0x1111),(1,3,0,0x8888,0x1111)])
        self.assertEqual((self.ports[1],self.ports[4],self.state.enabled_mask,self.state.callbacks[0]),(0x8888,0xABCD,0xA5A5,self.callbacks[2]))

    def test_nested_exchange_preserves_original_snapshot_restore_behavior(self):
        self.ports[1]=0x200;self.state.enabled_mask=0
        events=[];nested=[];entered=False
        def reenter(kind,a,b):
            nonlocal entered
            events.append((kind,a,b,self.ports[1]))
            if kind==0 and not entered:
                entered=True;nested.append(self.lib.observed_exchange(4,self.callbacks[2]))
        self.install_hook(reenter)
        self.assertEqual(self.lib.observed_exchange(0,self.callbacks[1]),self.callbacks[0])
        self.assertEqual(nested,[self.callbacks[0]])
        self.assertEqual(events,[(0,0,0,0),(1,0,0,0),(1,3,0,0x10)])
        self.assertEqual(self.trace(),[(0,0,0x200),(1,0,0),(0,0,0),(1,0,0),(1,0,0x10),(1,0,0x201)])
        self.assertEqual((self.ports[1],self.state.enabled_mask),(0x201,0x11))
        self.assertEqual((self.state.callbacks[0],self.state.callbacks[4]),(self.callbacks[1],self.callbacks[2]))

    def test_real_driver_wrapper_returns_usable_full_native_pointer(self):
        a,b,c=self.callbacks
        if C.sizeof(C.c_void_p)>4:self.assertGreater(a,0xFFFFFFFF)
        previous=self.lib.InterruptCallback(7,b);self.assertEqual(previous,a)
        self.assertEqual(self.state.callbacks[7],b)
        self.assertEqual(self.lib.InterruptCallback(7,c),b)
        self.assertEqual(self.lib.InterruptCallback(7,None),c)
        self.port.value=None;self.assertIsNone(self.lib.InterruptCallback(7,None))
        self.lib.invoke(previous);self.assertEqual(list(self.called),[1,0,0])
