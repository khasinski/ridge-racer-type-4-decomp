"""Exercise all recovered command initializers with shared types and real bodies."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

from c_functions import source_owners

# Constructor, command code, read callback, write callback, parameter globals.
CASES = [
    ('8009D198', 7, '8009D118', '8009C610', ('CardCmdValue',)),
    ('8009D348', 8, '8009D3E8', '8009C610', ('CardCmdSector',)),
    ('8009D5B4', 100, '8009D65C', '8009D680', ('CardCmdResultPtrA', 'CardCmdResultPtrB')),
    ('8009D700', 12, '8009D7A0', '8009D7C4', ('CardCmdBuffer',)),
    ('8009DD88', 101, '8009DE30', '8009DE54', ('CardCmdResultPtrA', 'CardCmdResultPtrB')),
    ('8009D238', 13, '8009D2E0', '8009C610', ('CardCmdSector', 'CardVarRxLength')),
    ('8009CB84', 14, '8009CC44', '8009CCF0', ('CardCmdSector', 'CardCmdBuffer', 'CardCmdResultPtrA')),
    ('8009D068', 6, '8009D118', '8009C610', ('CardCmdSector', 'CardVarRxLength')),
    ('8009CDFC', 15, '8009CEBC', '8009CF94', ('CardCmdSector', 'CardCmdBuffer', 'CardCmdResultPtrA')),
    ('8009C638', 3, '8009C6D8', '8009C6F8', ('CardCmdBuffer',)),
]
SCALARS = ['CardTxCount', 'CardCommandRequest', 'CardReceiveIndex', 'CardRxCount',
           'CardCmdSector', 'CardVarRxLength', 'CardCmdValue', 'CardBytesRemaining', 'CardCurrentCommand']
POINTERS = ['CardCmdBuffer', 'CardCmdResultPtrA', 'CardCmdResultPtrB', 'CardCommandReader', 'CardWriteHandler']


NA = lambda n: __import__('c_functions').name_at(int(n, 16) | 0x80000000)


class Parameter(C.Union):
    _fields_ = [('value', C.c_int32), ('buffer', C.c_void_p)]


class CommandInitializersTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        asm_header = (root / 'include/rr_asm.h').read_text()
        header = (root / 'include/rr/commands.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n|^extern[^\n]*\n', '', header, flags=re.M)
        # Place flag storage in a full mapped page, so the negative page offset
        # remains within the same native object. Other globals keep native pointers.
        prefix = '''#include <stdint.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef int32_t s32;
u32 test_ram[0x4000];
s32 test_parameter_page[0x4000];
#define CardCmdSector test_parameter_page[0x4724 / 4]
#define CardCommandStatus test_ram[0xBDE4 / 4]
u8 *CardCmdBuffer;
void *CardCmdResultPtrA;
u32 *CardCmdResultPtrB;
s32 test_callback_calls;
''' + '\n'.join('s32 ' + name + ';' for name in SCALARS if name not in ('CardCmdValue', 'CardCmdSector')) + '''
s32 (*CardCommandReader)(void);
s32 (*CardWriteHandler)(s32);
'''
        callbacks = ''
        for n in sorted(({row[2] for row in CASES} | {'8009C55C', '8009CA8C', '8009C87C'}) - {'8009D118'}):
            callbacks += f's32 {NA(n)}(void) {{ test_callback_calls++; return 1; }}\n'
        for n in sorted(({row[3] for row in CASES} | {'8009CB50', '8009C91C'}) - {'8009CF94', '8009C91C', '8009C6F8', '8009CCF0'}):
            callbacks += f's32 {NA(n)}(s32 value) {{ test_callback_calls++; return value; }}\n'
        serial = (root / 'include/rr/serial.h').read_text()
        serial = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', serial, flags=re.M)
        source = prefix + header + serial + asm_header + '''
RrSerialPortRegisters test_serial_ports[2];
RrSerialPortRegisters *volatile CardCmdSioRegs = &test_serial_ports[0];
RrCommandParameter CardCmdValue;
s32 test_validation_calls;
s32 test_validation_results[2];
s32 test_validation_modes[2], test_validation_values[2];
s32 func_8009DF14(s32 mode, s32 value) {
    s32 index = test_validation_calls++;
    if (index >= 2) return 0;
    test_validation_modes[index] = mode;
    test_validation_values[index] = value;
    return test_validation_results[index];
}
''' + callbacks
        owners = {n: root / 'tools/scripts/fixtures/legacy' / (n + '.c') for n in [f'func_{r[0]}' for r in CASES] + ['_cardCmd59ParamBegin', '_cardCmd5CRangeBegin', '_cardCmd5BRangeBegin', '_cardCmd5DTx', '_cardCmd5CVarRx', '_cardCmd5BRangeRx', '_cardCmd5BRx', '_cardCmd5BVarRx']}
        sources = dict.fromkeys([owners['func_' + row[0]] for row in CASES] + [owners[n] for n in ('_cardCmd59ParamBegin', '_cardCmd5CRangeBegin', '_cardCmd5BRangeBegin', '_cardCmd5DTx', '_cardCmd5CVarRx', '_cardCmd5BRangeRx', '_cardCmd5BRx', '_cardCmd5BVarRx')])
        for path in {q.read_text(): q for q in sources}.values():
            body = path.read_text()
            body = body.replace('#include "rr_asm.h"', '')
            body = body.replace('#include "rr/commands.h"', '').replace('#include "rr/serial.h"', '')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+", ', '', body)
            body = re.sub(r' : "\$[0-9]+"', '', body)
            body = body.replace('(s32 *)0x800B0000U', '(s32 *)(test_ram + 0x4000)')
            body = body.replace('(s32 *)0x80110000U', 'test_parameter_page')
            source += body
        (directory / 'functions.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'functions.c'),
                        '-o', str(directory / 'functions.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'functions.so'))
        cls.serial_ports = (C.c_uint8 * 32).in_dll(cls.library, 'test_serial_ports')
        cls.serial_pointer = C.c_void_p.in_dll(cls.library, 'CardCmdSioRegs')
        cls.ram = (C.c_uint32 * 0x4000).in_dll(cls.library, 'test_ram')
        cls.parameter_page = (C.c_int32 * 0x4000).in_dll(cls.library, 'test_parameter_page')
        cls.parameter = Parameter.in_dll(cls.library, 'CardCmdValue')
        cls.scalars = {name: C.c_int32.from_buffer(cls.parameter_page, 0x4724) if name == 'CardCmdSector' else C.c_int32.in_dll(cls.library, name) for name in SCALARS}
        cls.validated_start = cls.library._cardCmd59ParamBegin
        cls.validated_start.argtypes = [C.c_int32, C.c_int32, C.c_void_p]
        cls.validated_start.restype = C.c_int32
        cls.range_starts = []
        for name in ('8009C768', '8009C978'):
            function = getattr(cls.library, NA(name))
            function.argtypes = [C.c_int32, C.c_void_p, C.c_int32, C.c_int32]
            function.restype = C.c_int32
            cls.range_starts.append(function)
        cls.validation_calls = C.c_int32.in_dll(cls.library, 'test_validation_calls')
        cls.validation_results = (C.c_int32 * 2).in_dll(cls.library, 'test_validation_results')
        cls.validation_modes = (C.c_int32 * 2).in_dll(cls.library, 'test_validation_modes')
        cls.validation_values = (C.c_int32 * 2).in_dll(cls.library, 'test_validation_values')
        cls.pointers = {name: C.c_void_p.in_dll(cls.library, name) for name in POINTERS}
        cls.calls = C.c_int32.in_dll(cls.library, 'test_callback_calls')
        cls.functions = {}
        for n, code, read, write, fields in CASES:
            fn = getattr(cls.library, NA(n))
            fn.argtypes = [C.c_int32] + [C.c_void_p if field in POINTERS else C.c_int32 for field in fields]
            fn.restype = C.c_int32
            cls.functions[n] = fn
        cls.buffer = (C.c_uint8 * 64)(*range(64))
        cls.words = (C.c_uint32 * 16)(*range(16))

    def initialize(self, flags):
        C.memset(C.addressof(self.serial_ports), 0xA5, C.sizeof(self.serial_ports))
        C.c_uint16.from_buffer(self.serial_ports, 4).value = 2
        C.c_uint16.from_buffer(self.serial_ports, 20).value = 2
        self.serial_pointer.value = C.addressof(self.serial_ports)
        C.memset(C.addressof(self.parameter_page), 0xA5, C.sizeof(self.parameter_page))
        C.memset(C.addressof(self.ram), 0xA5, C.sizeof(self.ram))
        self.ram[0xBDE4 // 4] = flags
        for index, value in enumerate(self.scalars.values()):
            value.value = -1000 - index
        for index, value in enumerate(self.pointers.values()):
            value.value = C.addressof(self.buffer) + index
        self.calls.value = 0
        self.validation_calls.value = 0
        self.validation_results[:] = (1, 1)
        self.validation_modes[:] = (-1, -1)
        self.validation_values[:] = (-1, -1)

    def snapshot(self):
        return (bytes(self.ram), {n: v.value for n, v in self.scalars.items()},
                {n: v.value for n, v in self.pointers.items()})

    def arguments(self, fields, value):
        return [C.addressof(self.buffer) if n == 'CardCmdBuffer' else
                C.addressof(self.words) + 4 * i if n in POINTERS else value
                for i, n in enumerate(fields)]

    def test_real_parameter_reader_selects_full_width_fields_without_state_changes(self):
        reader = self.library._cardCmd5DTx
        reader.argtypes = []
        reader.restype = C.c_int32
        for phase in (-2147483648, -1, *range(16), 0x12340002, 2147483647):
            for first, second, third in [(0, -1, 2147483647),
                                          (-2147483648, 0x12345678, -77),
                                          (65536, -2, 128)]:
                self.initialize(1)
                self.scalars['CardTxCount'].value = phase
                self.scalars['CardCmdSector'].value = first
                self.parameter.value = second
                self.scalars['CardVarRxLength'].value = third
                before = self.snapshot(), bytes(self.parameter_page), bytes(self.parameter)
                expected = {2: 0x5D, 4: first, 5: second, 6: third}.get(phase, 0)
                self.assertEqual(reader(), expected)
                self.assertEqual((self.snapshot(), bytes(self.parameter_page), bytes(self.parameter)), before)
                self.assertEqual(self.calls.value, 0)

    def test_real_constructors_install_and_use_the_parameter_reader(self):
        for name, arguments, expected in [
                ('8009D198', (-77, -2147483648), {2: 0x5D, 5: -2147483648}),
                ('8009D068', (123, -1, 2147483647), {2: 0x5D, 4: -1, 5: 0, 6: 2147483647})]:
            self.initialize(1)
            self.assertEqual(self.functions[name](*arguments), 1)
            self.assertEqual(self.pointers['CardCommandReader'].value,
                             C.cast(self.library._cardCmd5DTx, C.c_void_p).value)
            reader = C.CFUNCTYPE(C.c_int32)(self.pointers['CardCommandReader'].value)
            for phase, value in expected.items():
                self.scalars['CardTxCount'].value = phase
                before = self.snapshot(), bytes(self.parameter_page), bytes(self.parameter)
                self.assertEqual(reader(), value)
                self.assertEqual((self.snapshot(), bytes(self.parameter_page), bytes(self.parameter)), before)
            self.assertEqual(self.calls.value, 0)

    def test_fixed_response_writer_all_bytes_phases_and_canaries(self):
        writer = self.library._cardCmd5BRx
        writer.argtypes = [C.c_int32]
        writer.restype = C.c_int32
        for phase in (-2147483648, -1, *range(24)):
            for byte in range(256):
                self.initialize(1)
                self.pointers['CardCmdBuffer'].value = C.addressof(self.buffer)
                self.scalars['CardRxCount'].value = phase
                before = bytes(self.buffer)
                expected = bytearray(before)
                ram, scalars, pointers = self.snapshot()
                if phase == 5:
                    scalars['CardBytesRemaining'] = byte
                elif phase == 10:
                    expected[4] = (byte - 1) & 255
                elif phase >= 6:
                    expected[phase - 6] = byte
                self.assertEqual(writer(C.c_int32(0xFEDCBA00 | byte).value), 0)
                self.assertEqual(bytes(self.buffer), bytes(expected))
                self.assertEqual(self.snapshot(), (ram, scalars, pointers))

    def test_fixed_response_constructor_installs_real_writer(self):
        self.initialize(1)
        base = C.addressof(self.buffer)
        self.assertEqual(self.functions['8009C638'](17, base), 1)
        self.assertEqual(self.pointers['CardWriteHandler'].value,
                         C.cast(self.library._cardCmd5BRx, C.c_void_p).value)
        writer = C.CFUNCTYPE(C.c_int32, C.c_int32)(self.pointers['CardWriteHandler'].value)
        expected = bytearray(self.buffer)
        for phase, value in ((5, 0x123456AB), (6, -1), (7, 0x180), (10, -2147483648)):
            self.scalars['CardRxCount'].value = phase
            self.assertEqual(writer(value), 0)
            if phase == 10:
                expected[4] = 255
            elif phase >= 6:
                expected[phase - 6] = value & 255
        self.assertEqual(bytes(self.buffer), bytes(expected))
        self.assertEqual(self.scalars['CardBytesRemaining'].value, 0xAB)
        self.assertEqual(self.calls.value, 0)

    def test_serial_payload_response_constructor_length_completion_and_bytes(self):
        for count in (0, 1, 7, 31):
            for received in (0, 127, 255):
                self.initialize(1)
                base = C.addressof(self.buffer)
                first = base + 32
                self.assertEqual(self.functions['8009CB84'](23, 123, base, first), 1)
                self.assertEqual(self.pointers['CardWriteHandler'].value,
                                 C.cast(self.library._cardCmd5BVarRx, C.c_void_p).value)
                writer = C.CFUNCTYPE(C.c_int32, C.c_int32)(self.pointers['CardWriteHandler'].value)
                self.scalars['CardRxCount'].value = 3
                self.serial_ports[0] = count
                expected = bytearray(self.buffer)
                self.assertEqual(writer(-1), 0)
                self.assertEqual(self.scalars['CardVarRxLength'].value, count)
                self.assertEqual(self.pointers['CardCmdBuffer'].value, base)
                self.scalars['CardRxCount'].value = count + 4
                self.serial_ports[0] = received
                self.assertEqual(writer(-2147483648), 0)
                self.assertEqual(self.pointers['CardCmdBuffer'].value, first)
                self.assertEqual(self.parameter.value, 1)
                self.assertEqual(self.scalars['CardBytesRemaining'].value, received + 1)
                self.assertEqual(bytes(self.buffer), bytes(expected))
                serial_before = bytes(self.serial_ports)
                for index in range(32):
                    phase = count + 6 + index
                    value = C.c_int32(0xFEDCBA00 | (index * 29 & 255)).value
                    self.scalars['CardRxCount'].value = phase
                    ram, scalars, pointers = self.snapshot()
                    self.assertEqual(writer(value), 0)
                    expected[32 + index] = value & 255
                    self.assertEqual(bytes(self.buffer), bytes(expected))
                    self.assertEqual(self.snapshot(), (ram, scalars, pointers))
                    self.assertEqual(bytes(self.serial_ports), serial_before)
                self.assertEqual(self.calls.value, 0)

    def test_response_callbacks_inert_phases_allow_null_buffer_and_port(self):
        for name in ('8009C6F8', '8009CCF0'):
            writer = getattr(self.library, NA(name))
            writer.argtypes = [C.c_int32]
            writer.restype = C.c_int32
            for phase in (-2147483648, -1, 0, 1, 2, 4, 5):
                self.initialize(1)
                self.pointers['CardCmdBuffer'].value = None
                self.serial_pointer.value = None
                self.scalars['CardRxCount'].value = phase
                self.scalars['CardVarRxLength'].value = 200
                ram, scalars, pointers = self.snapshot()
                if name == '8009C6F8' and phase == 5:
                    scalars['CardBytesRemaining'] = 0
                self.assertEqual(writer(-2147483648), 0)
                self.assertEqual(self.snapshot(), (ram, scalars, pointers))
                self.assertIsNone(self.serial_pointer.value)

    def test_real_serial_response_length_reads_all_byte_values(self):
        writer = self.library._cardCmd5CVarRx
        writer.argtypes = [C.c_int32]
        writer.restype = C.c_int32
        for byte in range(256):
            self.initialize(1)
            self.scalars['CardRxCount'].value = 3
            self.serial_ports[0] = byte
            before, union = self.snapshot(), bytes(self.parameter)
            serial_before = bytes(self.serial_ports), self.serial_pointer.value
            ram, scalars, pointers = before
            scalars['CardVarRxLength'] = byte
            self.assertEqual(writer(-2147483648), 0)
            self.assertEqual(self.snapshot(), (ram, scalars, pointers))
            self.assertEqual(bytes(self.parameter), union)
            self.assertEqual((bytes(self.serial_ports), self.serial_pointer.value), serial_before)

    def test_real_serial_response_completion_preserves_full_buffer_pointer_and_byte_plus_one(self):
        writer = self.library._cardCmd5CVarRx
        writer.argtypes = [C.c_int32]
        writer.restype = C.c_int32
        for byte in range(256):
            self.initialize(1)
            self.scalars['CardVarRxLength'].value = 7
            self.scalars['CardRxCount'].value = 11
            self.serial_ports[0] = byte
            ram, scalars, pointers = self.snapshot()
            union = bytearray(self.parameter)
            union[:4] = (1).to_bytes(4, 'little')
            scalars['CardCmdValue'] = 1
            scalars['CardBytesRemaining'] = byte + 1
            pointers['CardCmdBuffer'] = pointers['CardCmdResultPtrA']
            serial_before = bytes(self.serial_ports), self.serial_pointer.value
            self.assertEqual(writer(0x12345678), 0)
            self.assertEqual(self.snapshot(), (ram, scalars, pointers))
            self.assertEqual(bytes(self.parameter), bytes(union))
            self.assertEqual((bytes(self.serial_ports), self.serial_pointer.value), serial_before)

    def test_serial_response_nonreading_phases_and_wrapped_count_boundaries(self):
        writer = self.library._cardCmd5CVarRx
        writer.argtypes = [C.c_int32]
        writer.restype = C.c_int32
        for count in (-2147483648, -1, 0, 2, 255, 2147483644, 2147483647):
            for phase in (-2147483648, -1, 0, 1, 2, 4, 5, 6, 255, 2147483647):
                if phase >= 4 and (phase & 0xFFFFFFFF) == ((count + 4) & 0xFFFFFFFF):
                    continue
                self.initialize(1)
                self.serial_pointer.value = None
                self.scalars['CardVarRxLength'].value = count
                self.scalars['CardRxCount'].value = phase
                before = self.snapshot(), bytes(self.parameter), bytes(self.serial_ports)
                self.assertEqual(writer(-1), 0)
                self.assertEqual((self.snapshot(), bytes(self.parameter), bytes(self.serial_ports)), before)
                self.assertIsNone(self.serial_pointer.value)

    def test_real_constructor_installs_serial_writer_for_length_and_completion(self):
        self.initialize(1)
        buffer = C.addressof(self.buffer)
        first = C.addressof(self.buffer) + 32
        self.assertEqual(self.functions['8009CDFC'](-77, 123, buffer, first), 1)
        self.assertEqual(self.pointers['CardWriteHandler'].value,
                         C.cast(self.library._cardCmd5CVarRx, C.c_void_p).value)
        writer = C.CFUNCTYPE(C.c_int32, C.c_int32)(self.pointers['CardWriteHandler'].value)
        self.scalars['CardRxCount'].value = 3
        self.serial_ports[0] = 2
        self.assertEqual(writer(123), 0)
        self.assertEqual(self.scalars['CardVarRxLength'].value, 2)
        self.assertEqual(self.pointers['CardCmdBuffer'].value, buffer)
        self.scalars['CardRxCount'].value = 6
        self.serial_ports[0] = 255
        self.assertEqual(writer(-1), 0)
        self.assertEqual(self.pointers['CardCmdBuffer'].value, first)
        self.assertEqual(self.scalars['CardCmdValue'].value, 1)
        self.assertEqual(self.scalars['CardBytesRemaining'].value, 256)
        self.assertEqual(self.calls.value, 0)

    def test_buffer_writer_counter_wrap_and_inactive_phases_need_no_buffer(self):
        writer = self.library._cardCmd5BRangeRx
        writer.argtypes = [C.c_int32]
        writer.restype = C.c_int32
        for count in (-2147483648, -1, 0, 1, 255, 2147483647):
            self.initialize(1)
            self.pointers['CardCmdBuffer'].value = None
            self.scalars['CardVarRxLength'].value = count
            for phase in (-2147483648, -1, *range(11)):
                self.scalars['CardRxCount'].value = phase
                ram, scalars, pointers = self.snapshot()
                union, before_buffer = bytes(self.parameter), bytes(self.buffer)
                if phase == 9:
                    scalars['CardBytesRemaining'] = C.c_int32((count & 0xFFFFFFFF) + 1).value
                self.assertEqual(writer(-1), 0)
                self.assertEqual(self.snapshot(), (ram, scalars, pointers))
                self.assertEqual((bytes(self.parameter), bytes(self.buffer)), (union, before_buffer))

    def test_buffer_writer_truncates_each_byte_and_preserves_other_context(self):
        writer = self.library._cardCmd5BRangeRx
        writer.argtypes = [C.c_int32]
        writer.restype = C.c_int32
        for offset in (0, 1, 31, 63):
            for byte in range(256):
                for high in (0, 0x12345600, 0xFFFFFF00):
                    self.initialize(1)
                    self.pointers['CardCmdBuffer'].value = C.addressof(self.buffer)
                    self.scalars['CardRxCount'].value = offset + 11
                    before, union = self.snapshot(), bytes(self.parameter)
                    expected = bytearray(self.buffer)
                    expected[offset] = byte
                    self.assertEqual(writer(C.c_int32(high | byte).value), 0)
                    self.assertEqual(bytes(self.buffer), bytes(expected))
                    self.assertEqual(self.snapshot(), before)
                    self.assertEqual(bytes(self.parameter), union)

    def test_real_range_constructor_installs_and_uses_buffer_writer(self):
        self.initialize(1)
        pointer = C.addressof(self.buffer)
        self.assertEqual(self.range_starts[0](-77, pointer, 123, 3), 1)
        self.assertEqual(self.pointers['CardWriteHandler'].value,
                         C.cast(self.library._cardCmd5BRangeRx, C.c_void_p).value)
        writer = C.CFUNCTYPE(C.c_int32, C.c_int32)(self.pointers['CardWriteHandler'].value)
        self.scalars['CardRxCount'].value = 9
        self.assertEqual(writer(123), 0)
        self.assertEqual(self.scalars['CardBytesRemaining'].value, 4)
        for phase, value, offset in [(11, 0x123456AB, 0), (74, -1, 63)]:
            self.scalars['CardRxCount'].value = phase
            expected = bytearray(self.buffer)
            expected[offset] = value & 255
            self.assertEqual(writer(value), 0)
            self.assertEqual(bytes(self.buffer), bytes(expected))
            self.assertEqual(self.scalars['CardBytesRemaining'].value, 4)
            self.assertEqual(self.scalars['CardRxCount'].value, phase)
        self.assertEqual(self.calls.value, 0)

    def test_disabled_gate_changes_nothing(self):
        for n, code, read, write, fields in CASES:
            for flags in (0, 2, 4, 0x80000000, 0xFFFFFFFE):
                self.initialize(flags)
                before = self.snapshot()
                self.assertEqual(self.functions[n](-123, *self.arguments(fields, -456)), 0)
                self.assertEqual(self.snapshot(), before)
                self.assertEqual(self.calls.value, 0)

    def test_ready_gate_resets_and_installs_only_the_selected_configuration(self):
        for n, code, read, write, fields in CASES:
            for flags in (1, 3, 5, 0x80000001, 0xFFFFFFFF):
                for value in (-2147483648, -1, 0, 1, 2147483647):
                    self.initialize(flags)
                    ram, scalars, pointers = self.snapshot()
                    expected_ram = bytearray(ram)
                    expected_ram[0xBDE4:0xBDE8] = (2).to_bytes(4, 'little')
                    for field in ('CardTxCount', 'CardReceiveIndex', 'CardRxCount', 'CardBytesRemaining'):
                        scalars[field] = 0
                    scalars['CardCommandRequest'] = value
                    scalars['CardCurrentCommand'] = code
                    pointers['CardCommandReader'] = C.cast(getattr(self.library, NA(read)), C.c_void_p).value
                    pointers['CardWriteHandler'] = C.cast(getattr(self.library, NA(write)), C.c_void_p).value
                    arguments = self.arguments(fields, value)
                    for field, argument in zip(fields, arguments):
                        (pointers if field in POINTERS else scalars)[field] = argument
                    if n in ('8009CB84', '8009CDFC'):
                        scalars['CardVarRxLength'] = scalars['CardCmdValue'] = 0
                    if n == '8009D068':
                        scalars['CardCmdValue'] = 0
                    before_buffer, before_words = bytes(self.buffer), bytes(self.words)
                    self.assertEqual(self.functions[n](value, *arguments), 1)
                    self.assertEqual(self.snapshot(), (bytes(expected_ram), scalars, pointers))
                    self.assertEqual((bytes(self.buffer), bytes(self.words)), (before_buffer, before_words))
                    self.assertEqual(self.calls.value, 0)

    def test_validated_start_rejects_large_parameter_before_any_write(self):
        for value in (16, 17, 65535, 2147483647):
            for flags in (0, 1, 2, 3):
                self.initialize(flags)
                before = self.snapshot(), bytes(self.parameter_page), bytes(self.parameter)
                self.assertEqual(self.validated_start(7, value, C.addressof(self.buffer)), 0)
                self.assertEqual((self.snapshot(), bytes(self.parameter_page), bytes(self.parameter)), before)
                self.assertEqual(self.calls.value, 0)

    def test_validated_start_publishes_parameter_before_readiness_gate(self):
        for value in (-2147483648, -123, -1, 0, 1, 15):
            for flags in (0, 2, 0xFFFFFFFE):
                self.initialize(flags)
                ram, scalars, pointers = self.snapshot()
                scalars['CardCmdSector'] = max(-1, value)
                before_union = bytes(self.parameter)
                page = bytearray(self.parameter_page)
                page[0x4724:0x4728] = max(-1, value).to_bytes(4, 'little', signed=True)
                self.assertEqual(self.validated_start(77, value, C.addressof(self.buffer)), 0)
                self.assertEqual(self.snapshot(), (ram, scalars, pointers))
                self.assertEqual(bytes(self.parameter), before_union)
                self.assertEqual(bytes(self.parameter_page), bytes(page))
                self.assertEqual(self.calls.value, 0)

    def test_validated_start_uses_full_native_buffer_pointer(self):
        for value in (-2147483648, -1, 0, 15):
            self.initialize(1)
            before_buffer = bytes(self.buffer)
            self.assertEqual(self.validated_start(-77, value, C.addressof(self.buffer)), 1)
            self.assertEqual(self.parameter.buffer, C.addressof(self.buffer))
            self.assertEqual(self.pointers['CardCmdBuffer'].value, C.addressof(self.buffer))
            self.assertEqual(self.scalars['CardCmdSector'].value, max(-1, value))
            self.assertEqual(self.scalars['CardCommandRequest'].value, -77)
            self.assertEqual(self.scalars['CardCurrentCommand'].value, 2)
            self.assertEqual(self.ram[0xBDE4 // 4], 2)
            for name in ('CardTxCount', 'CardReceiveIndex', 'CardRxCount', 'CardBytesRemaining'):
                self.assertEqual(self.scalars[name].value, 0)
            for field, target in [('CardCommandReader', '8009C55C'), ('CardWriteHandler', '8009C610')]:
                self.assertEqual(self.pointers[field].value, C.cast(getattr(self.library, NA(target)), C.c_void_p).value)
            self.assertEqual(bytes(self.buffer), before_buffer)
            self.assertEqual(self.calls.value, 0)

    def test_range_validation_failure_stops_before_state_changes(self):
        for mode, function in enumerate(self.range_starts):
            for results, expected_calls in [((0, 1), 1), ((1, 0), 2), ((0, 0), 1)]:
                self.initialize(1)
                self.validation_results[:] = results
                before = self.snapshot()
                self.assertEqual(function(77, C.addressof(self.buffer), 123, 9), 0)
                self.assertEqual(self.snapshot(), before)
                self.assertEqual(self.validation_calls.value, expected_calls)
                self.assertEqual(list(self.validation_modes)[:expected_calls], [mode] * expected_calls)
                self.assertEqual(list(self.validation_values)[:expected_calls], [123, 131][:expected_calls])
                self.assertEqual(self.calls.value, 0)

    def test_range_validation_precedes_readiness_gate(self):
        for mode, function in enumerate(self.range_starts):
            self.initialize(2)
            before = self.snapshot()
            self.assertEqual(function(77, C.addressof(self.buffer), 123, 9), 0)
            self.assertEqual(self.snapshot(), before)
            self.assertEqual(self.validation_calls.value, 2)
            self.assertEqual(list(self.validation_modes), [mode, mode])
            self.assertEqual(list(self.validation_values), [123, 131])

    def test_range_end_wrap_and_success_configuration(self):
        values = [(-2147483648, -1), (2147483647, 2), (123, 0), (123, 1),
                  (123, 9), (-1, 2147483647), (0, -2147483648)]
        for mode, function in enumerate(self.range_starts):
            for sector, count in values:
                self.initialize(1)
                ram, scalars, pointers = self.snapshot()
                expected_ram = bytearray(ram)
                expected_ram[0xBDE4:0xBDE8] = (2).to_bytes(4, 'little')
                for name in ('CardTxCount', 'CardReceiveIndex', 'CardRxCount', 'CardBytesRemaining'):
                    scalars[name] = 0
                scalars.update(CardCommandRequest=-77, CardCmdSector=sector, CardVarRxLength=count,
                               CardCurrentCommand=4 + mode)
                pointers['CardCmdBuffer'] = C.addressof(self.buffer)
                read, write = ('8009C87C', '8009C91C') if mode == 0 else ('8009CA8C', '8009CB50')
                for field, target in [('CardCommandReader', read), ('CardWriteHandler', write)]:
                    pointers[field] = C.cast(getattr(self.library, NA(target)), C.c_void_p).value
                before_buffer = bytes(self.buffer)
                self.assertEqual(function(-77, C.addressof(self.buffer), sector, count), 1)
                self.assertEqual(self.snapshot(), (bytes(expected_ram), scalars, pointers))
                self.assertEqual(self.validation_calls.value, 2)
                self.assertEqual(list(self.validation_modes), [mode, mode])
                self.assertEqual(list(self.validation_values), [sector, C.c_int32(sector + count - 1).value])
                self.assertEqual(bytes(self.buffer), before_buffer)
                self.assertEqual(self.calls.value, 0)

    def test_second_start_is_rejected_after_state_becomes_active(self):
        for n, code, read, write, fields in CASES:
            self.initialize(1)
            self.assertEqual(self.functions[n](7, *self.arguments(fields, 8)), 1)
            before = self.snapshot()
            self.assertEqual(self.functions[n](9, *self.arguments(fields, 10)), 0)
            self.assertEqual(self.snapshot(), before)


if __name__ == '__main__':
    unittest.main()
