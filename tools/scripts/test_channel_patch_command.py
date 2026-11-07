"""Recovered channel command, real installed handlers, and full native callback pointers."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Command(C.Structure):
    _fields_ = [('pad00',C.c_uint8*0x14),('update',C.c_void_p),('complete',C.c_void_p),
                ('pad1C',C.c_uint8*0x10),('data2C',C.c_void_p),('pad30',C.c_uint8*6),
                ('length36',C.c_uint8),('command37',C.c_uint8),('pad38',C.c_uint8*14),
                ('phase',C.c_uint8),('pad47',C.c_uint8*10),('first',C.c_uint8),
                ('second',C.c_uint8),('equal',C.c_uint8),('pad54',C.c_uint8*9),
                ('payload',C.c_uint8*6),('pad63',C.c_uint8*129),('argument',C.c_uint8)]


class Guarded(C.Structure):
    _fields_ = [('before',C.c_uint64*4),('command',Command),('after',C.c_uint64*4)]


STATUS = C.CFUNCTYPE(C.c_int32,C.c_void_p)
UPDATE = C.CFUNCTYPE(None,C.c_void_p)
GETTER = C.CFUNCTYPE(C.c_void_p,C.c_int32)


class ChannelPatchCommandTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory();cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2]
        header=(root/'include/rr/channels.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        source='''#include <stdint.h>
#include <stddef.h>
typedef uint8_t u8;typedef int8_t s8;typedef uint16_t u16;typedef int16_t s16;
typedef uint32_t u32;typedef int32_t s32;
''' + header + '''
RrChannelStatusCallback PadChannelBusyFn;
RrChannelUpdateCallback PadResetChannelFn;
RrChannelRecordCallback PadPortToChannelPtr;
unsigned long test_size(void) {return sizeof(RrChannelPatchCommand);}
unsigned long test_argument_offset(void) {return offsetof(RrChannelPatchCommand,argument);}
'''
        for name in ('_padSetMainMode','channel_patch_handlers','PadSetMainMode'):
            body=(root/('tools/scripts/fixtures/legacy/'+{'_padSetMainMode':'func_'+'800994CC'}.get(name,name)+'.c')).read_text()
            body=re.sub(r'^#include[^\n]*\n','',body,flags=re.M)
            body=re.sub(r'\s+__asm__\("\$[0-9]+"\)','',body)
            body=re.sub(r'"\$[0-9]+",\s*','',body)
            body=re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))','',body)
            source+=body
        directory=Path(cls.temp.name);(directory/'command.c').write_text(source)
        result=subprocess.run(['cc','-shared','-fPIC','-O2','-Wall','-Werror','-Wno-error=deprecated-non-prototype',
                               str(directory/'command.c'),'-o',str(directory/'command.so')],
                              capture_output=True,text=True)
        if result.returncode:raise AssertionError(result.stderr)
        cls.lib=C.CDLL(str(directory/'command.so'))
        cls.start=cls.lib._padSetMainMode
        cls.start.argtypes=[C.POINTER(Command),C.c_int32,C.c_int32];cls.start.restype=C.c_int32
        cls.lib.PadSetMainMode.argtypes=[C.c_int32]*3;cls.lib.PadSetMainMode.restype=None
        cls.update_address=C.cast(cls.lib._padMainModeUpdate,C.c_void_p).value
        cls.complete_address=C.cast(cls.lib._padMainModeComplete,C.c_void_p).value
        for name in ('test_size','test_argument_offset'):getattr(cls.lib,name).restype=C.c_ulong

    def setUp(self):
        self.storage=Guarded();self.record=self.storage.command
        C.memset(C.addressof(self.storage),0xA5,C.sizeof(self.storage))
        self.errors=[];self.calls=[];self.ready_status=0;self.on_ready=lambda:None
        self.on_complete=lambda:None;self.selector=None
        @STATUS
        def ready(pointer):
            try:
                self.calls.append(('ready',pointer));self.on_ready()
                self.after_ready=bytes(self.storage)
                return self.ready_status
            except BaseException as error:self.errors.append(error);return -1
        @UPDATE
        def complete(pointer):
            try:self.calls.append(('complete',pointer));self.on_complete()
            except BaseException as error:self.errors.append(error)
        @GETTER
        def getter(selector):
            self.selector=selector;return C.addressof(self.record)
        self.callbacks=[ready,complete,getter]
        for name,callback in zip(('PadChannelBusyFn','PadResetChannelFn','PadPortToChannelPtr'),self.callbacks):
            C.c_void_p.in_dll(self.lib,name).value=C.cast(callback,C.c_void_p).value

    def expected(self,before,**fields):
        result=bytearray(before)
        for name,value in fields.items():
            field=getattr(Command,name);offset=Guarded.command.offset+field.offset
            if name in ('update','complete','data2C'):
                result[offset:offset+C.sizeof(C.c_void_p)]=value.to_bytes(C.sizeof(C.c_void_p),'little')
            else:result[offset]=value&255
        return bytes(result)

    def check_start(self,first,second):
        result=self.start(C.byref(self.record),first,second)
        if self.errors:raise self.errors[0]
        self.assertEqual(self.calls[-1],('ready',C.addressof(self.record)))
        if self.ready_status:
            self.assertEqual(result,0);self.assertEqual(bytes(self.storage),self.after_ready)
        else:
            self.assertEqual(result,1)
            self.assertEqual(bytes(self.storage),self.expected(self.after_ready,
                update=self.update_address,complete=self.complete_address,phase=1,
                first=first,second=second,equal=(first&255)==self.record.argument))

    def test_layout_and_readiness_failure_preserve_callback_mutations(self):
        self.assertEqual(self.lib.test_size(),C.sizeof(Command))
        self.assertEqual(self.lib.test_argument_offset(),Command.argument.offset)
        for status in (-2**31,-123,-1,1,123,2**31-1):
            self.ready_status=status
            def change():self.record.argument^=255;self.record.phase=23;self.record.payload[2]^=127
            self.on_ready=change
            self.check_start(0x12345678,-1)

    def test_all_comparison_bytes_and_full_word_inputs(self):
        for argument in range(256):
            self.record.argument=argument
            for first in range(256):
                self.check_start(first,(first*73+19)&255)
        for first in (-2**31,-65537,-256,-1,256,65535,2**31-1):
            for second in (-2**31,-257,-1,0,256,2**31-1):
                self.record.argument=first&255;self.check_start(first,second)

    def test_real_installed_handlers_and_every_phase(self):
        self.record.argument=77;self.check_start(77,88)
        update=UPDATE(self.record.update);complete=STATUS(self.record.complete)
        for phase in range(256):
            self.record.phase=phase;before=bytes(self.storage)
            update(C.addressof(self.record))
            changes={}
            if phase==2:changes=dict(command37=0x44,data2C=C.addressof(self.record)+Command.first.offset,length36=2)
            if phase==3:changes=dict(command37=0x4D,data2C=C.addressof(self.record)+Command.payload.offset,length36=6)
            self.assertEqual(bytes(self.storage),self.expected(before,**changes))
        for equal in (0,1,2,127,255):
            for phase in range(256):
                self.record.equal=equal;self.record.phase=phase;before=bytes(self.storage);n=len(self.calls)
                result=complete(C.addressof(self.record))
                self.assertEqual(result,int(bool(equal) and phase==2))
                self.assertEqual(bytes(self.storage),self.expected(before,**({'phase':254} if equal and phase!=2 else {})))
                self.assertEqual(self.calls[n:],[('complete',C.addressof(self.record))] if not equal else [])
        self.record.equal=0
        self.on_complete=lambda:setattr(self.record,'phase',17)
        self.assertEqual(complete(C.addressof(self.record)),0)
        self.assertEqual(self.record.phase,17)
        if self.errors:raise self.errors[0]

    def test_wrapper_preserves_selector_pointer_and_late_argument_read(self):
        for selector in (-2**31,-257,-1,0,15,16,255,2**31-1):
            self.record.argument=3
            self.on_ready=lambda:setattr(self.record,'argument',254)
            self.lib.PadSetMainMode(selector,-258,0x12345)
            self.assertEqual(self.selector,selector)
            self.assertEqual((self.record.first,self.record.second,self.record.equal),(254,0x45,1))
            self.assertEqual(bytes(self.storage),self.expected(self.after_ready,update=self.update_address,
                complete=self.complete_address,phase=1,first=254,second=0x45,equal=1))
        if self.errors:raise self.errors[0]


if __name__=='__main__':unittest.main()
