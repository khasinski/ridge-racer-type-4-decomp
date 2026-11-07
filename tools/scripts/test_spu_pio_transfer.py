"""Real SPU PIO transfer/delay and typed write/read/public wrappers."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Status=C.CFUNCTYPE(C.c_uint16,C.c_void_p)
Write=C.CFUNCTYPE(None,C.c_void_p,C.c_uint16)
Void=C.CFUNCTYPE(None)
Diagnostic=C.CFUNCTYPE(None,C.c_void_p,C.c_void_p)


class SpuPioTransferTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root=Path(__file__).resolve().parents[2]
        header=(root/'include/rr/spu.h').read_text()
        header=re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',header,flags=re.M)
        source='''#include <stdint.h>
#include <stdarg.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef int16_t s16;
typedef uint32_t u32;typedef int32_t s32;
''' + header + '''
RrSpuRegisterBank test_ports[3],*_spu_RXX;
volatile u16 _spu_tsa;
s32 _spu_transMode,_spu_mem_mode_plus,_spu_inTransfer;
void (*volatile _spu_transferCallback)(void);
void test_completion(void) {}
char test_rom[0x10000];
#define SpuTimeoutFmt (test_rom+0x1698)
u16 (*test_status_hook)(void *);
void (*test_data_hook)(void *,u16),(*test_control_hook)(void *,u16);
void (*test_delay_hook)(void),(*test_diagnostic_hook)(void *,void *);
s32 test_delay_calls,test_dma_calls,test_dma_commands[8];
u32 test_dma_address,test_dma_bytes;void *test_dma_buffer;
void test_delay_actual(void);
void _spu_Fw1ts(void) {test_delay_actual();test_delay_calls++;test_delay_hook();}
u16 test_status_read(RrSpuRegisterBank *p) {return test_status_hook(p);}
void test_data_write(RrSpuRegisterBank *p,u16 value) {
 p->transfer_data=value;test_data_hook(p,value);
}
void test_control_write(RrSpuRegisterBank *p,u16 value) {
 p->spu_control=value;test_control_hook(p,value);
}
s32 DebugPrintStub(const char *format,...) {
 va_list args;const char *reason;
 va_start(args,format);reason=va_arg(args,const char *);va_end(args);
 test_diagnostic_hook((void *)format,(void *)reason);return 123;
}
s32 _spu_t(s32 command,...) {
 va_list args;test_dma_commands[test_dma_calls++]=command;
 va_start(args,command);
 if(command==2)test_dma_address=va_arg(args,u32);
 if(command==3) {test_dma_buffer=va_arg(args,void *);test_dma_bytes=va_arg(args,u32);}
 va_end(args);return -123;
}
void test_reset(void) {
 s32 i;memset(test_ports,0x5A,sizeof(test_ports));
 for(i=0;i<3;i++) {test_ports[i].spu_status=0xA123;test_ports[i].spu_control=0xA5EF;}
 _spu_RXX=&test_ports[0];_spu_tsa=0xBCDE;_spu_transMode=1;_spu_mem_mode_plus=3;
 _spu_inTransfer=99;_spu_transferCallback=test_completion;test_delay_calls=test_dma_calls=0;
 test_dma_address=test_dma_bytes=0;test_dma_buffer=0;
}
'''
        for name in ('8007F59C','8007ED44','8007F2E8','8007F36C','8008AF84'):
            body=(root/('tools/scripts/fixtures/views/func_'+name+'.c')).read_text()
            body=re.sub(r'^#include[^\n]*\n|^extern[^\n]*\n','',body,flags=re.M)
            body=re.sub(r'\s+__asm__\("\$[0-9]+"\)','',body)
            body=re.sub(r'"\$[0-9]+",\s*','',body)
            body=re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))','',body)
            if name=='8007F59C':
                assert body.count('_spu_Fw1ts(')==1
                body=body.replace('_spu_Fw1ts(','test_delay_actual(')
            if name=='8007ED44':
                substitutions={
                    '_spu_RXX->spu_status':('test_status_read(_spu_RXX)',3),
                    'port->spu_status':('test_status_read(port)',2),
                    'output->transfer_data=*input++;':('test_data_write(output,*input++);',1),
                    '*(u16 *)&control->spu_control=command|0x10;':('test_control_write(control,command|0x10);',1),
                    'port->spu_control=command;':('test_control_write(port,command);',1),
                    '(const char *)0x80010000U':('test_rom',2)}
                for old,(new,count) in substitutions.items():
                    assert body.count(old)==count,old
                    body=body.replace(old,new)
            source+=body
        directory=Path(cls.temp.name)
        (directory/'function.c').write_text(source)
        result=subprocess.run(['cc','-shared','-fPIC','-O2',str(directory/'function.c'),
                               '-o',str(directory/'function.so')],capture_output=True)
        if result.returncode:raise AssertionError(result.stderr.decode())
        cls.lib=C.CDLL(str(directory/'function.so'))
        cls.ports=(C.c_uint16*(3*256)).in_dll(cls.lib,'test_ports')
        cls.rom=(C.c_uint8*0x10000).in_dll(cls.lib,'test_rom')
        cls.rom[:]=(root/'assets/PAL/main.exe').read_bytes()[0x800:0x10800]
        cls.transfer=cls.lib._spu_FwriteByIO
        cls.transfer.argtypes=[C.POINTER(C.c_uint16),C.c_uint32]
        cls.transfer.restype=None
        for name in ('_spu_Fw','_spu_Fr','SpuWrite'):
            function=getattr(cls.lib,name)
            function.argtypes=[C.POINTER(C.c_uint16),C.c_uint32]
            function.restype=C.c_uint32

    def setUp(self):
        self.lib.test_reset()
        self.errors=[];self.callbacks=[];self.data=[];self.controls=[];self.diagnostics=[]
        self.status=lambda address:C.c_uint16.from_address(address+0x1AE).value
        self.on_data=lambda address,value:None
        self.on_control=lambda address,value:None
        self.on_delay=lambda:None
        self.on_diagnostic=lambda format,reason:None
        def status(address):
            try:return self.status(address)
            except BaseException as error:self.errors.append(error);return 0
        def data(address,value):
            self.data.append((address,value))
            try:self.on_data(address,value)
            except BaseException as error:self.errors.append(error)
        def control(address,value):
            self.controls.append((address,value))
            try:self.on_control(address,value)
            except BaseException as error:self.errors.append(error)
        def delay():
            try:self.on_delay()
            except BaseException as error:self.errors.append(error)
        def diagnostic(format,reason):
            self.diagnostics.append((C.string_at(format),C.string_at(reason)))
            try:self.on_diagnostic(format,reason)
            except BaseException as error:self.errors.append(error)
        for name,kind,action in (('status',Status,status),('data',Write,data),('control',Write,control),
                                 ('delay',Void,delay),('diagnostic',Diagnostic,diagnostic)):
            callback=kind(action);self.callbacks.append(callback)
            C.c_void_p.in_dll(self.lib,'test_'+name+'_hook').value=C.cast(callback,C.c_void_p).value

    def scalar(self,name):return C.c_int32.in_dll(self.lib,name)
    def port(self,index):return C.addressof(self.ports)+index*512
    def select_port(self,index):C.c_void_p.in_dll(self.lib,'_spu_RXX').value=self.port(index)
    def input(self,size):
        values=(C.c_uint16*((size+1)//2+8))()
        for i in range(len(values)):values[i]=(i*0x4567+0x1234)&65535
        return values
    def check_errors(self):
        if self.errors:raise self.errors[0]

    def test_every_size_through_three_chunks_odd_rounding_and_zero_input(self):
        for size in range(193):
            self.setUp()
            values=self.input(size);before=bytes(values);ports_before=bytearray(bytes(self.ports))
            self.transfer(values if size else None,size);self.check_errors()
            self.assertEqual([value for _,value in self.data],list(values[:(size+1)//2]))
            self.assertEqual([address for address,_ in self.data],[self.port(0)]*((size+1)//2))
            chunks=(size+63)//64
            self.assertEqual(self.scalar('test_delay_calls').value,1+chunks*3)
            self.assertEqual(self.controls,[(self.port(0),0xA5DF)]*chunks+[(self.port(0),0xA5CF)])
            ports_before[0x1A6:0x1A8]=(0xBCDE).to_bytes(2,'little')
            ports_before[0x1AA:0x1AC]=(0xA5CF).to_bytes(2,'little')
            if size:ports_before[0x1A8:0x1AA]=values[(size-1)//2].to_bytes(2,'little')
            self.assertEqual(bytes(self.ports),bytes(ports_before))
            self.assertEqual(bytes(values),before)
            self.assertEqual(self.diagnostics,[])

    def test_all_halfword_values_and_initial_status_mask(self):
        values=(C.c_uint16*65536)(*range(65536))
        self.transfer(values,C.sizeof(values));self.check_errors()
        self.assertEqual([value for _,value in self.data],list(range(65536)))
        self.assertEqual(self.diagnostics,[])
        for previous in (0,1,0x3FF,0x400,0x7FF):
            self.setUp()
            reads=[]
            def status(address):
                reads.append(address)
                return (previous|0xF800) if len(reads)==1 else previous
            self.status=status
            self.transfer(None,0);self.check_errors()
            self.assertEqual(len(reads),2)
            self.assertEqual(self.diagnostics,[])

    def test_busy_and_restore_timeout_limits_and_continuing_after_diagnostic(self):
        for phase in ('busy','restore'):
            self.setUp();values=self.input(66);state=['initial',0];counts={'busy':0,'restore':0}
            def control(address,value):state[:]=['busy' if value&0x10 else 'restore',0]
            def status(address):
                kind=state[0]
                if kind=='initial':return 0x123
                counts[kind]+=1
                if kind==phase:return 0x400 if kind=='busy' else 0x122
                return 0 if kind=='busy' else 0x123
            self.on_control=control;self.status=status
            self.transfer(values,66);self.check_errors()
            self.assertEqual([value for _,value in self.data],list(values[:33]))
            self.assertEqual(self.scalar('test_delay_calls').value,7)
            self.assertEqual(counts[phase],(0xF01*2) if phase=='busy' else 0xF01)
            self.assertEqual(len(self.diagnostics),2 if phase=='busy' else 1)
            expected_reason=C.string_at(C.addressof(self.rom)+(0x16B8 if phase=='busy' else 0x16CC))
            self.assertTrue(all(item[1]==expected_reason for item in self.diagnostics))

    def test_captured_batch_port_fresh_control_and_pointer_after_delay(self):
        values=self.input(66)
        def delay():
            number=self.scalar('test_delay_calls').value
            if number==1:
                self.assertEqual(self.ports[0x1A6//2],0xBCDE)
                self.select_port(1)
            if number==2:self.select_port(0)
        def data(address,value):
            if len(self.data)==1:self.select_port(2)
        self.on_delay=delay;self.on_data=data
        self.transfer(values,66);self.check_errors()
        self.assertEqual([address for address,_ in self.data],[self.port(1)]*32+[self.port(0)])
        self.assertEqual([address for address,_ in self.controls],[self.port(2),self.port(0),self.port(0)])

    def test_typed_dma_wrappers_full_pointer_clamp_and_late_completion_flag(self):
        values=self.input(128)
        self.scalar('_spu_transMode').value=0
        for name,command in (('_spu_Fw',1),('_spu_Fr',0),('SpuWrite',1)):
            for size in (0,1,127,0x7EFF0,0x7EFF1,0xFFFFFFFF):
                self.scalar('test_dma_calls').value=0
                C.c_void_p.in_dll(self.lib,'_spu_transferCallback').value=0;self.scalar('_spu_inTransfer').value=99
                expected=min(size,0x7EFF0) if name=='SpuWrite' else size
                self.assertEqual(getattr(self.lib,name)(values,size),expected)
                self.assertEqual(self.scalar('test_dma_calls').value,3)
                commands=(C.c_int32*8).in_dll(self.lib,'test_dma_commands')
                self.assertEqual(list(commands[:3]),[2,command,3])
                self.assertEqual(C.c_void_p.in_dll(self.lib,'test_dma_buffer').value,C.addressof(values))
                self.assertEqual(C.c_uint32.in_dll(self.lib,'test_dma_bytes').value,expected)
                self.assertEqual(C.c_uint32.in_dll(self.lib,'test_dma_address').value,0xBCDE<<3)
                self.assertEqual(self.scalar('_spu_inTransfer').value,0 if name=='SpuWrite' else 99)
        self.setUp()
        C.c_void_p.in_dll(self.lib,'_spu_transferCallback').value=0
        def changed():C.c_void_p.in_dll(self.lib,'_spu_transferCallback').value=C.cast(self.lib.test_completion,C.c_void_p).value
        self.on_delay=changed
        self.assertEqual(self.lib.SpuWrite(values,65),65);self.check_errors()
        self.assertEqual(self.scalar('_spu_inTransfer').value,99)
        self.assertEqual(self.scalar('test_dma_calls').value,0)
        self.assertEqual(len(self.data),33)
