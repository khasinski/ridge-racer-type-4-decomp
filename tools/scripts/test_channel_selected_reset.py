"""Execute the selected-channel reset gate, adapting only its PAL page address."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class ChannelSelectedResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        d = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        header = (root / 'include/rr/channels.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        body = (root / 'tools/scripts/fixtures/views/func_80085AEC.c').read_text()
        body = body.replace('#include "rr/channels.h"', '')
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r' : "\$[0-9]+"', '', body)
        assert body.count('(int *)0x80110000U') == 1
        body = body.replace('(int *)0x80110000U', '(test_ram + 0xFA4/4)')
        code = '''#include <stdint.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
int test_ram[0xFA4/4+1];
#define _snd_ev_flag test_ram[0]
u16 SvmVoiceIndex;
int calls, seen_mode, seen_lock, seen_selected, mode, nested;
void _SsVmKeyOffNow(s32 unused_mode) {
    calls++;seen_mode=unused_mode;seen_lock=_snd_ev_flag;seen_selected=SvmVoiceIndex;
    if(mode==1) nested=SsUtKeyOffV(7);
    if(mode==2) {_snd_ev_flag=-123;SvmVoiceIndex=0xBEEF;}
}
''' + body
        (d / 'f.c').write_text(code)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(d/'f.c'), '-o', str(d/'f.so')], check=True, capture_output=True)
        cls.lib = C.CDLL(str(d/'f.so'))
        cls.call = cls.lib.SsUtKeyOffV
        cls.call.argtypes = [C.c_int32]
        cls.call.restype = C.c_int32
        cls.ram = (C.c_int32 * (0xFA4//4+1)).in_dll(cls.lib, 'test_ram')
        cls.selected = C.c_uint16.in_dll(cls.lib, 'SvmVoiceIndex')

    def var(self, name):
        return C.c_int.in_dll(self.lib, name)

    def prepare(self, lock=0, mode=0):
        for i in range(len(self.ram)): self.ram[i] = 0x12345678
        self.ram[0] = lock
        self.selected.value = 0xCAFE
        for name in ('calls','seen_mode','seen_lock','seen_selected','nested'): self.var(name).value=-99
        self.var('calls').value=0
        self.var('mode').value=mode

    def test_every_low_halfword_and_upper_bits(self):
        self.prepare()
        for upper in (0, 0x1234, 0xFFFF):
            for low in range(65536):
                self.ram[0]=0;self.selected.value=0xCAFE;self.var('calls').value=0
                result=self.call(C.c_int32((upper<<16)|low).value)
                self.assertEqual(result, 0 if low<24 else -1)
                self.assertEqual(self.ram[0],0)
                self.assertEqual(self.selected.value, low if low<24 else 0xCAFE)
                self.assertEqual(self.var('calls').value, int(low<24))
                if low<24:
                    self.assertEqual((self.var('seen_mode').value,self.var('seen_lock').value,self.var('seen_selected').value),(0,1,low))

    def test_busy_gate_and_noncanonical_lock_values(self):
        for lock in (1, -1, 2, -2147483648, 2147483647):
            for selected in (0,23,24,65535,-65536):
                self.prepare(lock)
                before=list(self.ram)
                r=self.call(selected)
                if lock==1:
                    self.assertEqual((r,list(self.ram),self.selected.value,self.var('calls').value),(-1,before,0xCAFE,0))
                else:
                    self.assertEqual(r,0 if (selected&65535)<24 else -1)
                    self.assertEqual(self.ram[0],0)
                    self.assertEqual(list(self.ram)[1:],before[1:])

    def test_reentry_is_rejected_and_cleanup_follows_callback(self):
        self.prepare(mode=1)
        self.assertEqual(self.call(3),0)
        self.assertEqual((self.var('nested').value,self.var('calls').value,self.ram[0],self.selected.value),(-1,1,0,3))
        self.prepare(mode=2)
        self.assertEqual(self.call(4),0)
        self.assertEqual((self.ram[0],self.selected.value),(0,0xBEEF))
