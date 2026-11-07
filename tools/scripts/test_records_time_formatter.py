"""Time buffer layout and signed32 formatting, including preserved separators."""
from overlay_units import read_c_source
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay663TimeFormatterTest(unittest.TestCase):
    def test_signed32_values_and_original_template(self):
        root=Path(__file__).resolve().parents[2]
        blob=(root/'build/overlays/663.bin').read_bytes()
        offset=0x80116874-0x80114878
        self.assertEqual(blob[offset:offset+10],b'00\'00"000\0')
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "rr/overlay663_time.h"
static struct {u32 before[4];u8 text[10];u8 padding[6];u32 after[4];} buffer,expected;
#define records_time_text buffer.text
''' + read_c_source(root/'src/overlays/PAL/records/func_80115EE8.c') + r'''
static u32 seed=0x15EE8;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void check(s32 input,unsigned template){
 unsigned i;int64_t value=input,minutes=value/60000,seconds=(value/1000)%60,millis=value%1000;u8 *result;
 for(i=0;i<sizeof(buffer);i++)((u8*)&buffer)[i]=(u8)next();
 if(template)memcpy(buffer.text,"00'00\"000",10);
 memcpy(&expected,&buffer,sizeof(buffer));
 expected.text[0]=(u8)(minutes/10+48);expected.text[1]=(u8)(minutes%10+48);
 expected.text[3]=(u8)(seconds/10+48);expected.text[4]=(u8)(seconds%10+48);
 expected.text[6]=(u8)(millis/100+48);expected.text[7]=(u8)((millis%100)/10+48);expected.text[8]=(u8)(millis%10+48);
 result=records_format_time(input);assert(result==buffer.text);assert((uintptr_t)result>UINT32_MAX);assert(memcmp(&buffer,&expected,sizeof(buffer))==0);
}
int main(void){
 unsigned i;static const s32 cases[]={INT32_MIN,INT32_MIN+1,-6000001,-6000000,-5999999,-60001,-60000,-59999,-1001,-1000,-999,-101,-100,-99,-11,-10,-9,-1,0,1,9,10,11,99,100,101,999,1000,1001,59999,60000,60001,5999999,6000000,6000001,INT32_MAX-1,INT32_MAX};
 for(i=0;i<sizeof(cases)/sizeof(cases[0]);i++){check(cases[i],0);check(cases[i],1);}
 for(i=0;i<1048576;i++){u32 bits=next();s32 value;memcpy(&value,&bits,4);check(value,i&1);}
 for(i=0;i<120001;i++){check((s32)i,1);check(-(s32)i,1);}
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
