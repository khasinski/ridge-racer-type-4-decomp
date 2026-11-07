"""Actual card write/read verifier with deterministic asynchronous BIOS stubs."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Buffer(C.Structure):
    _fields_ = [('before', C.c_uint32 * 2), ('data', C.c_uint8 * 128), ('after', C.c_uint32 * 2)]


Operation = C.CFUNCTYPE(C.c_int32, C.c_int32, C.c_int32, C.c_int32, C.c_void_p, C.c_int32)


class CardSectorVerifyTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        header = (root/'include/rr/bios_devices.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        code = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef int32_t s32;
''' + header + '''
s32 (*test_operation)(s32,s32,s32,void *,s32);
s32 _card_write(s32 port,s32 sector,const u8 *buffer){return test_operation(1,port,sector,(void *)buffer,128);}
void _new_card(void){test_operation(0,0,0,0,0);}
s32 _card_read(s32 port,s32 sector,u8 *buffer){return test_operation(4,port,sector,buffer,128);}
s32 _card_status(s32 slot){return test_operation(2,slot,0,0,0);}
void PsyqBzero(void *buffer,s32 size){memset(buffer,0,size);test_operation(3,0,0,buffer,size);}
char CardReadErrorMsg[64];
s32 DebugPrintStub(const char *format,...){return test_operation(5,0,0,(void *)format,0);}
'''
        body = (root/'tools/scripts/fixtures/views/func_80096054.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+",\s*', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(code+body)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.function = cls.lib._card_sector_write
        cls.function.argtypes = [C.c_int32,C.c_int32,C.c_void_p]
        cls.function.restype = C.c_int32
        exe=(root/'assets/PAL/main.exe').read_bytes();start=0x800+0x1FD8
        cls.message=exe[start:exe.index(b'\0',start)]
        C.memmove(C.addressof((C.c_char*64).in_dll(cls.lib,'CardReadErrorMsg')),cls.message+b'\0',len(cls.message)+1)

    @staticmethod
    def checksum(data):
        value=0
        for byte in data[:127]:value^=byte
        return value

    def transfer(self, data, *, port=0, sector=5, writes=None, reads=None,
                 contents=None, statuses=None, on_read=None, on_diagnostic=None):
        box=Buffer();C.memset(C.addressof(box),0xA5,C.sizeof(box));box.data[:]=data
        address=C.addressof(box)+Buffer.data.offset
        events=[];errors=[];sent=[];readbacks=[];diagnostics=[]
        state={'attempt':-1,'phase':None,'poll':0,'output':None,'read_return':None}
        def choose(values,index,default):
            return values[min(index,len(values)-1)] if values is not None else default
        def operate(operation, first, second, pointer, size):
            try:
                events.append((operation,first,second,pointer,size))
                attempt=state['attempt']
                if operation==1:
                    state.update(attempt=attempt+1,phase='write',poll=0)
                    self.assertEqual((first,second,pointer,size),(port,sector,address,128))
                    sent.append(bytes(box.data))
                    return choose(writes,attempt+1,1)
                if operation==2:
                    self.assertEqual(first,port>>4)
                    sequence=(statuses or {}).get(state['phase'],[1])
                    result=sequence[min(state['poll'],len(sequence)-1)]
                    state['poll']+=1
                    if result&1 and state['phase']=='read':
                        C.memmove(state['output'],state['read_data'],128)
                    return result
                if operation==3:
                    self.assertEqual(size,128)
                    self.assertEqual(C.string_at(pointer,128),bytes(128))
                    self.assertNotEqual(pointer,address)
                    readbacks.append(pointer)
                    return 0
                if operation==4:
                    self.assertEqual((first,second,size),(port,sector,128))
                    self.assertEqual(pointer,readbacks[-1])
                    result=choose(reads,attempt,1)
                    value=choose(contents,attempt,sent[-1])
                    self.assertEqual(len(value),128)
                    state.update(phase='read',poll=0,output=pointer,read_return=result,read_data=bytes(value))
                    if result!=1:C.memmove(pointer,value,128)
                    if on_read:on_read(box,attempt,pointer)
                    return result
                if operation==5:
                    diagnostics.append(C.string_at(pointer))
                    self.assertEqual(diagnostics[-1],self.message)
                    if on_diagnostic:on_diagnostic(box,state['output'])
                    return 123
                self.assertEqual(operation,0)
                return 0
            except BaseException as error:
                errors.append(error)
                # Let status loops finish and stop at the next write if a callback failed.
                return 1 if operation==2 else 0
        callback=Operation(operate)
        C.c_void_p.in_dll(self.lib,'test_operation').value=C.cast(callback,C.c_void_p).value
        result=self.function(port,sector,address)
        if errors:raise errors[0]
        self.assertEqual(list(box.before),[0xA5A5A5A5]*2)
        self.assertEqual(list(box.after),[0xA5A5A5A5]*2)
        if C.sizeof(C.c_void_p)>4:
            self.assertGreater(address,0xFFFFFFFF)
            for pointer in readbacks:self.assertGreater(pointer,0xFFFFFFFF)
        return result,box,events,sent,diagnostics

    def test_every_checksum_byte_and_signed_port_sector_patterns(self):
        for byte in range(256):
            data=bytes([byte])+bytes(126)+bytes([0xA5])
            result,box,events,sent,diagnostics=self.transfer(data,port=C.c_int32(0xFFFFFF00|byte).value,sector=C.c_int32(0x80000000|byte).value)
            self.assertEqual(result,1)
            self.assertEqual(bytes(box.data),bytes([byte])+bytes(126)+bytes([byte]))
            self.assertEqual(sent,[bytes(box.data)])
            self.assertEqual([event[0] for event in events],[0,1,2,3,0,4,2])
            self.assertEqual(diagnostics,[])

    def test_each_attempt_can_succeed_or_all_eight_can_mismatch(self):
        data=bytes([1])+bytes(127)
        bad=bytes([2])+bytes(127);good=bytes([1])+bytes(127)
        for success in range(1,10):
            sequence=[bad]*8
            if success<=8:sequence[success-1]=good
            result,box,events,sent,_=self.transfer(data,contents=sequence)
            self.assertEqual(result,int(success<=8))
            self.assertEqual(len(sent),min(success,8))
            self.assertEqual(bytes(box.data),bytes([1])+bytes(126)+b'\x01')
            self.assertEqual([event[0] for event in events],[0,1,2,3,0,4,2]*min(success,8))

    def test_write_failure_exits_immediately_without_retry_or_readback(self):
        for value in (0,2,-1,-0x80000000,0x7FFFFFFF):
            result,box,events,sent,_=self.transfer(bytes(range(128)),writes=[value])
            self.assertEqual(result,0)
            self.assertEqual([event[0] for event in events],[0,1])
            self.assertEqual(len(sent),1)
            self.assertEqual(box.data[127],self.checksum(bytes(range(128))))

    def test_status_busy_bits_error_completion_and_checksum_only_verification(self):
        data=bytearray(range(128));readback=bytearray(data)
        readback[0]^=0x80;readback[1]^=0x80;readback[127]^=0xFF
        result,_,events,_,_=self.transfer(data,port=0x10,contents=[readback],statuses={'write':[0,2,4,0x11],'read':[0,8,0x21]})
        self.assertEqual(result,1)
        self.assertEqual([event[0] for event in events],[0,1,2,2,2,2,3,0,4,2,2,2])

    def test_failed_read_still_verifies_and_reloads_current_caller_checksum(self):
        for value in (0,2,-1):
            result,_,events,_,diagnostics=self.transfer(bytes(128),reads=[value],contents=[bytes(128)])
            self.assertEqual(result,1)
            self.assertEqual([event[0] for event in events],[0,1,2,3,0,4,5])
            self.assertEqual(diagnostics,[self.message])
        def change_checksum(box,attempt,pointer):box.data[127]=0x5A
        result,box,_,_,_=self.transfer(bytes(128),contents=[bytes([0x5A])+bytes(127)],on_read=change_checksum)
        self.assertEqual(result,1);self.assertEqual(box.data[127],0x5A)
        def change_after_diagnostic(box,pointer):
            C.c_uint8.from_address(pointer).value=0x33
            box.data[127]=0x33
        result,_,_,_,_=self.transfer(bytes(128),reads=[0],contents=[bytes(128)],on_diagnostic=change_after_diagnostic)
        self.assertEqual(result,1)

    def test_each_payload_position_contributes_but_readback_checksum_byte_is_ignored(self):
        for position in range(127):
            data=bytearray(128);data[position]=0xA7;data[127]=0xFF
            readback=bytearray(data);readback[127]=0
            result,box,_,sent,_=self.transfer(data,contents=[readback])
            self.assertEqual(result,1)
            self.assertEqual(box.data[127],0xA7)
            self.assertEqual(sent[0][:127],bytes(data[:127]))
