"""Actual grouped SPU selection/accessors: native pointers and publication aliases."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class SpuProgramSelectionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        header = (root/'include/rr/spu.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef int16_t s16;
typedef uint32_t u32;typedef int32_t s32;
''' + header + '''
u8 test_flag_storage[65536+0x200];
#define test_flag_page (test_flag_storage+32768)
void *test_flags(void) {return test_flag_page;}
u8 *test_data_page[0x60/sizeof(u8 *)+16];
RrSpuProgram *test_program_page[0x20/sizeof(RrSpuProgram *)+16];
u8 *_svm_vab_tn[16];
RrSpuProgram test_programs[65536+16];
u8 test_data[16][8],test_tones[16][8];
s16 kMaxPrograms;
typedef union {RrSpuProgram program;struct {u8 pad[8];RrSpuSelectionPrefix state;} view;} SelectionAlias;
typedef union {RrSpuProgram program;struct {u8 pad[8];u8 *pointer;} view;} DataAlias;
typedef union {RrSpuProgram program;struct {u8 pad[8];RrSpuProgram *pointer;} view;} ProgramAlias;
SelectionAlias test_selection;
DataAlias test_selected_data,test_selected_tones;
ProgramAlias test_selected_program;
#define SvmCurVabId test_selection.view.state
#define _svm_vh test_selected_data.view.pointer
#define _svm_tn test_selected_tones.view.pointer
#define _svm_pg test_selected_program.view.pointer
void *test_current(s32 which) {
 if(which==0)return _svm_vh;
 if(which==1)return _svm_pg;
 return _svm_tn;
}
void *test_expected(s32 bank,s32 which) {
 if(which==0)return test_data[bank];
 if(which==1)return test_programs+32768+bank;
 return test_tones[bank];
}
void *test_program(s32 bank,s32 index) {return test_programs+32768+bank+index;}
void *test_alias(s32 which) {
 if(which==0)return &test_selection.program;
 if(which==1)return &test_selected_data.program;
 if(which==2)return &test_selected_program.program;
 return &test_selected_tones.program;
}
void test_current_table(RrSpuProgram *program) {_svm_pg=program;}
void *test_pointer_storage(void) {return &_svm_pg;}
void test_table(s32 bank,RrSpuProgram *program) {
 test_program_page[0x20/sizeof(RrSpuProgram *)+bank]=program;
}
void test_reset(void) {
 s32 i;
 memset(test_flag_storage,0xA5,sizeof(test_flag_storage));
 memset(test_programs,0xA5,sizeof(test_programs));
 memset(&test_selection,0xA5,sizeof(test_selection));
 memset(test_data_page,0xA5,sizeof(test_data_page));
 memset(test_program_page,0xA5,sizeof(test_program_page));
 for(i=0;i<16;i++) {
  test_flag_page[0x120+i]=1;
  test_data_page[0x60/sizeof(u8 *)+i]=test_data[i];
  test_program_page[0x20/sizeof(RrSpuProgram *)+i]=test_programs+32768+i;
  _svm_vab_tn[i]=test_tones[i];
 }
 kMaxPrograms=32767;
 _svm_vh=test_data[0];_svm_pg=test_programs+32768;_svm_tn=test_tones[0];
}
'''
        body = ((root/'tools/scripts/fixtures/views/func_8008A564.c').read_text() +
                (root/'src/main/PAL/psyq/libsnd/ssatr2.c').read_text() +
                (root/'tools/scripts/fixtures/views/func_80084FB4.c').read_text())
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+",\s*', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
        assert body.count('=0x80110000U;') == 2
        body = body.replace('=0x80110000U;', '=(unsigned long)test_flag_page;')
        assert body.count('(u8 **)0x80110000U') == 1
        body = body.replace('(u8 **)0x80110000U', 'test_data_page')
        assert body.count('(RrSpuProgram **)0x80110000U') == 1
        body = body.replace('(RrSpuProgram **)0x80110000U', 'test_program_page')
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(source+body)
        result = subprocess.run(['cc','-shared','-fPIC','-O2',str(directory/'function.c'),
                                 '-o',str(directory/'function.so')],capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.lib.test_flags.restype=C.c_void_p
        cls.flag_address=cls.lib.test_flags()
        cls.flags = (C.c_uint8*0x200).from_address(cls.flag_address)
        cls.programs = (C.c_uint8*((65536+16)*16)).in_dll(cls.lib,'test_programs')
        cls.selection = (C.c_uint8*16).in_dll(cls.lib,'test_selection')
        cls.limit = C.c_int16.in_dll(cls.lib,'kMaxPrograms')
        cls.select = cls.lib._SsVmVSetUp
        cls.select.argtypes = [C.c_int32,C.c_int32]
        cls.select.restype = C.c_int32
        cls.lib.test_current.argtypes = [C.c_int32]
        cls.lib.test_expected.argtypes = [C.c_int32,C.c_int32]
        cls.lib.test_program.argtypes = [C.c_int32,C.c_int32]
        cls.lib.test_alias.argtypes = [C.c_int32]
        for name in ('test_current','test_expected','test_program','test_alias'):
            getattr(cls.lib,name).restype = C.c_void_p
        cls.lib.test_table.argtypes = [C.c_int32,C.c_void_p]
        cls.lib.test_current_table.argtypes=[C.c_void_p]
        cls.lib.test_current_table.restype=None
        cls.lib.test_pointer_storage.restype=C.c_void_p
        cls.copy=cls.lib.SsUtGetProgAtr
        cls.copy.argtypes=[C.c_int32,C.c_int16,C.POINTER(C.c_uint8)]
        cls.copy.restype=C.c_int32
        cls.accessors = [getattr(cls.lib,'func_'+name) for name in ('80089624','80089694','800896E4','80089754')]
        for index, function in enumerate(cls.accessors):
            function.argtypes = [C.c_int16,C.c_int16]+([C.c_int32] if index%2==0 else [])
            function.restype = C.c_int32

    def setUp(self):
        self.lib.test_reset()

    def snapshot(self):
        return bytes(self.selection),tuple(self.lib.test_current(i) for i in range(3))

    def test_all_banks_flags_limits_signed_programs_and_high_selector_words(self):
        for bank in range(16):
            for flag in (0,1,2,255):
                self.flags[0x120+bank]=flag
                for limit in (-32768,-1,0,1,32767):
                    self.limit.value=limit
                    for program in (-32768,-32767,-1,0,1,32766,32767):
                        for high in (0,0xFEDC0000):
                            before=self.snapshot()
                            raw_bank=C.c_int32(high|bank).value
                            raw_program=C.c_int32(high|(program&65535)).value
                            accepted=flag==1 and program<limit
                            self.assertEqual(self.select(raw_bank,raw_program),0 if accepted else -1)
                            if accepted:
                                self.assertEqual(tuple(self.lib.test_current(i) for i in range(3)),
                                                 tuple(self.lib.test_expected(bank,i) for i in range(3)))
                                self.assertEqual(list(self.selection[8:15]),[bank,0xA5,0xA5,0xA5,0xA5,program&255,0xA5])
                            else:
                                self.assertEqual(self.snapshot(),before)
        self.assertEqual(bytes(self.programs),b'\xA5'*len(self.programs))
        self.assertEqual(bytes(self.flags[:0x120]),b'\xA5'*0x120)
        self.assertEqual(bytes(self.flags[0x130:]),b'\xA5'*0xD0)
        for bank in (-1,16,65535,0x12340010,0x7FFFFFFF):
            before=self.snapshot()
            self.assertEqual(self.select(bank,-32768),-1)
            self.assertEqual(self.snapshot(),before)

    def test_every_program_byte_and_selected_attribute_value(self):
        for bank in (0,7,15):
            for program in range(256):
                for value in range(256):
                    address=self.lib.test_program(bank,program)
                    C.c_uint8.from_address(address+8).value=value
                    self.assertEqual(self.select(bank,0x12340000|program),0)
                    self.assertEqual(self.selection[14],value)
                    self.assertEqual(self.selection[13],program)

    def test_attribute_read_occurs_after_selector_and_all_pointer_publications(self):
        for which in range(4):
            for bank in (0,1,15):
                self.lib.test_reset()
                alias=self.lib.test_alias(which)
                self.lib.test_table(bank,alias)
                self.assertEqual(self.select(0x12340000|bank,0x56780000),0)
                expected=(bank if which==0 else
                          ((self.lib.test_expected(bank,0) if which==1 else
                            alias if which==2 else self.lib.test_expected(bank,2))&255))
                self.assertEqual(self.selection[14],expected)
                self.assertEqual(self.lib.test_current(0),self.lib.test_expected(bank,0))
                self.assertEqual(self.lib.test_current(1),alias)
                self.assertEqual(self.lib.test_current(2),self.lib.test_expected(bank,2))

    def test_real_grouped_accessors_signed_prefix_and_byte_truncation(self):
        for program in (-32768,-1,0,1,32766):
            for bank in (0,15):
                address=self.lib.test_program(bank,program)
                for field, setter, getter in ((1,self.accessors[0],self.accessors[1]),
                                             (4,self.accessors[2],self.accessors[3])):
                    for byte in range(256):
                        before=bytearray(C.string_at(address,16))
                        self.assertEqual(setter(bank,program,C.c_int32(0x80000000|byte).value),byte)
                        before[field]=byte
                        self.assertEqual(C.string_at(address,16),bytes(before))
                        self.assertEqual(getter(bank,program),byte)
        self.flags[0x120]=0
        before=self.snapshot(),bytes(self.programs)
        for setter in (self.accessors[0],self.accessors[2]):
            self.assertEqual(setter(0,0,123),-1)
        for getter in (self.accessors[1],self.accessors[3]):
            self.assertEqual(getter(0,0),-1)
        self.assertEqual((self.snapshot(),bytes(self.programs)),before)


    def attribute_buffer(self):
        # Halfword backing guarantees the original SH at offset6 is aligned.
        storage=(C.c_uint16*20)()
        C.memset(C.addressof(storage),0xA5,C.sizeof(storage))
        output=C.cast(C.addressof(storage)+16,C.POINTER(C.c_uint8))
        return storage,output

    def expected_attributes(self,before,source):
        expected=bytearray(before)
        expected[16:21]=source[:5]
        expected[22:24]=source[6:8]
        return bytes(expected)

    def test_program_copy_all_flags_signed_banks_and_ignored_selection_failure(self):
        for bank in (-32768,-1,0,7,15,16,32767):
            flag=C.c_uint8.from_address(self.flag_address+0x120+bank)
            for value in range(256):
                flag.value=value
                previous=self.lib.test_current(1)
                selected=self.lib.test_expected(bank,1) if 0<=bank<16 else previous
                record=(C.c_uint8*16).from_address(selected)
                record[:]=bytes((bank+j*37)&255 for j in range(16))
                source=bytes(record);storage,output=self.attribute_buffer()
                before=bytes(storage);state=self.snapshot()
                result=self.copy(C.c_int32(0xABCD0000|(bank&65535)).value,0,output)
                self.assertEqual(result,0 if value==1 else -1)
                self.assertEqual(bytes(storage),self.expected_attributes(before,source) if value==1 else before)
                self.assertEqual(bytes(record),source)
                if value!=1:
                    self.assertEqual(self.snapshot(),state)
                    self.assertEqual(self.copy(bank,0,None),-1)
        self.lib.test_reset()
        self.limit.value=0
        for program in (0,1,32767):
            address=self.lib.test_program(0,program)
            source=bytes((program+j*29)&255 for j in range(16))
            C.memmove(address,source,16)
            storage,output=self.attribute_buffer();before=bytes(storage);state=self.snapshot()
            self.assertEqual(self.copy(0,program,output),0)
            self.assertEqual(bytes(storage),self.expected_attributes(before,source))
            self.assertEqual(self.snapshot(),state)

    def test_program_copy_every_signed_program_and_preserved_byte5(self):
        self.limit.value=1
        for program in range(-32768,32768):
            address=self.lib.test_program(0,program)
            source=bytes((program+j*73)&255 for j in range(16))
            C.memmove(address,source,16)
            storage,output=self.attribute_buffer();before=bytes(storage)
            self.assertEqual(self.copy(0x12340000,program,output),0)
            self.assertEqual(bytes(storage),self.expected_attributes(before,source))
            self.assertEqual(C.string_at(address,16),source)

    def test_program_copy_live_pointer_alias_and_overlapping_record(self):
        self.flags[0x130]=1  # Bank16 passes this copy's flag gate but selector rejects it.
        first=self.lib.test_program(0,0)
        second=first+16 if first&255<=223 else first-16
        self.lib.test_current_table(first)
        first_bytes=(C.c_uint8*16).from_address(first)
        second_bytes=(C.c_uint8*16).from_address(second)
        first_bytes[:]=b'\xA7'*16;second_bytes[:]=b'\xD3'*16
        pointer_bytes=second.to_bytes(C.sizeof(C.c_void_p),'little')
        first_bytes[0]=pointer_bytes[0]
        for index in (1,2,3,4,6,7):second_bytes[index]=pointer_bytes[index]
        storage=self.lib.test_pointer_storage()
        before=bytes(self.programs)
        self.assertEqual(self.copy(16,0,C.cast(storage,C.POINTER(C.c_uint8))),0)
        self.assertEqual(self.lib.test_current(1),second)
        self.assertEqual(bytes(self.programs),before)
        for shift in (-4,-2,0,2,4):
            self.lib.test_current_table(self.lib.test_expected(0,1))
            source=self.lib.test_program(0,0)
            C.memmove(source,bytes(range(16)),16)
            destination=source+shift
            before=bytearray(self.programs)
            base=C.addressof(self.programs)
            src=source-base;dst=destination-base
            for index in (0,1,2,3,4):before[dst+index]=before[src+index]
            before[dst+6:dst+8]=before[src+6:src+8]
            self.assertEqual(self.copy(16,0,C.cast(destination,C.POINTER(C.c_uint8))),0)
            self.assertEqual(bytes(self.programs),bytes(before))
