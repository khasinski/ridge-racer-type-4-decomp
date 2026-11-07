"""Actual GPU watchdog and mask exchange with observed hardware accesses."""
import ctypes as C
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class GpuTimeoutResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        headers = ''
        for name in ('gpu', 'interrupts', 'timing'):
            header = (root / ('include/rr/' + name + '.h')).read_text()
            headers += re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        code = '''#include <stdint.h>
#include <stdarg.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef int16_t s16;typedef uint32_t u32;typedef int32_t s32;
''' + headers + '''
u32 ports[8];u16 mask_storage,*I_MASK=&mask_storage;
volatile u32 *GPU_STATUS=&ports[0],*DMA2_MADR=&ports[1],*DMA2_CHCR=&ports[2],*DPCR=&ports[3];
s32 _qin,SavedIntrMask,GpuTimeoutDeadline,GpuAlarmCount;
volatile s32 _qout;
char GpuTimeoutFmt[]="GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\\n";
s32 test_now,test_diag_calls,test_read_count,test_time_calls;
s32 test_mask_calls,test_tail_reads,test_mutate_diag,test_mutate_mask,test_swap_write;
s32 test_event_count;u32 test_events[64][3];u32 test_diag_args[4];const char *test_format;
s32 test_counter_at_diag;
void event(u32 kind,u32 arg,u32 value) {
 s32 i=test_event_count++;if(i<64){test_events[i][0]=kind;test_events[i][1]=arg;test_events[i][2]=value;}
}
s32 VSync(s32 mode) {test_time_calls++;event(1,(u32)mode,0);return test_now;}
u32 test_read(volatile u32 *p) {
 u32 value=*p;test_read_count++;event(2,p==&ports[0]?0:4,value);
 if(test_read_count==1)*p=0x89ABCDEFU;
 return value;
}
s32 DebugPrintStub(const char *format,...) {
 va_list args;s32 i;va_start(args,format);
 for(i=0;i<4;i++)test_diag_args[i]=va_arg(args,u32);
 va_end(args);test_format=format;test_diag_calls++;test_counter_at_diag=GpuAlarmCount;
 event(3,0,0);
 if(test_mutate_diag){GPU_STATUS=&ports[4];DMA2_CHCR=&ports[5];DPCR=&ports[6];}
 return 1;
}
s32 test_exchange(s32 value) {
 s32 previous=SetIntrMask(value);
 test_mask_calls++;event(4,(u32)value,(u32)previous);
 if(test_mutate_mask && test_mask_calls==1){GPU_STATUS=&ports[4];DMA2_CHCR=&ports[5];DPCR=&ports[6];}
 return previous;
}
s32 test_tail(void) {test_tail_reads++;event(5,0,(u32)_qout);return _qout;}
void test_dma(volatile u32 *p,u32 value) {*p=value;event(6,p==&ports[2]?2:5,value);}
void test_config(volatile u32 *p) {*p|=0x800;event(7,p==&ports[3]?3:6,*p);}
void test_gpu(volatile u32 *p,u32 value) {
 *p=value;event(8,p==&ports[0]?0:(p==&ports[4]?4:7),value);
 if(test_swap_write && value==0x02000000){GPU_STATUS=&ports[7];SavedIntrMask=0x1234;}
}
void test_reset(void) {
 u32 i;for(i=0;i<8;i++)ports[i]=0xA5000000+i;
 GPU_STATUS=&ports[0];DMA2_MADR=&ports[1];DMA2_CHCR=&ports[2];DPCR=&ports[3];
 _qin=70;_qout=5;SavedIntrMask=-1;GpuTimeoutDeadline=100;GpuAlarmCount=0;
 mask_storage=0xF123;test_now=101;
 test_diag_calls=test_read_count=test_time_calls=test_mask_calls=test_tail_reads=test_event_count=0;
 test_mutate_diag=test_mutate_mask=test_swap_write=0;
 memset(test_events,0,sizeof(test_events));memset(test_diag_args,0,sizeof(test_diag_args));test_format=0;
}
'''
        def native(body):
            body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+",\s*', '', body)
            return re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
        for name in ('8008B4E0', '80095360', '80095394'):
            body = native((root/('tools/scripts/fixtures/views/func_' + name + '.c')).read_text())
            if name == '80095394':
                assert body.count('*gpu') == 3  # One pointer declaration and two real status reads.
                body = body.replace('        *gpu;', '        test_read(gpu);')
                body = body.replace('            *gpu,*DMA2_CHCR,*DMA2_MADR);',
                                    '            test_read(gpu),*DMA2_CHCR,*DMA2_MADR);')
                replacements = {
                    'saved=SetIntrMask(0);': 'saved=test_exchange(0);',
                    'SetIntrMask(SavedIntrMask);': 'test_exchange(SavedIntrMask);',
                    '_qin=_qout;': '_qin=test_tail();',
                    '*DMA2_CHCR=0x401;': 'test_dma(DMA2_CHCR,0x401);',
                    '*DPCR|=0x800;': 'test_config(DPCR);',
                    '*GPU_STATUS=0x02000000;': 'test_gpu(GPU_STATUS,0x02000000);',
                    '*GPU_STATUS=0x01000000;': 'test_gpu(GPU_STATUS,0x01000000);',
                }
                for old, new in replacements.items():
                    assert body.count(old) == 1, old
                    body = body.replace(old, new)
            code += body
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(code)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.watchdog = cls.lib.get_alarm
        cls.watchdog.argtypes = []
        cls.watchdog.restype = C.c_int32
        cls.mask = cls.lib.SetIntrMask
        cls.mask.argtypes = [C.c_int32]
        cls.mask.restype = C.c_int32
        cls.ports = (C.c_uint32 * 8).in_dll(cls.lib, 'ports')
        cls.events = ((C.c_uint32 * 3) * 64).in_dll(cls.lib, 'test_events')

    def scalar(self, name):
        return C.c_int32.in_dll(self.lib, name)

    def trace(self):
        return [tuple(row) for row in self.events[:self.scalar('test_event_count').value]]

    def test_signed_time_and_counter_boundaries_and_wrapping(self):
        for now in (-0x80000000, -1, 0, 99, 100, 101, 0x7FFFFFFF):
            for old in (-0x80000000, -1, 0, 0xEFFFF, 0xF0000, 0xF0001, 0x7FFFFFFF):
                self.lib.test_reset()
                self.scalar('test_now').value = now
                self.scalar('GpuAlarmCount').value = old
                before = bytes(self.ports)
                expired = now > 100 or old > 0xF0000
                self.assertEqual(self.watchdog(), -1 if expired else 0)
                expected = old if now > 100 else C.c_int32(old+1).value
                self.assertEqual(self.scalar('GpuAlarmCount').value, expected)
                self.assertEqual(self.scalar('test_diag_calls').value, int(expired))
                if not expired:
                    self.assertEqual(bytes(self.ports), before)
                    self.assertEqual(self.trace(), [(1, 0xFFFFFFFF, 0)])
                    self.assertEqual(self.scalar('_qin').value, 70)
                    self.assertEqual(self.scalar('_qout').value, 5)

    def test_diagnostic_reads_mask_reset_and_full_hardware_sequence(self):
        self.lib.test_reset()
        self.assertEqual(self.watchdog(), -1)
        args = (C.c_uint32 * 4).in_dll(self.lib, 'test_diag_args')
        self.assertEqual(list(args), [1, 0x89ABCDEF, 0xA5000002, 0xA5000001])
        format_pointer = C.c_void_p.in_dll(self.lib, 'test_format').value
        self.assertEqual(C.string_at(format_pointer), b'GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\n')
        self.assertEqual(self.trace(), [
            (1, 0xFFFFFFFF, 0), (2, 0, 0xA5000000), (2, 0, 0x89ABCDEF), (3, 0, 0),
            (4, 0, 0xF123), (5, 0, 0), (6, 2, 0x401), (7, 3, 0xA5000803),
            (8, 0, 0x02000000), (8, 0, 0x01000000), (4, 0xF123, 0)])
        self.assertEqual((self.scalar('_qin').value, self.scalar('_qout').value), (0, 0))
        self.assertEqual(self.scalar('SavedIntrMask').value, 0xF123)
        self.assertEqual(C.c_uint16.in_dll(self.lib, 'mask_storage').value, 0xF123)
        self.assertEqual(list(self.ports)[1:], [0xA5000001, 0x401, 0xA5000803,
                                               0xA5000004, 0xA5000005, 0xA5000006, 0xA5000007])

    def test_fresh_register_pointers_after_helpers_and_each_gpu_write(self):
        for mutation in ('test_mutate_diag', 'test_mutate_mask'):
            self.lib.test_reset()
            self.scalar(mutation).value = 1
            self.scalar('test_swap_write').value = 1
            self.assertEqual(self.watchdog(), -1)
            self.assertEqual(list(self.ports)[2:4], [0xA5000002, 0xA5000003])
            self.assertEqual(list(self.ports)[4:], [0x02000000, 0x401, 0xA5000806, 0x01000000])
            self.assertEqual(self.trace()[-1], (4, 0x1234, 0))
            self.assertEqual(C.c_uint16.in_dll(self.lib, 'mask_storage').value, 0x1234)

    def test_mask_exchange_every_previous_halfword_and_initializer_wrap(self):
        storage = C.c_uint16.in_dll(self.lib, 'mask_storage')
        for value in range(65536):
            storage.value = value
            self.assertEqual(self.mask(-1), value)
            self.assertEqual(storage.value, 65535)
        for now in (-0x80000000, -1, 0, 0x7FFFFFFF):
            self.lib.test_reset()
            self.scalar('test_now').value = now
            self.lib.set_alarm()
            self.assertEqual(self.scalar('GpuTimeoutDeadline').value, C.c_int32(now+240).value)
            self.assertEqual(self.scalar('GpuAlarmCount').value, 0)

    def test_diagnostic_queue_difference_wraps_before_masking(self):
        args = (C.c_uint32 * 4).in_dll(self.lib, 'test_diag_args')
        for head in (-0x80000000, -1, 0, 1, 63, 64, 65, 0x7FFFFFFF):
            for tail in (-0x80000000, -1, 0, 1, 63, 64, 65, 0x7FFFFFFF):
                self.lib.test_reset()
                self.scalar('_qin').value = head
                self.scalar('_qout').value = tail
                self.assertEqual(self.watchdog(), -1)
                self.assertEqual(args[0], (head-tail) & 63)
