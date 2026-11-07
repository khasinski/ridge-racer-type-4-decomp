"""Check shutdown call ordering, mode selection and port writes on the host."""
import ctypes as C
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


Helper = C.CFUNCTYPE(C.c_int32, C.c_int32, C.c_int32)


class ShutdownTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        source = (root / 'tools/scripts/fixtures/views/func_8008F564.c').read_text()
        source = source.replace('#include "types.h"', '''#include <stdint.h>
typedef int8_t s8;
typedef int32_t s32;''')
        source += '''
s32 StUseAltCallbacks;
s8 *StCdReg0, *CD_REG3_RING;
s32 (*test_helper)(s32, s32);
s32 EnterCriticalSection(void) { return test_helper(0, 0); }
s32 CdAltDataCallback(s32 value) { return test_helper(1, value); }
s32 CdAltReadyCallback(s32 value) { return test_helper(2, value); }
s32 CdDataCallback(s32 value) { return test_helper(3, value); }
s32 CdReadyCallback(s32 value) { return test_helper(4, value); }
s32 ExitCriticalSection(void) { return test_helper(5, 0); }
'''
        (directory / 'function.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'function.c'),
                        '-o', str(directory / 'function.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'function.so'))
        cls.function = cls.library.StUnSetRing
        cls.function.argtypes = []
        cls.function.restype = None

    def check_shutdown(self, mode, mode_after_enter=None, alias=False):
        first, second = C.c_int8(-1), C.c_int8(127)
        C.c_int32.in_dll(self.library, 'StUseAltCallbacks').value = mode
        C.c_void_p.in_dll(self.library, 'StCdReg0').value = C.addressof(first)
        C.c_void_p.in_dll(self.library, 'CD_REG3_RING').value = C.addressof(first if alias else second)
        calls, ports_at_exit = [], []

        @Helper
        def helper(identity, value):
            calls.append((identity, value))
            if identity == 0 and mode_after_enter is not None:
                C.c_int32.in_dll(self.library, 'StUseAltCallbacks').value = mode_after_enter
            if identity == 5:
                ports_at_exit.append((first.value, second.value))
            return -99

        C.c_void_p.in_dll(self.library, 'test_helper').value = C.cast(helper, C.c_void_p).value
        self.function()
        selected = mode if mode_after_enter is None else mode_after_enter
        branch = [1, 2] if selected == 1 else [3, 4]
        self.assertEqual(calls, [(identity, 0) for identity in [0, *branch, 5]])
        expected_ports = (0, 127 if alias else 0)
        self.assertEqual((first.value, second.value), expected_ports)
        self.assertEqual(ports_at_exit, [expected_ports])

    def test_mode_values_and_aliasing_ports(self):
        for mode in (-(1 << 31), -1, 0, 1, 2, (1 << 31) - 1):
            for alias in (False, True):
                self.check_shutdown(mode, alias=alias)

    def test_mode_is_read_after_entering_critical_section(self):
        self.check_shutdown(0, mode_after_enter=1)
        self.check_shutdown(1, mode_after_enter=0)


if __name__ == '__main__':
    unittest.main()
