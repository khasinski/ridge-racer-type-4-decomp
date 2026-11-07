"""Actual rectangle diagnostic and coordinate helpers sharing real settings."""
import ctypes as C
from itertools import product
from pathlib import Path
import random
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest


class Rect(C.Structure):
    _fields_ = [('x', C.c_int16), ('y', C.c_int16), ('w', C.c_int16), ('h', C.c_int16)]


class Settings(C.Structure):
    _fields_ = [('level', C.c_uint8), ('mode', C.c_uint8), ('width', C.c_int16), ('height', C.c_int16)]


Report = C.CFUNCTYPE(C.c_int32, C.c_int32, C.c_int32, C.c_void_p, C.c_void_p,
                    C.c_int32, C.c_int32, C.c_int32, C.c_int32)


class GpuRectangleDiagnosticTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        root = Path(__file__).resolve().parents[2]
        headers = ''
        for name in ('gpu', 'library'):
            text = (root/('include/rr/' + name + '.h')).read_text()
            text = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', text, flags=re.M)
            text = text.replace('extern s16 GpuVramWidth, GpuVramHeight;', '')
            headers += text
        code = '''#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
typedef uint8_t u8;typedef int8_t s8;typedef uint16_t u16;typedef int16_t s16;typedef uint32_t u32;typedef int32_t s32;
''' + headers + '''
RrGpuDebugSettings GpuDebugLevel;
#define GpuVramWidth GpuDebugLevel.width
#define GpuVramHeight GpuDebugLevel.height
char test_rom[0x2000];
#define StrBadRectFmt (test_rom+0x1EA8)
#define StrRectFmt (test_rom+0x1EB4)
#define GpuLabelColonFmt (test_rom+0x1EC8)
u32 test_layout[]={sizeof(RrGpuDebugSettings),offsetof(RrGpuDebugSettings,width),offsetof(RrGpuDebugSettings,height),sizeof(RrGpuRectangle)};
s32 (*test_report)(s32,s32,const char *,const char *,s32,s32,s32,s32);
s32 collect(s32 which,const char *format,va_list args) {
 if(format==StrRectFmt){
  s32 x=va_arg(args,s32),y=va_arg(args,s32),w=va_arg(args,s32),h=va_arg(args,s32);
  return test_report(2,which,format,0,x,y,w,h);
 }
 return test_report(format==StrBadRectFmt?0:1,which,format,va_arg(args,const char *),0,0,0,0);
}
s32 test_printf1(const char *format,...){s32 r;va_list args;va_start(args,format);r=collect(1,format,args);va_end(args);return r;}
s32 test_printf2(const char *format,...){s32 r;va_list args;va_start(args,format);r=collect(2,format,args);va_end(args);return r;}
s32 (*GPU_printf)(const char *,...)=test_printf1;
'''
        for name in ('func_80092DB8.c', 'gpu_coordinates.c'):
            body = (root/('src/main/PAL/' + name)).read_text()
            body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
            body = body.replace('extern char StrBadRectFmt[],StrRectFmt[],GpuLabelColonFmt[];', '')
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = re.sub(r'"\$[0-9]+",\s*', '', body)
            body = re.sub(r':\s*"\$[0-9]+"(?:,\s*"\$[0-9]+")*(?=\))', '', body)
            if name == 'func_80092DB8.c':
                assert body.count('(const char *)0x80010000U') == 1
                body = body.replace('(const char *)0x80010000U', 'test_rom')
            code += body
        directory = Path(cls.temp.name)
        (directory/'function.c').write_text(code)
        result = subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory/'function.c'),
                                 '-o', str(directory/'function.so')], capture_output=True)
        if result.returncode:
            raise AssertionError(result.stderr.decode())
        cls.lib = C.CDLL(str(directory/'function.so'))
        cls.settings = Settings.in_dll(cls.lib, 'GpuDebugLevel')
        cls.rom = (C.c_char * 0x2000).in_dll(cls.lib, 'test_rom')
        cls.rom[:] = (root/'assets/PAL/main.exe').read_bytes()[0x800:0x2800]
        cls.function = cls.lib.checkRECT
        cls.function.argtypes = [C.c_void_p, C.c_void_p]
        cls.function.restype = None
        cls.coordinates = [cls.lib.get_cs, cls.lib.get_ce]
        for function in cls.coordinates:
            function.argtypes = [C.c_int32, C.c_int32]
            function.restype = C.c_uint32
        if list((C.c_uint32 * 4).in_dll(cls.lib,'test_layout')) != [6,2,4,8]:
            raise AssertionError('Native settings/rectangle layout differs from recovered fields')

    def setUp(self):
        self.settings.level=1;self.settings.mode=0xA5;self.settings.width=320;self.settings.height=240
        self.events=[];self.errors=[];self.hook=None
        self.label=C.create_string_buffer(b'actual operation name')
        def report(kind, which, format_pointer, label, x, y, w, h):
            try:
                self.events.append((kind,which,C.string_at(format_pointer),label,(x,y,w,h)))
                if self.hook:self.hook(kind)
            except BaseException as error:
                self.errors.append(error)
            return -123
        self.report=Report(report)
        C.c_void_p.in_dll(self.lib,'test_report').value=C.cast(self.report,C.c_void_p).value
        C.c_void_p.in_dll(self.lib,'GPU_printf').value=C.cast(self.lib.test_printf1,C.c_void_p).value

    def invoke(self, rectangle):
        self.function(self.label,C.byref(rectangle) if rectangle is not None else None)
        if self.errors:raise self.errors.pop(0)

    def check(self, values, limits):
        self.settings.width,self.settings.height=limits
        rect=Rect(*values);before=bytes(rect);settings_before=bytes(self.settings)
        self.events.clear();self.invoke(rect)
        x,y,w,h=values;maxw,maxh=limits
        valid=0<w<=maxw and 0<=x and x+w<=maxw and 0<=y<=maxh and 0<h and y+h<=maxh
        expected=[] if valid else [
            (0,1,b'%s:bad RECT',C.addressof(self.label),(0,0,0,0)),
            (2,1,b'(%d,%d)-(%d,%d)\n',None,values)]
        self.assertEqual(self.events,expected)
        self.assertEqual(bytes(rect),before)
        self.assertEqual(bytes(self.settings),settings_before)

    def test_all_levels_and_no_rectangle_access_for_other_levels(self):
        for level in range(256):
            self.settings.level=level;self.events.clear()
            if level in (1,2):self.invoke(Rect(0,0,1,1))
            else:self.invoke(None)
            expected=[] if level!=2 else [
                (1,1,b'%s:',C.addressof(self.label),(0,0,0,0)),
                (2,1,b'(%d,%d)-(%d,%d)\n',None,(0,0,1,1))]
            self.assertEqual(self.events,expected)

    def test_signed_limits_rectangle_boundaries_and_nonoverflowing_sums(self):
        values=(-32768,-1,0,1,2,239,240,319,320,32767)
        for limits in ((320,240),(1,1),(32767,32767),(0,0),(-1,240),(-32768,32767)):
            for rectangle in product(values,repeat=4):self.check(rectangle,limits)

    def test_random_signed_halfwords_against_independent_validity_oracle(self):
        rng=random.Random(0x92DB8)
        for _ in range(10000):
            values=tuple(rng.randrange(-32768,32768) for _ in range(4))
            limits=tuple(rng.randrange(-32768,32768) for _ in range(2))
            self.check(values,limits)

    def test_second_report_reloads_rectangle_and_replacement_callback(self):
        for level in (1,2):
            self.setUp();self.settings.level=level
            rect=Rect(1,2,0,0)
            def mutate(kind):
                if kind in (0,1):
                    rect.x=-32768;rect.y=32767;rect.w=-123;rect.h=456
                    self.settings.level=0
                    C.c_void_p.in_dll(self.lib,'GPU_printf').value=C.cast(self.lib.test_printf2,C.c_void_p).value
            self.hook=mutate;self.invoke(rect)
            self.assertEqual(len(self.events),2)
            self.assertEqual(self.events[1],(2,2,b'(%d,%d)-(%d,%d)\n',None,(-32768,32767,-123,456)))

    def test_coordinate_helpers_use_the_same_limit_storage(self):
        for width,height in ((320,240),(1024,512),(1,1)):
            self.settings.width=width;self.settings.height=height
            for function,prefix in zip(self.coordinates,(0xE3000000,0xE4000000)):
                self.assertEqual(function(width+5,height+5),prefix|((width-1)&1023)|(((height-1)&1023)<<10))
            self.events.clear();self.invoke(Rect(0,0,width,height));self.assertEqual(self.events,[])
            self.settings.width=width-1
            self.invoke(Rect(0,0,width,height));self.assertEqual(len(self.events),2)
