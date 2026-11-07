"""Real command completion dispatch, including the real restart helper."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

Poll = C.CFUNCTYPE(C.c_int32)
Hook = C.CFUNCTYPE(None)


class CommandCompletionTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        headers = ''
        for name in ('commands', 'interrupts', 'serial'):
            text = (root / ('include/rr/' + name + '.h')).read_text()
            text = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', text, flags=re.M)
            text = re.sub(r'^extern s32 CardCommandStatus,[^\n]*\n', '', text, flags=re.M)
            headers += text
        source = '''#include <stdint.h>
#include <string.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef int32_t s32;
s32 test_ram[0x4000];
#define CardCommandStatus test_ram[0xBDE4/4]
#define CardTxCount test_ram[0xBDE8/4]
#define CardCommandRequest test_ram[0xBDEC/4]
#define CardReceiveIndex test_ram[0xBDF4/4]
#define CardRxCount test_ram[0xBDF8/4]
''' + headers + '''
s32 CardRetriesLeft,CardBytesRemaining;
u8 D_800ABDC0[16];
RrInterruptRegisters test_irq[2],*I_STAT_Card;
RrSerialPortRegisters test_serial[2],*CardSio0Regs;
s32 (*test_poll)(void);void (*test_restart_hook)(void);
s32 test_restart_calls,test_request,test_event_calls,test_event_flag;
void *test_event_pointer;
s32 test_event_state,test_event_counter,test_restart_state;
s32 _cardCmdStep(void) { return test_poll(); }
s32 test_restart(s32 request) {
 s32 result;
 test_restart_calls++;test_request=request;test_restart_state=CardCommandStatus;
 result=_cardCmdReset(request);
 if(test_restart_hook)test_restart_hook();
 return result;
}
s32 SysDeqIntRP(s32 flag,void *event) {
 test_event_calls++;test_event_flag=flag;test_event_pointer=event;
 test_event_state=CardCommandStatus;test_event_counter=CardTxCount;
 return -123;
}
void test_reset(void) {
 memset(test_ram,0xA5,sizeof(test_ram));memset(test_irq,0,sizeof(test_irq));
 memset(test_serial,0x5A,sizeof(test_serial));
 I_STAT_Card=&test_irq[0];CardSio0Regs=&test_serial[0];
 CardCommandStatus=3;CardTxCount=10;CardCommandRequest=123;
 CardRetriesLeft=0;CardReceiveIndex=9;CardRxCount=11;CardBytesRemaining=17;
 test_restart_calls=test_event_calls=0;test_restart_hook=0;
}
'''
        for name in ('8009C0A4', '8009BC3C'):
            body = (root / ('tools/scripts/fixtures/views/func_' + name + '.c')).read_text()
            body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
            body = re.sub(r'^extern s32 D_[^\n]*\n', '', body, flags=re.M)
            body = body.replace('(s32 *)0x800B0000U', '(test_ram+0x4000)')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+",\s*', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
            if name == '8009BC3C':
                assert body.count('_cardCmdReset(') == 1
                body = body.replace('_cardCmdReset(', 'test_restart(')
            source += body
        directory = Path(cls.temp.name)
        (directory / 'function.c').write_text(source)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.ram = (C.c_int32 * 0x4000).in_dll(cls.lib, 'test_ram')
        cls.irq = (C.c_uint32 * 4).in_dll(cls.lib, 'test_irq')
        cls.serial = (C.c_uint16 * 16).in_dll(cls.lib, 'test_serial')
        cls.run_completion = cls.lib._cardSioHandler
        cls.run_completion.argtypes = []
        cls.run_completion.restype = C.c_int32

    def scalar(self, name):
        return C.c_int32.in_dll(self.lib, name)

    def pointer(self, name):
        return C.c_void_p.in_dll(self.lib, name)

    def run_with(self, poll, hook=None):
        errors = []
        def observed_poll():
            try:
                return poll()
            except BaseException as error:
                errors.append(error)
                return 0
        def observed_hook():
            try:
                hook()
            except BaseException as error:
                errors.append(error)
        callback = Poll(observed_poll)
        restart = Hook(observed_hook) if hook else None
        self.pointer('test_poll').value = C.cast(callback, C.c_void_p).value
        self.pointer('test_restart_hook').value = C.cast(restart, C.c_void_p).value if restart else None
        self.assertEqual(self.run_completion(), 0)
        if errors:
            raise errors[0]

    def test_result_branches_retry_extrema_and_wrapped_counter(self):
        for result in (0, 1, 2, -1, -0x80000000, 0x7FFFFFFF):
            for retries in (0, 1, -1, -0x80000000, 0x7FFFFFFF):
                self.lib.test_reset()
                self.scalar('CardRetriesLeft').value = retries
                self.ram[0xBDE8//4] = 0x7FFFFFFF
                self.irq[1] = 0x12340001
                serial_before = bytes(self.serial)
                def poll():
                    self.assertEqual(self.ram[0xBDE8//4], -0x80000000)
                    return result
                self.run_with(poll)
                self.assertEqual(list(self.irq), [0xFFFFFF7F, 0x12340081, 0, 0])
                retried = result == 1 and retries != 0
                self.assertEqual(self.scalar('test_restart_calls').value, int(retried))
                self.assertEqual(self.scalar('test_event_calls').value, int(result != 0))
                self.assertEqual(self.ram[0xBDE8//4], 0 if result else -0x80000000)
                state = (2 if retried else 0x41) if result == 1 else (0x21 if result else 3)
                self.assertEqual(self.ram[0xBDE4//4], state)
                expected_retries = C.c_int32(retries - int(retried)).value
                self.assertEqual(self.scalar('CardRetriesLeft').value, expected_retries)
                expected = bytearray(serial_before)
                if result:
                    expected[10:12] = b'\0\0'
                    self.assertEqual(self.scalar('test_event_flag').value, 1)
                    self.assertEqual(self.scalar('test_event_counter').value, 0)
                    self.assertEqual(self.scalar('test_event_state').value, state)
                    event = (C.c_uint8 * 16).in_dll(self.lib, 'D_800ABDC0')
                    self.assertEqual(self.pointer('test_event_pointer').value, C.addressof(event))
                self.assertEqual(bytes(self.serial), bytes(expected))

    def test_fresh_pointers_and_retry_count_after_helper_mutations(self):
        self.lib.test_reset()
        self.scalar('CardRetriesLeft').value = 4
        def poll():
            self.pointer('I_STAT_Card').value = C.addressof(self.irq) + 8
            self.ram[0xBDEC//4] = -123
            return 1
        def restarted():
            self.assertEqual(self.scalar('test_request').value, -123)
            self.assertEqual(self.scalar('test_restart_state').value, 0x41)
            self.assertEqual(self.ram[0xBDF4//4], 0)
            self.assertEqual(self.ram[0xBDF8//4], 0)
            self.assertEqual(self.scalar('CardBytesRemaining').value, 0)
            self.scalar('CardRetriesLeft').value = -0x80000000
            self.ram[0xBDE4//4] = 99
            self.ram[0xBDE8//4] = 999
            self.pointer('CardSio0Regs').value = C.addressof(self.serial) + 16
        self.run_with(poll, restarted)
        self.assertEqual(list(self.irq), [0, 0, 0xFFFFFF7F, 0x80])
        self.assertEqual(self.scalar('CardRetriesLeft').value, 0x7FFFFFFF)
        self.assertEqual((self.serial[5], self.serial[13]), (0x5A5A, 0))
        self.assertEqual(self.ram[0xBDE4//4], 2)
        self.assertEqual(self.ram[0xBDE8//4], 0)

    def test_zero_result_does_not_access_serial_port_or_signal_event(self):
        self.lib.test_reset()
        self.pointer('CardSio0Regs').value = None
        self.run_with(lambda: 0)
        self.assertEqual(self.scalar('test_event_calls').value, 0)
        self.assertEqual(self.ram[0xBDE8//4], 11)
