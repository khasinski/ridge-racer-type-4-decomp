"""Execute actual queue acquisition with native pointers and a state-read observer."""
import ctypes as C
from pathlib import Path
import random
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Output(C.Structure):
    _fields_=[('guard0',C.c_uint64),('data',C.c_void_p),('guard1',C.c_uint64),('header',C.c_void_p),('guard2',C.c_uint64)]


class LibraryQueueAcquireTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        header=(root/'include/rr/library.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        body=(root/'tools/scripts/fixtures/views/func_8008F854.c').read_text().replace('#include "rr/library.h"','')
        body=re.sub(r'\s+__asm__\("\$[0-9]+"\)','',body)
        body=re.sub(r' : "\$[0-9]+"','',body)
        assert body.count('(s32 *)0x80110000U')==1
        body=body.replace('(s32 *)0x80110000U','test_page')
        assert body.count('selected->state==1')==1 and body.count('selected->state!=2')==1
        observed=body.replace('StGetNext','observed_acquire').replace('selected->state==1','test_read(selected)==1').replace('selected->state!=2','test_read(selected)!=2')
        code='''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
'''+header+'''
RrLibQueueRecord32 test_records[4096];
RrLibQueueRecord32 *StRingHeaders=test_records;
s32 test_page[0x10000/4];
#define D_801102FC test_page[0x2FC/4]
s32 StEndFrame,D_80110314;
s32 reads,change_after_first;
u16 test_read(RrLibQueueRecord32 *p) {
    u16 state=p->state;
    reads++;
    if(reads==1 && change_after_first) p->state=2;
    return state;
}
'''+body+observed
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'))
        cls.records=(C.c_uint16*(4096*16)).in_dll(cls.lib,'test_records')
        cls.page=(C.c_int32*(0x10000//4)).in_dll(cls.lib,'test_page')
        cls.reset=C.c_int32.in_dll(cls.lib,'StEndFrame');cls.count=C.c_int32.in_dll(cls.lib,'D_80110314')
        cls.reads=C.c_int32.in_dll(cls.lib,'reads');cls.change=C.c_int32.in_dll(cls.lib,'change_after_first')
        for name in ('StGetNext','observed_acquire'):
            fn=getattr(cls.lib,name);fn.argtypes=[C.POINTER(C.c_void_p),C.POINTER(C.c_void_p)];fn.restype=C.c_int32

    def setUp(self):
        C.memset(C.addressof(self.records),0xA5,C.sizeof(self.records))
        for i in range(len(self.page)):self.page[i]=0x12345678
        self.page[0x2FC//4]=0;self.reset.value=0;self.count.value=32
        self.reads.value=0;self.change.value=0

    def output(self):return Output(0x0123456789ABCDEF,0x1234,0xFEDCBA9876543210,0x5678,0xA55A5AA5A55A5AA5)

    def call(self,out,observed=False):
        fn=self.lib.observed_acquire if observed else self.lib.StGetNext
        return fn(C.cast(C.byref(out,Output.data.offset),C.POINTER(C.c_void_p)),C.cast(C.byref(out,Output.header.offset),C.POINTER(C.c_void_p)))

    def guards(self,out):self.assertEqual((out.guard0,out.guard1,out.guard2),(0x0123456789ABCDEF,0xFEDCBA9876543210,0xA55A5AA5A55A5AA5))

    def test_every_state_halfword_and_reset_policy(self):
        for reset in (0,1):
            for state in range(65536):
                self.page[0x2FC//4]=3;self.reset.value=reset
                self.records[0]=2;self.records[3*16]=state
                out=self.output();r=self.call(out)
                selected=0 if state==1 else 3
                ready=state in (1,2)
                self.assertEqual(r,0 if ready else 1)
                self.assertEqual(self.page[0x2FC//4],selected)
                expected_old=0 if state==1 and reset else (4 if state==2 else state)
                self.assertEqual(self.records[3*16],expected_old)
                self.assertEqual(self.records[0],4 if state==1 else 2)
                base=C.addressof(self.records)
                self.assertEqual((out.data,out.header),(base+32*32+selected*0x7E0,base+selected*32) if ready else (0x1234,0x5678))
                self.guards(out)

    def test_exact_memory_changes_and_native_pointer_outputs(self):
        rng=random.Random(0x8F854)
        for _ in range(500):
            index=rng.randrange(32);first=rng.choice((0,1,2,4,65535));state=rng.choice((0,1,2,3,4,65535))
            reset=rng.choice((-2147483648,-1,0,1,2));count=rng.randint(32,64)
            self.page[0x2FC//4]=index;self.reset.value=reset;self.count.value=count
            self.records[0]=first;self.records[index*16]=state
            expected=bytearray(self.records);page_before=bytearray(self.page)
            selected=index
            if state==1:
                selected=0
                if reset:expected[index*32:index*32+2]=b'\0\0'
                page_before[0x2FC:0x300]=b'\0'*4
            selected_state=int.from_bytes(expected[selected*32:selected*32+2],'little')
            ready=selected_state==2
            if ready:expected[selected*32:selected*32+2]=struct.pack('<H',4)
            out=self.output();self.assertEqual(self.call(out),0 if ready else 1)
            self.assertEqual(bytes(self.records),bytes(expected));self.assertEqual(bytes(self.page),bytes(page_before))
            base=C.addressof(self.records)
            self.assertEqual((out.data,out.header),(base+count*32+selected*0x7E0,base+selected*32) if ready else (0x1234,0x5678))
            self.guards(out)

    def test_failed_acquisition_accepts_null_output_slots(self):
        for index in (0,1,31):
            for state in (0,1,3,4,65535):
                self.page[0x2FC//4]=index;self.reset.value=1
                self.records[0]=0;self.records[index*16]=state
                self.assertEqual(self.lib.StGetNext(None,None),1)

    def test_second_read_observes_new_ready_state(self):
        self.page[0x2FC//4]=7;self.records[7*16]=3;self.change.value=1
        out=self.output()
        self.assertEqual(self.call(out,observed=True),0)
        self.assertEqual(self.reads.value,2)
        self.assertEqual(self.records[7*16],4)
        self.assertEqual(out.header,C.addressof(self.records)+7*32)
        self.assertEqual(out.data,C.addressof(self.records)+32*32+7*0x7E0)
        self.guards(out)
