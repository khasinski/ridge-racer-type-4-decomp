"""Actual selected-voice cache update: all signed selectors and ordered aliases."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class SpuVoiceCacheTest(unittest.TestCase):
    def test_actual_body_with_overlapping_tone_and_program_sources(self):
        with TemporaryDirectory() as temporary:
            p=Path(temporary)
            s=(ROOT/'tools/scripts/fixtures/views/func_80086E74.c').read_text()
            s=re.sub(r'^#include.*\n','',s,flags=re.M)
            s=re.sub(r'^extern .*;\n','',s,flags=re.M)
            s=re.sub(r' asm\("\$\d+"\)','',s)
            s=re.sub(r'__asm__\(""[^;]*;', ';',s)
            assert s.count('(u8 *)0x80110000U') == 7
            s=s.replace('(u8 *)0x80110000U','page')
            prefix=r'''
            #include <stdint.h>
            #include <stddef.h>
            #include <stdio.h>
            #include <stdlib.h>
            #include <string.h>
            #include "rr/layout.h"
            #undef RR_ASSERT_SIZE
            #undef RR_ASSERT_OFFSET
            #define RR_ASSERT_SIZE(t,s)
            #define RR_ASSERT_OFFSET(t,f,o)
            #undef RR_FIELD_OFFSET
            #define RR_FIELD_OFFSET(t,f) offsetof(t,f)
            #include "rr/spu_voice_cache.h"
            _Static_assert(sizeof(RrSpuProgram)==16,"program");
            _Static_assert(sizeof(RrSpuTone32)==32,"tone");
            _Static_assert(sizeof(RrSpuVolumeCache)==416,"cache");
            _Static_assert(sizeof(RrChannelControlRow36)==54,"row");
            _Static_assert(offsetof(RrSpuProgram,value0C)==12,"program0C");
            _Static_assert(offsetof(RrSpuProgram,value0E)==14,"program0E");
            _Static_assert(offsetof(RrSpuCachedVolumeEntry,program06)==6,"program06");
            _Static_assert(offsetof(RrSpuCachedVolumeEntry,tone08)==8,"tone08");
            _Static_assert(offsetof(RrChannelControlRow36,value06)==6,"row06");
            typedef union { uint64_t align; unsigned char bytes[4096]; } World;
            static World initial, actual, expected;
            static unsigned char *page;
            static RrSpuProgram programs[32768];
            static RrSpuTone32 tones[4352];
            static RrSpuProgram *program_ptr;
            static u8 *tone_ptr;
            #define AT(t,o) (*(t *)(page+(o)))
            #define SvmVoiceIndex AT(u16,280)
            #define _svm_envx_hist ((u32 *)(page-64))
            #define D_80110116 ((s16 *)(page+278))
            #define D_80110107 ((s8 *)(page+263))
            #define _svm_damper AT(u16,160)
            #define _svm_pg program_ptr
            #define _svm_tn tone_ptr
            #define D_8010F90E ((u16 *)(page-1778))
            #define D_8010FE3E ((u16 *)(page-450))
            #define D_8010FE40 ((u16 *)(page-448))
            #define rr_flags ((u8 *)(page-488))
            #define _svm_sreg_dirty AT(RrSpuVolumeCache,-488)
            '''
            reference=r'''
            static uint16_t rd16(const void *p) {uint16_t v;memcpy(&v,p,2);return v;}
            static void wr16(void *p,uint16_t v){memcpy(p,&v,2);}
            static uint32_t rd32(const void *p){uint32_t v;memcpy(&v,p,4);return v;}
            static void wr32(void *p,uint32_t v){memcpy(p,&v,4);}
            static void reference(void){
             unsigned voice=rd16(page+280); uint32_t scaled=voice*8;
             wr16(page-1778+(int16_t)voice*54,32767);
             for(int i=0;i<16;i++){
              unsigned shift=rd16(page+280);
              wr32(page-64+i*4,rd32(page-64+i*4)&~(UINT32_C(1)<<shift));
             }
             uint16_t raw=rd16(page+278); int prog=(int16_t)raw;
             int index=(prog-1)/2;
             uint16_t value=rd16((unsigned char *)&program_ptr[index]+((raw&1)?12:14));
             int offset=(int16_t)scaled*2;
             wr16(page-450+offset,value);
             int v=(int16_t)rd16(page+280); page[-488+v]|=8;
             int bank=(int8_t)page[263], note=(int8_t)page[268];
             unsigned char *tone=tone_ptr+(bank*16+note)*32;
             wr16(page-448+offset,rd16(tone+16));
             uint16_t flags=rd16(tone+18);
             uint32_t sum=rd16(page+160)+(flags&31);
             if((int16_t)sum>=32)sum=31;
             wr16(page-446+offset,(uint16_t)(sum|(flags&65504)));
             v=(int16_t)rd16(page+280);page[-488+v]|=48;
            }
            static uint32_t rng=0x519a21;
            static uint32_t next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
            static void bind(World *w,int mode,int tone_index){
             page=w->bytes+2048;
             program_ptr=programs+16384;
             tone_ptr=(u8 *)(tones+2176);
             int offset=rd16(page+280)*16;
             if(mode==1)tone_ptr=page-448+offset-16-tone_index*32;
             if(mode==2)tone_ptr=page-448+offset-18-tone_index*32;
             if(mode==3)program_ptr=(RrSpuProgram *)(page-1778+(int16_t)rd16(page+280)*54-12);
             if(mode==4)tone_ptr=page-450+offset-16-tone_index*32;
             if(mode==5)tone_ptr=page-64-16-tone_index*32;
            }
            int main(void){
             for(size_t i=0;i<sizeof(programs);i++)((u8 *)programs)[i]=next();
             for(size_t i=0;i<sizeof(tones);i++)((u8 *)tones)[i]=next();
             for(unsigned k=0;k<98304;k++){
              for(size_t i=0;i<sizeof(initial);i++)initial.bytes[i]=next();
              page=initial.bytes+2048;
              wr16(page+280,k%24);
              int mode=k<65536?0:1+(k%5);
              int16_t prog=k<65536?(int16_t)k:(int16_t)next();
              int8_t bank=k<65536?(int8_t)(k>>8):(int8_t)next();
              int8_t note=k<65536?(int8_t)k:(int8_t)next();
              if(mode==3)prog=1;
              if(mode)bank=note=0;
              wr16(page+278,(uint16_t)prog);
              page[263]=(uint8_t)bank;page[268]=(uint8_t)note;
              static const uint16_t bases[]={0,1,30,31,32,32736,32767,32768,65504,65535};
              wr16(page+160,k%2?bases[(k/2)%10]:(uint16_t)next());
              actual=expected=initial;
              int ti=bank*16+note;
              bind(&actual,mode,ti);_SsVmDoAllocate();
              bind(&expected,mode,ti);reference();
              if(memcmp(&actual,&expected,sizeof(actual))){fprintf(stderr,"case %u mode %d failed\n",k,mode);return 1;}
             }
             puts("98304 native cases: OK (UBSan)");return 0;
            }
            '''
            (p/'native.c').write_text(prefix+s+reference)
            r=subprocess.run(['clang','-std=c11','-O2','-g','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude','-I'+str(p),str(p/'native.c'),'-o',str(p/'native')],capture_output=True,text=True,cwd=ROOT)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            r=subprocess.run([str(p/'native')],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            self.assertIn('98304 native cases: OK',r.stdout)
