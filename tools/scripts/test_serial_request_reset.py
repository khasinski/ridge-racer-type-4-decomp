"""Actual serial reset: ordered hardware writes, IRQ call, and request reloads."""
import ctypes as C
from pathlib import Path
import random
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest


class SerialRequestResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        directory = Path(cls.temp.name)
        header = ''
        for name in ('interrupts.h', 'serial.h'):
            header += re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '',
                             (root / 'include/rr' / name).read_text(), flags=re.M)
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef int32_t s32;
''' + header + '''
RrInterruptRegisters test_irq;
RrInterruptRegisters *I_STAT_Sio=&test_irq;
RrSerialPortRegisters test_ports[2];
RrSerialPortRegisters *SIO1_REGS=&test_ports[0];
volatile RrSerialRequest SioRequest;
RrSerialReceiveRequest SioReceiveRequest;
u16 SioShadowControl;
u8 SioIrqQueueEntry[32];
u32 test_count,test_log[32][3],test_mutate,test_override;
void record(u32 op,u32 value,uintptr_t pointer) {
 u32 i=test_count++;if(i<32){test_log[i][0]=op;test_log[i][1]=value;test_log[i][2]=0;}
}
uintptr_t test_irq_pointer;
s32 SysDeqIntRP(s32 channel,void *pointer) {
 test_irq_pointer=(uintptr_t)pointer;record(6,channel,0);
 if(test_mutate) {
  SioRequest.first=123;SioReceiveRequest.request.first=-77;
  SioReceiveRequest.receive_status=0x12345678U;
  SIO1_REGS=&test_ports[1];SioShadowControl=0x5555;
 }
 return -99;
}
u32 read_mask(void) {u32 v=test_irq.mask;record(1,v,0);return v;}
void write_mask(u32 v) {record(2,v,0);test_irq.mask=v;}
u32 read_control(RrSerialPortRegisters *port) {u32 v=port->control;record(3,v,0);return v;}
void write_control(RrSerialPortRegisters *port,u32 value) {record(4,value,0);port->control=value;}
u32 read_configuration(void) {u32 v=SioShadowControl;record(5,v,0);return v;}
void write_send_active(s32 value) {record(7,(u32)value,0);SioRequest.active=value;}
s32 read_send_active(void) {
 s32 v;if(test_override)SioRequest.active=-1234567;
 v=SioRequest.active;record(8,(u32)v,0);return v;
}
void write_receive_active(s32 value) {record(9,(u32)value,0);SioReceiveRequest.request.active=value;}
void write_send_second(s32 value) {record(10,(u32)value,0);SioRequest.second=value;}
s32 read_send_second(void) {
 s32 v;if(test_override)SioRequest.second=0x12345678;
 v=SioRequest.second;record(11,(u32)v,0);return v;
}
void write_receive_second(s32 value) {record(12,(u32)value,0);SioReceiveRequest.request.second=value;}
void initialize(u32 mask,u32 control,u32 configuration) {
 memset(&test_irq,0xA5,sizeof(test_irq));test_irq.mask=mask;
 memset(test_ports,0xA5,sizeof(test_ports));test_ports[0].control=control;
 memset((void *)&SioRequest,0x69,sizeof(SioRequest));
 memset(&SioReceiveRequest,0x5A,sizeof(SioReceiveRequest));
 SIO1_REGS=&test_ports[0];SioShadowControl=configuration;
 test_count=test_mutate=test_override=0;test_irq_pointer=0;
 memset(test_log,0xA5,sizeof(test_log));
}
'''
        body = (root / 'tools/scripts/fixtures/legacy/func_8009B85C.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?:,\s*"memory")?', '', body)
        source += body.replace('_sio_deinit', 'test_native_reset')
        replacements = {
            'interrupts->mask &= ~0x100U;': 'write_mask(read_mask() & ~0x100U);',
            'port->control |= 0x50;': 'write_control(port,read_control(port) | 0x50);',
            '*(u16 *)&port->control=*control;': 'write_control(port,read_configuration());',
            'send->active=0;': 'write_send_active(0);',
            'active=send->active;': 'active=read_send_active();',
            'receive->request.active=active;': 'write_receive_active(active);',
            'send->second=0;': 'write_send_second(0);',
            'active=send->second;': 'active=read_send_second();',
            'receive->request.second=active;': 'write_receive_second(active);',
        }
        for old, new in replacements.items():
            if body.count(old) != 1:
                raise AssertionError('Unexpected adapter site count: ' + old)
            body = body.replace(old, new)
        source += body
        (directory / 'reset.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'reset.c'),
                                 '-o', str(directory / 'reset.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory / 'reset.so'))
        cls.irq = (C.c_uint32 * 2).in_dll(cls.lib, 'test_irq')
        cls.ports = (C.c_uint8 * 32).in_dll(cls.lib, 'test_ports')
        cls.send = (C.c_int32 * 3).in_dll(cls.lib, 'SioRequest')
        cls.receive = (C.c_int32 * 4).in_dll(cls.lib, 'SioReceiveRequest')
        cls.count = C.c_uint32.in_dll(cls.lib, 'test_count')
        cls.log = (C.c_uint32 * 96).in_dll(cls.lib, 'test_log')
        cls.config = C.c_uint16.in_dll(cls.lib, 'SioShadowControl')
        cls.mutate = C.c_uint32.in_dll(cls.lib, 'test_mutate')
        cls.override = C.c_uint32.in_dll(cls.lib, 'test_override')
        cls.irq_pointer = C.c_size_t.in_dll(cls.lib, 'test_irq_pointer')
        cls.irq_handler = (C.c_ubyte * 32).in_dll(cls.lib, 'SioIrqQueueEntry')
        cls.initialize = cls.lib.initialize
        cls.initialize.argtypes = [C.c_uint32]*3
        cls.reset, cls.native = cls.lib._sio_deinit, cls.lib.test_native_reset
        for function in (cls.reset, cls.native):
            function.argtypes = []
            function.restype = C.c_int32

    def check(self, mask, control, configuration, mutate=False, override=False, native=False):
        self.initialize(mask, control, configuration)
        self.mutate.value, self.override.value = mutate, override
        ports = bytearray(self.ports)
        struct.pack_into('=H', ports, 10, configuration)
        active = -1234567 if override else 0
        second = 0x12345678 if override else 0
        expected_send = [active, 123 if mutate else 0x69696969, second]
        expected_receive = [active, -77 if mutate else 0x5A5A5A5A, second,
                            0x12345678 if mutate else 0x5A5A5A5A]
        self.assertEqual((self.native if native else self.reset)(), 0)
        if native:
            expected_trace = [(6, 3)]
        else:
            expected_trace = [(1, mask), (2, mask & ~0x100), (3, control),
                              (4, control | 0x50), (5, configuration), (4, configuration),
                              (6, 3), (7, 0), (8, active & 0xFFFFFFFF),
                              (9, active & 0xFFFFFFFF), (10, 0), (11, second), (12, second)]
        actual = [(self.log[3*i], self.log[3*i+1]) for i in range(self.count.value)]
        self.assertEqual(actual, expected_trace)
        self.assertEqual(list(self.irq), [0xA5A5A5A5, mask & ~0x100])
        self.assertEqual(bytes(self.ports), bytes(ports))
        self.assertEqual(list(self.send), expected_send)
        self.assertEqual(list(self.receive), expected_receive)
        self.assertEqual(self.config.value, 0x5555 if mutate else configuration)
        self.assertEqual(self.irq_pointer.value, C.addressof(self.irq_handler))

    def test_ordered_mask_control_configuration_and_request_resets(self):
        cases = [(0, 0, 0), (0x100, 0x50, 0), (0xFFFFFFFF, 0xFFFF, 0xFFFF),
                 (0xFFFFFEFF, 0xA5A5, 0x5A5A)]
        rng = random.Random(0x9B85C)
        cases += [(rng.getrandbits(32), rng.getrandbits(16), rng.getrandbits(16)) for _ in range(500)]
        for mask, control, configuration in cases:
            self.check(mask, control, configuration)

    def test_irq_callback_changes_to_unreset_fields_are_preserved(self):
        for native in (False, True):
            self.check(0xFFFFFFFF, 0xA5A5, 0x1234, mutate=True, native=native)

    def test_volatile_request_words_are_reloaded_after_each_clear(self):
        self.check(0xFFFFFFFF, 0xA5A5, 0x1234, mutate=True, override=True)

    def test_unchanged_native_body_agrees_for_ram_requests(self):
        for mask in (0, 0x100, 0xFFFFFFFF):
            for control, configuration in ((0, 0), (0xFFFF, 0x1357), (0x1234, 0xFFFF)):
                self.check(mask, control, configuration, native=True)


if __name__ == '__main__':
    unittest.main()
