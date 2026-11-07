"""Recovered byte exchange with deterministic hardware/timer observations."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Port(C.Structure):
    _fields_ = [('data', C.c_uint8), ('pad', C.c_uint8 * 3),
                ('status', C.c_uint16), ('pad6', C.c_uint16),
                ('mode', C.c_uint16), ('control', C.c_uint16),
                ('padC', C.c_uint16), ('baud', C.c_uint16)]


Poll = C.CFUNCTYPE(C.c_int32)
Start = C.CFUNCTYPE(None, C.c_int32)
Status = C.CFUNCTYPE(C.c_uint16, C.c_void_p)


class SerialByteExchangeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        header = (root/'include/rr/serial.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        header = header.replace('extern s32 CardSioRxByte;', '')
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef int32_t s32;
s32 test_page[0x4000];
#define CardSioRxByte test_page[0x4700/4]
#define RR_NOP_DEP_ON(value) ((void)(value))
''' + header + '''
RrSerialPortRegisters test_ports[2],*CardSio0Base;
volatile u32 test_irq[2],*I_STAT_CardSel;
s32 (*test_poll)(void);void (*test_start)(s32);
u16 (*test_status)(void *);
s32 chkRC2wait(void) { return test_poll(); }
void setRC2wait(s32 limit) { test_start(limit); }
u16 test_initial_status(RrSerialPortRegisters *port) { return test_status(port); }
void test_reset(void) {
 memset(test_ports,0x5A,sizeof(test_ports));memset(test_page,0xA5,sizeof(test_page));
 test_ports[0].data=0xAB;test_ports[0].status=1;
 test_ports[1].data=0xCD;test_ports[1].status=0;
 test_irq[0]=test_irq[1]=0;
 CardSio0Base=&test_ports[0];I_STAT_CardSel=&test_irq[0];
}
'''
        body = (root/'tools/scripts/fixtures/views/func_8009BE18.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r'"\$[0-9]+",\s*', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
        assert body.count('(s32 *)0x80110000U') == 1
        body = body.replace('(s32 *)0x80110000U', 'test_page')
        assert body.count('initial->status') == 1
        body = body.replace('initial->status', 'test_initial_status(initial)')
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(source + body)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.ports = (Port * 2).in_dll(cls.lib, 'test_ports')
        cls.irq = (C.c_uint32 * 2).in_dll(cls.lib, 'test_irq')
        cls.page = (C.c_int32 * 0x4000).in_dll(cls.lib, 'test_page')
        cls.exchange = cls.lib._cardXferByte
        cls.exchange.argtypes = [C.c_int32]
        cls.exchange.restype = C.c_int32

    def pointer(self, name):
        return C.c_void_p.in_dll(self.lib, name)

    def invoke(self, value, poll, start=lambda limit: None, status=lambda address: 1):
        errors = []
        def checked_poll():
            try:
                return poll()
            except BaseException as error:
                errors.append(error)
                return 1  # Allow each timer-controlled loop to terminate before rethrowing.
        def checked_start(limit):
            try:
                start(limit)
            except BaseException as error:
                errors.append(error)
        def checked_status(address):
            try:
                return status(address)
            except BaseException as error:
                errors.append(error)
                return 1
        callbacks = [Poll(checked_poll), Start(checked_start), Status(checked_status)]
        for name, callback in zip(('test_poll', 'test_start', 'test_status'), callbacks):
            self.pointer(name).value = C.cast(callback, C.c_void_p).value
        result = self.exchange(value)
        if errors:
            raise errors[0]
        return result

    def test_every_captured_byte_and_negative_input_low_byte(self):
        for byte in range(256):
            for value in (-1, -256, -0x80000000, C.c_int32(0xFFFFFF00 | byte).value):
                self.lib.test_reset()
                self.ports[0].data = byte
                before = bytes(self.ports)
                calls = []
                def poll():
                    calls.append('poll')
                    return 1
                self.assertEqual(self.invoke(value, poll), byte)
                expected = bytearray(before)
                expected[0] = (~value) & 255
                self.assertEqual(bytes(self.ports), bytes(expected))
                self.assertEqual(calls, ['poll'])
                self.assertEqual(list(self.irq), [0, 0])
                self.assertEqual(self.page[0x4700//4], byte)
                self.assertEqual(self.page[0x4700//4-1], C.c_int32(0xA5A5A5A5).value)
                self.assertEqual(self.page[0x4700//4+1], C.c_int32(0xA5A5A5A5).value)

    def test_initial_status_uses_captured_port_but_write_and_return_are_fresh(self):
        self.lib.test_reset()
        initial = C.addressof(self.ports)
        reads = []
        polls = []
        def status(address):
            reads.append(address)
            self.pointer('CardSio0Base').value = initial + C.sizeof(Port)
            return 1 if len(reads) == 3 else 0
        def poll():
            polls.append(1)
            self.page[0x4700//4] = -123
            return 1 if len(polls) == 3 else 0
        self.assertEqual(self.invoke(-2, poll, status=status), -123)
        self.assertEqual(reads, [initial] * 3)
        self.assertEqual((self.ports[0].data, self.ports[1].data), (0xAB, 1))

    def test_both_timeout_paths_for_every_captured_byte(self):
        for byte in range(256):
            for timeout in (0x200, 0x300):
                self.lib.test_reset()
                self.ports[0].data = byte
                self.ports[0].status = 0x80
                self.irq[0] = 0x80 if timeout == 0x300 else 0
                before = bytes(self.ports)
                limits = []
                self.assertEqual(self.invoke(0x12345678, lambda: 1, limits.append), -timeout-byte)
                expected = bytearray(before)
                if timeout == 0x300:
                    expected[0] = 0x78
                self.assertEqual(bytes(self.ports), bytes(expected))
                self.assertEqual(limits, [5, 60] if timeout == 0x300 else [])
                self.assertEqual(self.irq[0], 0xFFFFFF7F if timeout == 0x300 else 0)

    def test_handshake_timer_order_and_fresh_port_and_interrupt_pointers(self):
        self.lib.test_reset()
        self.ports[0].status = 0x80
        self.ports[1].control = 0x5A0A
        stage = [0, 0]
        trace = []
        second = C.addressof(self.ports) + C.sizeof(Port)
        def start(limit):
            trace.append(('start', limit))
            if limit == 60:
                self.assertEqual(self.ports[0].data, 0xFF)
                self.pointer('I_STAT_CardSel').value = C.addressof(self.irq) + 4
            stage[:] = [limit, 0]
        def poll():
            stage[1] += 1
            trace.append(('poll', stage[0]))
            if stage[0] == 0:
                self.irq[0] = 0x80
                return 0
            if stage[0] == 5:
                return int(stage[1] == 2)
            self.assertEqual(self.irq[1], 0xFFFFFF7F)
            self.pointer('CardSio0Base').value = second
            self.page[0x4700//4] = 0x12345678
            return 0
        self.assertEqual(self.invoke(0x7FFFFFFF, poll, start), 0x12345678)
        self.assertEqual(trace, [('poll', 0), ('start', 5), ('poll', 5), ('poll', 5),
                                 ('start', 60), ('poll', 60)])
        self.assertEqual((self.ports[0].control, self.ports[1].control), (0x5A5A, 0x5A1A))
        self.assertEqual(list(self.irq), [0x80, 0xFFFFFF7F])

    def test_every_output_byte_with_immediate_transmit_completion(self):
        for byte in range(256):
            self.lib.test_reset()
            self.irq[0] = 0x80
            self.ports[0].status = 0
            self.ports[0].control = 0xA50F
            before = bytes(self.ports)
            limits = []
            self.assertEqual(self.invoke(0x12340000 | byte, lambda: 1, limits.append), 0xAB)
            expected = bytearray(before)
            expected[0] = byte
            expected[10:12] = (0xA51F).to_bytes(2, 'little')
            self.assertEqual(bytes(self.ports), bytes(expected))
            self.assertEqual(limits, [5, 60])
