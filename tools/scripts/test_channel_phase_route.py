"""Actual phase router, fallback and command constructors on one shared native record."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest
from test_cd_disc_probe import native_body
from test_channel_callbacks import ChannelRecord,ChannelStatus
from c_functions import source_owners


class ChannelPhaseRouteTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2];d=Path(cls.temp.name)
        h=(root/'include/rr/channels.h').read_text()
        h=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',h,flags=re.M)
        code='''#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8;typedef int8_t s8;typedef uint16_t u16;typedef int16_t s16;typedef uint32_t u32;typedef int32_t s32;
'''+h+'''
RrChannelRecordF0 channel;
s32 events,event_kind[4],mutate,next_phase;void *event_pointer[4];
unsigned long record_size=sizeof(RrChannelRecordF0),packet_offset=offsetof(RrChannelStatusView,packet),callback_offset=offsetof(RrChannelStatusView,update14),buffer_offset=offsetof(RrChannelStatusView,data2C);
void trace(s32 kind,void *p){s32 i=events++;event_kind[i]=kind;event_pointer[i]=p;}
void private_a(void *p){trace(2,p);}void private_b(void *p){trace(3,p);}
void global_a(void *p){
    RrChannelRecordF0 *c=p;trace(1,p);
    if(mutate){c->responseE8=0;c->status.phase=next_phase;c->status.update14=private_b;}
}
RrChannelUpdateCallback PadResetChannelFn=global_a;
'''
        owners=source_owners(root/'src/main/PAL')
        paths=dict.fromkeys(owners[name] for name in ('_padChannelDispatch','_padCmdParaMode','_padCmd45','_padCmd4C','_padCmd46','_padCmd47','_padCmd4B','_padSendAtLoadInfo'))
        for path in paths:code+=native_body(path.read_text())+'\n'
        (d/'f.c').write_text(code)
        subprocess.run(['cc','-shared','-fPIC','-O2',str(d/'f.c'),'-o',str(d/'f.so')],check=True,capture_output=True)
        cls.lib=C.CDLL(str(d/'f.so'));cls.f=cls.lib._padChannelDispatch;cls.f.argtypes=[C.c_void_p];cls.f.restype=C.c_int32
        assert C.c_ulong.in_dll(cls.lib,'record_size').value==C.sizeof(ChannelRecord)
        for name,field in [('packet_offset','packet'),('callback_offset','update14'),('buffer_offset','data2C')]:
            assert C.c_ulong.in_dll(cls.lib,name).value==getattr(ChannelStatus,field).offset
        cls.channel=ChannelRecord.in_dll(cls.lib,'channel')
        cls.events=C.c_int32.in_dll(cls.lib,'events');cls.kind=(C.c_int32*4).in_dll(cls.lib,'event_kind');cls.pointers=(C.c_void_p*4).in_dll(cls.lib,'event_pointer')
        cls.mutate=C.c_int32.in_dll(cls.lib,'mutate');cls.next_phase=C.c_int32.in_dll(cls.lib,'next_phase')
        cls.private_a=C.cast(cls.lib.private_a,C.c_void_p).value;cls.private_b=C.cast(cls.lib.private_b,C.c_void_p).value
        cls.buffer=(C.c_uint8*8)(*range(8));cls.packet=(C.c_uint8*4)(0,0x12,0x34,0x56)
        for name in ('_padCmdParaMode','_padCmd4C','_padCmd46','_padCmd47'):
            fn=getattr(cls.lib,name);fn.argtypes=[C.c_void_p,C.c_int32];fn.restype=None
        for name in ('_padCmd45','_padCmd4B'):
            fn=getattr(cls.lib,name);fn.argtypes=[C.c_void_p];fn.restype=None

    def reset(self,phase=0,packet=0,response=1,status=0,private=False):
        C.memset(C.addressof(self.channel),0xA5,C.sizeof(self.channel))
        self.channel.status.phase=phase;self.channel.status.state49=status;self.channel.responseE8=response
        self.channel.status.packet=C.addressof(self.packet);self.packet[0]=packet
        self.channel.status.update14=self.private_a if private else None
        self.channel.status.data2C=C.addressof(self.buffer)
        self.events.value=0;self.mutate.value=0

    def command(self,expected,code,value=None):
        expected.status.command37=code
        expected.status.length36=int(value is not None)
        expected.status.data2C=None if value is None else C.addressof(self.channel)+ChannelStatus.argument24.offset
        if value is not None:expected.status.argument24=value&255

    def check(self,expected,kinds):
        before=(bytes(self.packet),bytes(self.buffer))
        self.assertEqual(self.f(C.byref(self.channel)),0)
        self.assertEqual(bytes(self.channel),bytes(expected))
        self.assertEqual(list(self.kind)[:self.events.value],kinds)
        self.assertEqual(list(self.pointers)[:self.events.value],[C.addressof(self.channel)]*len(kinds))
        self.assertEqual((bytes(self.packet),bytes(self.buffer)),before)

    def test_all_phases_packet_paths_response_gates_and_private_callback(self):
        for phase in range(256):
            for packet in (0,0xF3,0xFF):
                for response in (0,1):
                    for status in (0,2):
                        for private in (False,True):
                            self.reset(phase,packet,response,status,private)
                            expected=ChannelRecord.from_buffer_copy(bytes(self.channel));kinds=[]
                            if packet==0xF3 and (response==0 or phase==255):self.command(expected,0x43,0)
                            else:
                                if packet==0xF3 and status==2:kinds.append(1)
                                if phase==1:self.command(expected,0x43,1)
                                elif phase==254:self.command(expected,0x43,0)
                                elif phase not in (0,255):
                                    if private:kinds.append(2)
                                    elif phase==2:self.command(expected,0x45)
                                    elif phase==3:self.command(expected,0x4C,expected.status.argumentE4)
                                    elif phase==4:self.command(expected,0x47,expected.status.state47)
                            self.check(expected,kinds)

    def test_global_callback_changes_phase_and_private_handler_before_dispatch(self):
        for phase in range(256):
            self.reset(2,0xF3,1,2,True);self.mutate.value=1;self.next_phase.value=phase
            expected=ChannelRecord.from_buffer_copy(bytes(self.channel));expected.responseE8=0
            expected.status.phase=phase;expected.status.update14=self.private_b;kinds=[1]
            if phase==1:self.command(expected,0x43,1)
            elif phase==254:self.command(expected,0x43,0)
            elif phase not in (0,255):kinds.append(3)
            self.check(expected,kinds)
        if C.sizeof(C.c_void_p)>4:self.assertGreater(self.private_b,0xFFFFFFFF)

    def test_constructor_full_word_arguments_native_pointer_and_guards(self):
        for name,code in [('_padCmdParaMode',0x43),('_padCmd4C',0x4C),('_padCmd46',0x46),('_padCmd47',0x47)]:
            for high in (0,0x12340000,0x80000000,0xFFFF0000):
                for value in range(256):
                    self.reset();expected=ChannelRecord.from_buffer_copy(bytes(self.channel))
                    self.command(expected,code,value)
                    getattr(self.lib,name)(C.byref(self.channel),C.c_int32(high|value).value)
                    self.assertEqual(bytes(self.channel),bytes(expected))
                    self.assertEqual(self.channel.status.data2C,C.addressof(self.channel)+ChannelStatus.argument24.offset)
        for name,code in [('_padCmd45',0x45),('_padCmd4B',0x4B)]:
            self.reset();expected=ChannelRecord.from_buffer_copy(bytes(self.channel));self.command(expected,code)
            getattr(self.lib,name)(C.byref(self.channel));self.assertEqual(bytes(self.channel),bytes(expected))
            self.assertIsNone(self.channel.status.data2C)

    def test_fallback_transmits_every_byte_from_both_distinct_fields(self):
        for phase,code,field in [(3,0x4C,'argumentE4'),(4,0x47,'state47')]:
            for value in range(256):
                self.reset(phase);setattr(self.channel.status,field,value)
                expected=ChannelRecord.from_buffer_copy(bytes(self.channel));self.command(expected,code,value)
                self.check(expected,[])
