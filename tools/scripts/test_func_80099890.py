"""Host-side behavioral checks; fd.py verifies the PSX layout and instructions."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


U8 = C.c_uint8


class State(C.Structure):
    _fields_ = [('pad00', U8 * 0x37), ('flag37', U8),
                ('pad38', U8 * 4), ('packet', C.POINTER(U8))]


Callback = C.CFUNCTYPE(C.c_int32, C.POINTER(State), C.c_int32)
Finish = C.CFUNCTYPE(C.c_int32, C.POINTER(State), U8)


class TransferResultTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        source = (root / 'tools/scripts/fixtures/legacy/func_80099890.c').read_text()
        source = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', source, flags=re.M)
        source = '#include <stdint.h>\ntypedef uint8_t u8;\ntypedef int32_t s32;\n' + source
        source += '''
s32 (*PadTxByteCallback)(RrTransferState *, s32);
s32 (*test_finish)(RrTransferState *, u8);
s32 _padSioRW2(RrTransferState *state, u8 value) {
    return test_finish(state, value);
}
'''
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'function.so'))
        cls.function = cls.library._padTxByteChkAck
        cls.function.argtypes = [C.POINTER(State)]
        cls.function.restype = C.c_int32

    def check_transfer(self, header, flag, callback_result, finish_result):
        packet = (U8 * 1)(header)
        state = State()
        state.packet = packet
        state.flag37 = flag
        calls = []

        @Callback
        def callback(pointer, mode):
            calls.append(('callback', C.addressof(pointer.contents), mode))
            return callback_result

        @Finish
        def finish(pointer, value):
            calls.append(('finish', C.addressof(pointer.contents), value))
            return finish_result

        C.c_void_p.in_dll(self.library, 'PadTxByteCallback').value = C.cast(callback, C.c_void_p).value
        C.c_void_p.in_dll(self.library, 'test_finish').value = C.cast(finish, C.c_void_p).value
        before = C.string_at(C.addressof(state), C.sizeof(state))
        expected = finish_result if finish_result <= 0 or finish_result == 0x5A else -4
        self.assertEqual(self.function(C.byref(state)), expected)
        mode = int(0x80 <= header <= 0x8F and flag == 0)
        self.assertEqual(calls, [('callback', C.addressof(state), mode),
                                 ('finish', C.addressof(state), callback_result & 0xFF)])
        self.assertEqual(C.string_at(C.addressof(state), C.sizeof(state)), before)
        self.assertEqual(packet[0], header)

    def test_all_header_values_and_flag_states(self):
        for header in range(256):
            for flag in (0, 1, 255):
                self.check_transfer(header, flag, 0x12345634, 0)

    def test_byte_truncation_and_result_boundaries(self):
        for callback_result in (-(1 << 31), -257, -1, 0, 255, 256, (1 << 31) - 1):
            for finish_result in (-(1 << 31), -5, -4, -1, 0, 1, 89, 90, 91, (1 << 31) - 1):
                self.check_transfer(0x81, 0, callback_result, finish_result)


if __name__ == '__main__':
    unittest.main()
