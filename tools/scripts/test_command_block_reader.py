"""Real command-11 reader and constructor, including returned checksum operands."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Poll = C.CFUNCTYPE(C.c_int32)


class CommandBlockReaderTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        header = (root/'include/rr/commands.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8;typedef uint32_t u32;typedef int32_t s32;
''' + header + '''
s32 CardCommandStatus,CardTxCount,CardCommandRequest,CardRetriesLeft,CardReceiveIndex,CardRxCount;
s32 CardChecksum,CardCmdSector,CardVarRxLength,CardBytesRemaining,CardCurrentCommand;
RrCommandParameter D_8011472C;
RrCommandReadCallback CardCommandReader;
RrCommandWriteCallback CardWriteHandler;
u8 CardRxTrace[160];
s32 (*test_tick)(void),(*test_phase)(void);
u8 D_80114714;
void test_reset(void) {
 memset(CardRxTrace,0xA5,sizeof(CardRxTrace));
 CardCommandStatus=3;CardTxCount=2;CardCommandRequest=0x123;CardRetriesLeft=0;
 CardReceiveIndex=4;CardRxCount=6;CardBytesRemaining=9;
 D_80114714=0xB7;
 CardChecksum=0x13579BDF;CardCmdSector=0x12345678;
 D_8011472C.value=-123;CardCurrentCommand=99;
 CardCommandReader=0;CardWriteHandler=0;
}
'''
        for name in ('8009D978', '8009DB18', '8009D8B0'):
            body = (root/('tools/scripts/fixtures/views/func_'+name+'.c')).read_text()
            body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+",\s*', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
            if name == '8009D978':
                assert body.count('CardTxCount') == 5
                body = body.replace('CardTxCount', 'test_tick()')
            if name == '8009DB18':
                assert body.count('CardRxCount') == 3
                body = body.replace('CardRxCount', 'test_phase()')
            source += body
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.trace = (C.c_uint8 * 160).in_dll(cls.lib, 'CardRxTrace')
        cls.reader = cls.lib._cardSectorTx
        cls.reader.argtypes = []
        cls.reader.restype = C.c_int32
        cls.begin = cls.lib._cardSectorBegin
        cls.begin.argtypes = [C.c_int32]*3
        cls.begin.restype = C.c_int32
        cls.writer = cls.lib._cardSectorRx
        cls.writer.argtypes = [C.c_int32]
        cls.writer.restype = C.c_int32
        cls.checksum = C.c_uint8.in_dll(cls.lib, 'D_80114714')
        cls.parameter = C.c_int32.in_dll(cls.lib, 'D_8011472C')
        cls.read_pointer = C.c_void_p.in_dll(cls.lib, 'CardCommandReader')
        cls.write_pointer = C.c_void_p.in_dll(cls.lib, 'CardWriteHandler')

    def setUp(self):
        self.lib.test_reset()
        self.errors = []
        self.observe_tick(lambda: self.get('CardTxCount'))
        self.observe_phase(lambda: self.get('CardRxCount'))

    def observe_tick(self, action):
        def checked():
            try:
                return action()
            except BaseException as error:
                self.errors.append(error)
                return 0
        self.tick_callback = Poll(checked)
        C.c_void_p.in_dll(self.lib, 'test_tick').value = C.cast(self.tick_callback, C.c_void_p).value

    def observe_phase(self, action):
        def checked():
            try:
                return action()
            except BaseException as error:
                self.errors.append(error)
                return 0
        self.phase_callback = Poll(checked)
        C.c_void_p.in_dll(self.lib, 'test_phase').value = C.cast(self.phase_callback, C.c_void_p).value

    def run_writer(self, value):
        result = self.writer(value)
        if self.errors:
            raise self.errors[0]
        return result

    def get(self, name):
        return C.c_int32.in_dll(self.lib, name).value

    def put(self, name, value):
        C.c_int32.in_dll(self.lib, name).value = value

    def run_reader(self):
        result = self.reader()
        if self.errors:
            raise self.errors[0]
        return result

    def test_phase_dispatch_signed_extrema_and_both_modes(self):
        phases = list(range(-8, 145))+[-0x80000000, 0x7FFFFFFF]
        for retry in (0, 1, -1, -0x80000000):
            for phase in phases:
                self.put('CardRetriesLeft', retry)
                self.put('CardTxCount', phase)
                self.put('CardChecksum', 0x13579BDF)
                self.put('CardCmdSector', -0x1234568)
                self.parameter.value = -0x7654321
                old = 0x13579BDF
                expected_checksum = old
                if retry:
                    expected = {2:0x52, 5:0xBA, 6:0x98}.get(phase, 0)
                elif phase == 2:
                    expected, expected_checksum = 0x57, 0
                elif phase == 5:
                    expected = 0xBA
                elif phase == 6:
                    expected = 0x98
                elif phase == 133:
                    expected = self.parameter.value
                    expected_checksum = C.c_int32(old ^ expected).value
                elif phase == 134:
                    expected = old
                elif phase == 135:
                    expected = 0xBA ^ 0x98
                elif 7 <= phase < 133:
                    expected = self.trace[phase+3]
                    expected_checksum = old ^ expected
                else:
                    expected = 0
                self.assertEqual(self.run_reader(), expected, (retry, phase))
                self.assertEqual(self.get('CardChecksum'), expected_checksum)
                self.assertEqual(bytes(self.trace), b'\xA5'*160)

    def test_every_payload_position_byte_and_checksum_high_bits(self):
        for phase in range(7, 133):
            self.put('CardTxCount', phase)
            for byte in range(256):
                checksum = C.c_int32(0x8000FF00 | (255-byte)).value
                self.put('CardChecksum', checksum)
                self.trace[phase+3] = byte
                self.assertEqual(self.run_reader(), byte)
                self.assertEqual(self.get('CardChecksum'), C.c_int32(checksum ^ byte).value)
            self.trace[phase+3] = 0xA5
        self.assertEqual(bytes(self.trace), b'\xA5'*160)
        for phase in (133, 134):
            self.put('CardTxCount', phase)
            for checksum in (-0x80000000, -1, 0, 0x12345678, 0x7FFFFFFF):
                for parameter in (-0x80000000, -1, 0, 1, 0x7FFFFFFF):
                    self.put('CardChecksum', checksum)
                    self.parameter.value = parameter
                    self.assertEqual(self.run_reader(), parameter if phase == 133 else checksum)
                    expected = C.c_int32(checksum ^ parameter).value if phase == 133 else checksum
                    self.assertEqual(self.get('CardChecksum'), expected)

    def test_all_parameter_byte_pairs_and_full_transfer_checksum(self):
        self.put('CardTxCount', 135)
        for word in range(65536):
            self.put('CardCmdSector', C.c_int32(0xFEDC0000 | word).value)
            self.assertEqual(self.run_reader(), ((word >> 8) ^ word) & 255)
        for i in range(160):
            self.trace[i] = (i*43+17)&255
        self.parameter.value = 1
        self.put('CardTxCount', 2)
        self.assertEqual(self.run_reader(), 0x57)
        checksum = 0
        for phase in range(7, 134):
            self.put('CardTxCount', phase)
            byte = self.run_reader()
            checksum ^= byte
            self.assertEqual(self.get('CardChecksum'), checksum)
        self.put('CardTxCount', 134)
        self.assertEqual(self.run_reader(), checksum)

    def test_fresh_tick_reads_and_independent_signed_bounds(self):
        for ticks, expected_reads, expected_result in (([7, 6], 2, 0),
                ([7, 7, 133], 3, 0), ([7, 7, 132, 10], 4, 0xA5)):
            seen = []
            def tick():
                seen.append(1)
                if len(seen) == 4:
                    self.put('CardChecksum', -1)
                return ticks[len(seen)-1]
            self.observe_tick(tick)
            self.put('CardChecksum', 123)
            self.assertEqual(self.run_reader(), expected_result)
            self.assertEqual(len(seen), expected_reads)
            self.assertEqual(self.get('CardChecksum'), C.c_int32(-1 ^ 0xA5).value if expected_reads == 4 else 123)

    def test_constructor_validates_block_before_begin_and_installs_real_reader(self):
        for block in (-0x80000000, -1, 0, *range(1, 17), 0x7FFFFFFF):
            for state in (0, 1, 2, 3, -1):
                for mode in (0, 1, -0x80000000):
                    self.lib.test_reset()
                    self.put('CardCommandStatus', state)
                    result = self.begin(-0x12345678, block, mode)
                    success = 1 <= block <= 15 and state & 1
                    self.assertEqual(result, int(bool(success)))
                    self.assertEqual(self.get('CardCmdSector'), block)
                    if success:
                        self.assertEqual(self.get('CardCommandStatus'), 2)
                        self.assertEqual(self.get('CardCommandRequest'), -0x12345678)
                        self.assertEqual([self.get(n) for n in ('CardTxCount','CardReceiveIndex','CardRxCount','CardBytesRemaining')], [0]*4)
                        self.assertEqual(self.parameter.value, int(mode != 0))
                        self.assertEqual(self.get('CardCurrentCommand'), 11)
                        self.assertEqual(self.get('CardRetriesLeft'), 1)
                        self.assertEqual(self.read_pointer.value, C.cast(self.reader, C.c_void_p).value)
                        self.assertEqual(self.write_pointer.value, C.cast(self.lib._cardSectorRx, C.c_void_p).value)
                        self.put('CardTxCount', 2)
                        self.assertEqual(Poll(self.read_pointer.value)(), 0x52)
                    else:
                        self.assertEqual(self.get('CardCommandStatus'), state)
                        self.assertEqual(self.get('CardCommandRequest'), 0x123)
                        self.assertEqual(self.parameter.value, -123)
                        self.assertIsNone(self.read_pointer.value)
                        self.assertIsNone(self.write_pointer.value)

    def test_writer_all_phases_bytes_high_words_and_signed_bounds(self):
        phases = list(range(-2, 145))+[-0x80000000, 0x7FFFFFFF]
        for retry in (0, 1, -1):
            self.put('CardRetriesLeft', retry)
            for phase in phases:
                self.put('CardRxCount', phase)
                for byte in range(256):
                    value = C.c_int32(0xFEDCBA00 | byte).value
                    self.checksum.value = 0xB7
                    self.put('CardBytesRemaining', -123)
                    self.put('CardRetriesLeft', retry)
                    expected = 0
                    checksum = 0xB7
                    remaining = -123
                    next_retry = retry
                    if retry:
                        if phase == 2:
                            remaining = 1 if byte == 8 else 138
                            if byte == 8:
                                next_retry = 0
                        elif phase == 3:
                            expected = 0 if byte == 0x5A else -3
                        elif phase == 8:
                            expected = 0 if byte == 0x5D else -8
                            if not expected:
                                checksum = 0
                        elif phase in (9, 10):
                            expected = 0 if byte == (0x56 if phase == 9 else 0x78) else -phase
                            if not expected:
                                checksum ^= byte
                        elif phase == 139:
                            expected = 0 if byte == checksum else -139
                        elif phase == 140:
                            expected = 0 if byte == 0x47 else -140
                        elif 11 <= phase < 139:
                            checksum ^= byte
                    else:
                        if phase == 2:
                            remaining = 1 if byte == 8 else 136
                        elif phase in (3, 137, 138):
                            expected = 0 if byte == {3:0x5A,137:0x5D,138:0x47}[phase] else -phase
                    self.assertEqual(self.run_writer(value), expected, (retry, phase, byte))
                    self.assertEqual(self.checksum.value, checksum)
                    self.assertEqual(self.get('CardBytesRemaining'), remaining)
                    self.assertEqual(self.get('CardRetriesLeft'), next_retry)
        self.assertEqual(bytes(self.trace), b'\xA5'*160)

    def test_writer_every_checksum_value_and_fresh_default_phase(self):
        self.put('CardRetriesLeft', 1)
        self.put('CardRxCount', 139)
        for checksum in range(256):
            for byte in range(256):
                self.checksum.value = checksum
                self.assertEqual(self.run_writer(byte), 0 if byte == checksum else -139)
                self.assertEqual(self.checksum.value, checksum)
        for phases, byte, expected in (([50,139], 0xFF, 0xB7),
                ([140,11], 0x47, 0xB7 ^ 0x47), ([50,138], 0xFF, 0xB7 ^ 0xFF)):
            seen = []
            def phase():
                seen.append(1)
                return phases[len(seen)-1]
            self.observe_phase(phase)
            self.checksum.value = 0xB7
            self.assertEqual(self.run_writer(byte), 0)
            self.assertEqual(self.checksum.value, expected)
            self.assertEqual(len(seen), 2)

    def test_actual_constructor_installed_writer_complete_retry_and_normal_stream(self):
        self.assertEqual(self.begin(0x12B, 5, -1), 1)
        writer = C.CFUNCTYPE(C.c_int32, C.c_int32)(self.write_pointer.value)
        self.put('CardRxCount', 2)
        self.assertEqual(writer(0), 0)
        self.assertEqual(self.get('CardBytesRemaining'), 138)
        self.assertEqual(self.get('CardRetriesLeft'), 1)
        for phase, byte in ((3,0x5A),(8,0x5D),(9,0),(10,5)):
            self.put('CardRxCount', phase)
            self.assertEqual(writer(byte), 0)
        checksum = 5
        for phase in range(11,139):
            byte = (phase*43+17)&255
            self.put('CardRxCount', phase)
            self.assertEqual(writer(0x12340000 | byte), 0)
            checksum ^= byte
            self.assertEqual(self.checksum.value, checksum)
        self.put('CardRxCount', 139)
        self.assertEqual(writer(checksum), 0)
        self.put('CardRxCount', 140)
        self.assertEqual(writer(0x47), 0)
        self.put('CardRxCount', 2)
        self.assertEqual(writer(8), 0)
        self.assertEqual(self.get('CardRetriesLeft'), 0)
        self.assertEqual(self.get('CardBytesRemaining'), 1)
        for phase, byte in ((3,0x5A),(137,0x5D),(138,0x47)):
            self.put('CardRxCount', phase)
            self.assertEqual(writer(byte), 0)
        self.assertEqual(self.checksum.value, checksum)
