"""Actual matching loop, both real reset paths and fresh state after each reset."""
import ctypes as C
from itertools import product
from pathlib import Path
import random
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

Hook = C.CFUNCTYPE(None, C.c_int32, C.c_int32)


class ChannelMatchResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        header = (root/'include/rr/channels.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8;typedef int8_t s8;typedef uint16_t u16;typedef int16_t s16;
typedef uint32_t u32;typedef int32_t s32;
''' + header + '''
RrChannelControlRow36 test_rows[320];
u16 test_masks[0x10010/2];
s8 spuVmMaxVoice;u16 SvmVoiceIndex;u32 _snd_vmask;
#define _svm_voice test_rows
#define SvmVoicePitch ((u8 *)test_rows+4)
#define SvmVoiceKeyStatus ((u8 *)test_rows+0x1D)
#define _svm_okon1 test_masks[0xF8F8/2]
#define _svm_okon2 test_masks[0xF8FA/2]
#define _svm_okof1 test_masks[0x10000/2]
#define _svm_okof2 test_masks[0x10002/2]
void (*test_hook)(s32,s32);
s32 test_calls,test_indices[128],test_kinds[128];
void test_simple(u8 index) {
 test_indices[test_calls]=index;test_kinds[test_calls++]=0;
 vmNoiseOff(index);if(test_hook)test_hook(index,0);
}
void test_selected(s32 mode) {
 s32 index=SvmVoiceIndex;
 test_indices[test_calls]=index;test_kinds[test_calls++]=mode+1;
 _SsVmKeyOffNow(mode);if(test_hook)test_hook(index,1);
}
void test_reset(void) {
 memset(test_rows,0xA5,sizeof(test_rows));memset(test_masks,0,sizeof(test_masks));
 _svm_okon1=_svm_okon2=0xFFFF;
 spuVmMaxVoice=24;SvmVoiceIndex=31;_snd_vmask=0;test_calls=0;test_hook=0;
}
'''
        for name in ('80088D64','80088DA4','8008A644','80088340','80089E1C'):
            body = (root/('tools/scripts/fixtures/views/func_'+name+'.c')).read_text()
            body = re.sub(r'^#include[^\n]*\n|^extern[^\n]*\n', '', body, flags=re.M)
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+",\s*', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
            if name in ('80088D64','80088340'):
                assert body.count('(u8 *)0x80110000U') == (1 if name == '80088D64' else 4)
                body = body.replace('(u8 *)0x80110000U', '((u8 *)test_rows+0x6F8)')
            if name == '80088DA4':
                assert body.count('(u16 *)0x80110000U') == 1
                body = body.replace('(u16 *)0x80110000U', '(test_masks+0x10000/2)')
            if name == '80088340':
                assert body.count('vmNoiseOff(') == body.count('_SsVmKeyOffNow(') == 1
                body = body.replace('vmNoiseOff(', 'test_simple(').replace('_SsVmKeyOffNow(', 'test_selected(')
            if name == '80089E1C':
                assert body.count('_SsVmKeyOffNow(') == 1
                body = body.replace('_SsVmKeyOffNow(', 'test_selected(')
            source += body
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(source)
        result = subprocess.run(['cc','-shared','-fPIC','-O2',str(directory/'function.c'),
                                 '-o',str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.rows = (C.c_uint8*(320*54)).in_dll(cls.lib,'test_rows')
        cls.masks = (C.c_uint16*(0x10010//2)).in_dll(cls.lib,'test_masks')
        cls.count = C.c_int8.in_dll(cls.lib,'spuVmMaxVoice')
        cls.blocked = C.c_uint32.in_dll(cls.lib,'_snd_vmask')
        cls.selected = C.c_uint16.in_dll(cls.lib,'SvmVoiceIndex')
        cls.calls = C.c_int32.in_dll(cls.lib,'test_calls')
        cls.indices = (C.c_int32*128).in_dll(cls.lib,'test_indices')
        cls.kinds = (C.c_int32*128).in_dll(cls.lib,'test_kinds')
        cls.match = cls.lib._SsVmKeyOff
        cls.match.argtypes = [C.c_int32]*3+[C.c_uint32]
        cls.match.restype = C.c_int32
        cls.group_reset = cls.lib._SsVmSeqKeyOff
        cls.group_reset.argtypes = [C.c_int32]
        cls.group_reset.restype = None
        cls.set_count = cls.lib.SsSetReservedVoice
        cls.set_count.argtypes = [C.c_uint32]
        cls.set_count.restype = C.c_int32

    def setUp(self):
        self.lib.test_reset()
        self.errors = []
        self.hook = None

    def row(self,index,first=10,second=20,third=30,group=40,tag=255):
        for offset,value in ((0,tag),(0xE,group),(0x10,first),(0x14,third),(0x18,second)):
            struct.pack_into('<H',self.rows,index*54+offset,value&65535)

    def invoke(self,first=10,second=20,third=30,group=40):
        result = self.match(first,second,third,group)
        if self.errors:
            raise self.errors[0]
        return result

    def observe(self,action):
        def checked(index,kind):
            try:
                action(index,kind)
            except BaseException as error:
                self.errors.append(error)
                self.count.value=0
        self.hook = Hook(checked)
        C.c_void_p.in_dll(self.lib,'test_hook').value = C.cast(self.hook,C.c_void_p).value

    def test_all_valid_counts_both_real_resets_masks_and_full_guards(self):
        for count in range(1,25):
            self.lib.test_reset()
            self.assertEqual(self.set_count(count),count)
            for index in range(32):
                self.row(index,tag=255 if index%2==0 else -1)
            before = bytearray(self.rows)
            self.assertEqual(self.invoke(),count)
            self.assertEqual(list(self.indices[:count]),list(range(count)))
            self.assertEqual(list(self.kinds[:count]),[i%2 for i in range(count)])
            low=high=0
            for index in range(count):
                offset=index*54
                before[offset:offset+2]=b'\0\0'
                before[offset+4:offset+6]=b'\0\0'
                before[offset+0x1D]=0
                if index%2:
                    if index<16:low|=1<<index
                    else:high|=1<<(index-16)
            self.assertEqual(bytes(self.rows),bytes(before))
            self.assertEqual((self.masks[0x10000//2],self.masks[0x10002//2]),(low,high))
            self.assertEqual((self.masks[0xF8F8//2],self.masks[0xF8FA//2]),(65535^low,65535^high))
            self.assertEqual(self.selected.value, max((i for i in range(count) if i%2),default=31))

    def test_signed_parameter_narrowing_unsigned_group_and_each_predicate(self):
        self.count.value=1
        values=(-32768,-1,0,1,32767)
        for first,second,third in product(values,repeat=3):
            for group in (0,1,32767,32768,65535):
                self.row(0,first,second,third,group)
                self.calls.value=0
                self.assertEqual(self.invoke(first|0x12340000,second|0x45670000,third|0x23450000,group|0xABCD0000),int(group<32768))
        rng=random.Random(0x88340)
        for _ in range(500):
            self.lib.test_reset()
            expected=[]
            for index in range(24):
                fields=[10,20,30,40]
                mismatch=rng.randrange(6)
                if mismatch<4:fields[mismatch]+=1
                self.row(index,*fields,tag=rng.choice((255,-1,0,1,32767)))
                if rng.randrange(3)==0:self.blocked.value|=1<<index
                if mismatch>=4 and not (self.blocked.value&(1<<index)):expected.append(index)
            self.assertEqual(self.invoke(),len(expected))
            self.assertEqual(list(self.indices[:self.calls.value]),expected)

    def test_nonpositive_count_does_not_touch_rows_masks_or_selection(self):
        for count in range(-128,1):
            self.count.value=count
            before=(bytes(self.rows),bytes(self.masks),self.selected.value)
            self.assertEqual(self.invoke(-0x80000000,0x7FFFFFFF,-1,0xFFFFFFFF),0)
            self.assertEqual((bytes(self.rows),bytes(self.masks),self.selected.value),before)
            self.assertEqual(self.calls.value,0)

    def test_fresh_count_block_mask_and_row_fields_after_actual_reset(self):
        for first_tag in (255,1):
            self.lib.test_reset()
            self.count.value=1
            self.row(0,tag=first_tag)
            self.row(1,tag=1)
            self.row(2,tag=255)
            self.row(3,tag=1)
            self.row(4,first=99,tag=255)
            def changed(index,kind):
                self.assertEqual(kind,int(index in (0,3) and first_tag!=255) if index==0 else 0)
                if index==0:
                    self.count.value=5
                    self.blocked.value=1<<1
                    self.row(4,tag=255)
                elif index==2:
                    self.blocked.value|=1<<3
                elif index==4:
                    self.count.value=-1
            self.observe(changed)
            self.assertEqual(self.invoke(),3)
            self.assertEqual(list(self.indices[:self.calls.value]),[0,2,4])

    def invoke_group(self,group):
        self.group_reset(group)
        if self.errors:
            raise self.errors[0]

    def test_group_reset_all_counts_masks_and_complete_row_guards(self):
        for count in range(1,25):
            for blocked in (0,0xFFFFFFFF,0x55555555,0x80010001):
                self.lib.test_reset()
                self.assertEqual(self.set_count(count),count)
                self.blocked.value=blocked
                for index in range(32):
                    self.row(index,first=-123,tag=99)
                before=bytearray(self.rows)
                self.invoke_group(-123)
                expected=[index for index in range(count) if not blocked&(1<<index)]
                self.assertEqual(list(self.indices[:self.calls.value]),expected)
                self.assertEqual(list(self.kinds[:self.calls.value]),[1]*len(expected))
                low=high=0
                for index in expected:
                    offset=index*54
                    before[offset:offset+2]=b'\0\0'
                    before[offset+4:offset+6]=b'\0\0'
                    before[offset+0x1D]=0
                    if index<16:low|=1<<index
                    else:high|=1<<(index-16)
                self.assertEqual(bytes(self.rows),bytes(before))
                self.assertEqual((self.masks[0x10000//2],self.masks[0x10002//2]),(low,high))
                self.assertEqual((self.masks[0xF8F8//2],self.masks[0xF8FA//2]),(65535^low,65535^high))
                self.assertEqual(self.selected.value,expected[-1] if expected else 31)

    def test_group_reset_signed_low16_and_nonpositive_counts(self):
        self.count.value=1
        for low in range(65536):
            struct.pack_into('<H',self.rows,0x10,low)
            for blocked,wanted,expected in ((0,low,1),(1,low,0),(0,(low+1)&65535,0)):
                self.calls.value=0;self.selected.value=31;self.blocked.value=blocked
                self.invoke_group(C.c_int32(0xABCD0000|wanted).value)
                self.assertEqual(self.calls.value,expected)
                self.assertEqual(self.selected.value,0 if expected else 31)
        for count in range(-128,1):
            self.count.value=count;self.calls.value=0
            before=(bytes(self.rows),bytes(self.masks),self.selected.value)
            self.invoke_group(-0x80000000)
            self.assertEqual(self.calls.value,0)
            self.assertEqual((bytes(self.rows),bytes(self.masks),self.selected.value),before)

    def test_group_reset_reloads_count_block_mask_and_match_after_real_helper(self):
        self.count.value=1
        for index in range(5):self.row(index,first=7 if index!=4 else 99,tag=1)
        def changed(index,kind):
            self.assertEqual(kind,1)
            if index==0:
                self.assertEqual(self.set_count(5),5)
                self.blocked.value=1<<1
                struct.pack_into('<H',self.rows,4*54+0x10,7)
            elif index==2:
                self.blocked.value|=1<<3
            elif index==4:
                self.count.value=-1
        self.observe(changed)
        self.invoke_group(7)
        self.assertEqual(list(self.indices[:self.calls.value]),[0,2,4])
        self.assertEqual(self.selected.value,4)
        self.assertEqual(self.count.value,-1)
