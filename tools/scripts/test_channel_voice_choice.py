"""Actual voice selection: signed priority ties, reserved voices and age wrap."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class ChannelVoiceChoiceTest(unittest.TestCase):
    def test_actual_body_and_complete_control_row_owner(self):
        with TemporaryDirectory() as temporary:
            p=Path(temporary)
            s=(ROOT/'tools/scripts/fixtures/views/func_80086BE4.c').read_text()
            s=re.sub(r'^#include.*\n','',s,flags=re.M)
            s=re.sub(r' asm\(\s*"\$\d+"\s*\)','',s)
            s=re.sub(r'__asm__\(""[^;]*;', ';',s)
            page_count=s.count('(u8 *)0x80110000U')
            assert page_count==16,page_count
            s=s.replace('(u8 *)0x80110000U','native_page')
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
            #include "rr/channel_voice_choice.h"
            _Static_assert(sizeof(RrChannelControlRow36)==54,"row stride");
            _Static_assert(offsetof(RrChannelControlRow36,age02)==2,"age");
            _Static_assert(offsetof(RrChannelControlRow36,value06)==6,"gate");
            _Static_assert(offsetof(RrChannelControlRow36,priority1A)==26,"priority");
            _Static_assert(offsetof(RrChannelControlRow36,flags1D)==29,"active");
            _Static_assert(offsetof(RrChannelControlRow36,pad1E)==30,"tail");
            typedef union {uint64_t alignment;uint8_t bytes[4096];} World;
            static World initial,actual,expected;
            static uint8_t *page;
            #define SvmCurTonePriority (*(u8 *)(page+271))
            #define spuVmMaxVoice (*(s8 *)(page+252))
            #define _snd_vmask (*(u32 *)(page-1900))
            '''
            reference=r'''
            static uint16_t read16(const void *p){uint16_t v;memcpy(&v,p,2);return v;}
            static void write16(void *p,uint16_t v){memcpy(p,&v,2);}
            static uint32_t read32(const void *p){uint32_t v;memcpy(&v,p,4);return v;}
            static void write32(void *p,uint32_t v){memcpy(p,&v,4);}
            static uint8_t *row(int i){return page-1784+i*54;}
            static int reference(void){
             int count=(int8_t)page[252];
             int threshold=(int8_t)page[271];
             uint32_t mask=read32(page-1900);
             int chosen=99,best=99,ties=0;
             uint16_t gate_best=65535,age_best=0;
             for(int i=0;i<count;i++){
              uint8_t *r=row(i);
              if(mask&(UINT32_C(1)<<i))continue;
              uint16_t gate=read16(r+6),age=read16(r+2),raw_priority=read16(r+26);
              if(r[29]==0 && gate==0){chosen=i;break;}
              int cmp=(int16_t)raw_priority;
              int cutoff=(uint16_t)threshold;
              if(cmp<cutoff){
               threshold=raw_priority;best=i;gate_best=gate;age_best=age;ties=1;
              }else if(cmp==cutoff){
               ties++;
               if(gate<gate_best){best=i;gate_best=gate;age_best=age;}
               else if(gate==gate_best && (int32_t)age_best<(int16_t)age){best=i;age_best=age;}
              }
             }
             if(chosen==99)chosen=(ties&255)?best:(uint8_t)page[252];
             if((uint8_t)chosen<count){
              mask=read32(page-1900);
              for(int i=0;i<count;i++)if(!(mask&(UINT32_C(1)<<i)))write16(row(i)+2,(uint16_t)(read16(row(i)+2)+1));
              uint8_t *r=row((uint8_t)chosen);
              write16(r+2,0);write16(r+42,0);write16(r+30,0);write16(r+26,(uint16_t)(int16_t)(int8_t)page[271]);
             }
             return (uint8_t)chosen;
            }
            static uint32_t rng=0x53a1bb90;
            static uint32_t next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
            static void run(unsigned k,int known){
             actual=expected=initial;
             page=actual.bytes+2048;int got=_SsVmAlloc();
             page=expected.bytes+2048;int want=reference();
             if(got!=want || memcmp(&actual,&expected,sizeof(actual)) || (known>=0 && got!=known)){
              fprintf(stderr,"case %u: got %d reference %d known %d\n",k,got,want,known);exit(1);
             }
            }
            int main(void){
             const uint16_t edges[]={0,1,127,128,255,256,32767,32768,65534,65535};
             for(unsigned k=0;k<65536;k++){
              for(size_t i=0;i<sizeof(initial);i++)initial.bytes[i]=next();
              page=initial.bytes+2048;
              int count=k<128?(int)k-128:(int)(k%25);
              page[252]=(uint8_t)count;page[271]=(uint8_t)(k/7);
              uint32_t mask=next();unsigned mode=k%8;
              if(mode==1 || mode==3 || mode==4 || mode==6)mask=0;
              if(mode==5)mask=UINT32_MAX;
              write32(page-1900,mask);
              for(int i=0;i<24;i++){
               uint8_t *r=row(i);
               if(mode==1 || mode==4){r[29]=1;write16(r+26,(uint16_t)((k/8)%128));write16(r+6,edges[(k/64+i)%10]);write16(r+2,edges[(k/640+i)%10]);}
               if(mode==3){r[29]=1;write16(r+26,(uint16_t)(int16_t)(k/8-4096));}
               if(mode==6){r[29]=1;write16(r+26,32767);}
              }
              if((mode==2 || mode==7) && count>0){int i=(k/8)%count;row(i)[29]=0;write16(row(i)+6,0);}
              run(k,-1);
             }
             // Named signedness/ordering checks, independently known selected indices.
             memset(&initial,0xA5,sizeof(initial));page=initial.bytes+2048;
             page[252]=2;page[271]=0;write32(page-1900,0);
             for(int i=0;i<2;i++){row(i)[29]=1;write16(row(i)+6,1);write16(row(i)+2,0);}
             write16(row(0)+26,65535);write16(row(1)+26,0);run(65536,1);
             page=initial.bytes+2048;page[271]=127;
             write16(row(0)+26,0);write16(row(1)+26,0);write16(row(0)+2,2);write16(row(1)+2,32768);run(65537,0);
             page=initial.bytes+2048;write16(row(0)+6,32768);write16(row(1)+6,32767);run(65538,1);
             page=initial.bytes+2048;row(0)[29]=0;write16(row(0)+6,0);write16(row(1)+2,65535);run(65539,0);
             page=initial.bytes+2048;write32(page-1900,1);run(65540,1);
             page=initial.bytes+2048;write32(page-1900,0);page[271]=0;
             for(int i=0;i<2;i++){row(i)[29]=1;write16(row(i)+26,0);write16(row(i)+6,65535);write16(row(i)+2,32768);}
             run(65541,99);
             puts("65542 native cases: OK (UBSan)");return 0;
            }
            '''
            prefix=re.sub(r'\bpage\b','native_page',prefix)
            reference=re.sub(r'\bpage\b','native_page',reference)
            (p/'native.c').write_text(prefix+s+reference)
            r=subprocess.run(['clang','-std=gnu11','-O2','-g','-fsanitize=undefined','-fno-sanitize-recover=all','-Iinclude','-I'+str(p),str(p/'native.c'),'-o',str(p/'native')],capture_output=True,text=True,cwd=ROOT)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            r=subprocess.run([str(p/'native')],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            self.assertIn('65542 native cases: OK',r.stdout)
