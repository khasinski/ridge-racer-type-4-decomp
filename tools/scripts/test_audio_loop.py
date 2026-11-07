"""Actual audio loop and volume wrapper with mutable helper-boundary snapshots."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class AudioLoopTest(unittest.TestCase):
    def test_actual_loop_and_promoted_volume_wrapper(self):
        s=(ROOT/'tools/scripts/fixtures/legacy/func_800518E0.c').read_text()
        s=re.sub(r'^#include.*\n','',s,flags=re.M)
        s=re.sub(r' asm\("\$\d+"\)','',s)
        s=re.sub(r'__asm__\(""[^;]*;',';',s)
        helper=(ROOT/'tools/scripts/fixtures/legacy/func_8004FE7C.c').read_text()
        helper=re.sub(r'^#include.*\n','',helper,flags=re.M)
        prefix=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,s)
#define RR_ASSERT_OFFSET(t,f,o)
#include "rr/audio_loop.h"
_Static_assert(sizeof(RrAudioLoopState)==0x1A08,"state extent");
_Static_assert(offsetof(RrAudioLoopState,muted04)==4,"mute");
_Static_assert(offsetof(RrAudioLoopState,master08)==8,"master alias");
_Static_assert(offsetof(RrAudioLoopState,gain18)==24,"gain");
_Static_assert(offsetof(RrAudioLoopState,bank26)==38,"bank");
_Static_assert(offsetof(RrAudioLoopState,left19F8)==0x19F8,"left");
_Static_assert(offsetof(RrAudioLoopState,right19FA)==0x19FA,"right");
_Static_assert(offsetof(RrAudioLoopState,pitch19FC)==0x19FC,"pitch");
_Static_assert(offsetof(RrAudioLoopState,active1A00)==0x1A00,"active");
_Static_assert(offsetof(RrAudioLoopState,program1A04)==0x1A04,"program");
_Static_assert(sizeof(RrAudioSettings)==52,"settings");
_Static_assert(offsetof(RrAudioSettings,volume31)==49,"volume");
_Static_assert(offsetof(RrAudioSettings,mono32)==50,"mono");
typedef struct {RrAudioLoopState audio;RrRuntimeSettings settings;s16 mode;u8 canary[26];} World;
typedef struct {int kind;u32 args[8];World snapshot;} Entry;
static World initial,actual,expected,*w;
static Entry entries[8],actual_entries[8];
static unsigned seed;
static int mutation,count;
#define SoundState (w->audio)
#define Options (w->settings)
#define RaceType (w->mode)
#define VoiceVolumeScale (w->audio.master08)
'''
        reference=r'''
static u32 emit(int kind,u32 a,u32 b,u32 c,u32 d,u32 e,u32 f,u32 g,u32 hh){
 assert(count<8);Entry *entry=&entries[count];memset(entry,0,sizeof(*entry));entry->kind=kind;
 entry->args[0]=a;entry->args[1]=b;entry->args[2]=c;entry->args[3]=d;entry->args[4]=e;entry->args[5]=f;entry->args[6]=g;entry->args[7]=hh;entry->snapshot=*w;
 unsigned sequence=(unsigned)count++;
 if(mutation){
  w->audio.master08=(s32)(seed^((sequence+1U)*0x9E3779B9U));
  w->audio.gain18=(s16)(u16)(seed+sequence*127U);
  w->audio.left19F8^=0xABCD;w->audio.right19FA^=0x7F81;
  w->audio.bank26=(s16)((u16)w->audio.bank26+257U);
  w->audio.program1A04=(s16)((u16)w->audio.program1A04+513U);
  w->audio.pitch19FC^=0xA57F8311U+sequence;
  w->audio.active1A00=(seed&4)?-77:0;
  w->audio.muted04^=0xA5;
  w->settings.audio.volume31+=37;w->settings.audio.mono32^=0x4000;
  w->mode=(s16)(u16)(seed+sequence);
 }
 return seed|0x80000000U;
}
s32 SsUtSetVVol(s32 voice,s32 left,s32 right){return (s32)emit(0,voice,left,right,0,0,0,0,0);}
s32 SsUtChangePitch(s32 voice,s32 bank,s32 program,s32 group,s32 unused,u32 first,u32 second){return (s32)emit(1,voice,bank,program,group,unused,first,second,0);}
s32 SsUtKeyOnV(s32 voice,s32 bank,s32 program,s32 mode,s32 group,s32 unused,s32 left,s32 right){return (s32)emit(2,voice,bank,program,mode,group,unused,left,right);}
s32 SsUtKeyOffV(s32 voice){return (s32)emit(3,voice,0,0,0,0,0,0,0);}
static u16 r16(const u8 *p){u16 v;memcpy(&v,p,2);return v;}
static u32 r32(const u8 *p){u32 v;memcpy(&v,p,4);return v;}
static void w32(u8 *p,u32 v){memcpy(p,&v,4);}
static int narrow(u32 v){v&=65535;return v<32768?(int)v:(int)v-65536;}
static int signed32(u32 v){return v<=INT32_MAX?(int)v:(int)((int64_t)v-INT64_C(4294967296));}
static int floor128(int64_t v){return v>=0?(int)(v/128):-(int)((-v+127)/128);}
static void volume_reference(u32 voice,int left,int right){
 int master=signed32(r32((u8*)&w->audio+8));
 int lp=signed32((u32)((uint64_t)(u32)left*(u32)master));
 int l=narrow((u32)floor128(lp));
 int rp=signed32((u32)((uint64_t)(u32)right*(u32)master));
 int r=narrow((u32)floor128(rp));
 emit(0,(u32)narrow(voice),(u32)l,(u32)r,0,0,0,0,0);
}
static void reference(void){
 u8 *audio=(u8*)&w->audio,*settings=(u8*)&w->settings;
 int left=narrow(r16(audio+0x19F8)),right=narrow(r16(audio+0x19FA));
 if(r16(settings+50))left=right=(left+right)/2;
 if(r32(audio+4)||w->mode==5)left=right=0;
 int gain=narrow(r16(audio+24)),volume=settings[49];
 int l=narrow((u32)((left*gain)/128))*volume/16;
 int r=narrow((u32)((right*gain)/128))*volume/16;
 left=narrow((u32)l);right=narrow((u32)r);
 if(left+right>0){
  u32 pitch=r32(audio+0x19FC);
  u32 first=(u32)narrow((pitch<<9)>>16),second=pitch&127;
  for(int voice=6;voice<=7;voice++){
   volume_reference(voice,left,right);
   emit(1,voice,(u32)narrow(r16(audio+38)),(u32)narrow(r16(audio+0x1A04)),60,0,first,second,0);
  }
  if(!r32(audio+0x1A00)){
   emit(2,6,(u32)narrow(r16(audio+38)),(u32)narrow(r16(audio+0x1A04)),0,60,0,(u32)left,(u32)right);
   w32(audio+0x1A00,1);
  }
 }else if(r32(audio+0x1A00)){
  emit(3,6,0,0,0,0,0,0,0);w32(audio+0x1A00,0);
 }
}
static u32 state=0x74C88319;
static u32 next(void){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
static void bind(World *owner){w=owner;count=0;memset(entries,0,sizeof(entries));}
static int compare(unsigned k){if(memcmp(&actual,&expected,sizeof(actual))||memcmp(entries,actual_entries,sizeof(entries))){fprintf(stderr,"case%u mutation%d failed\n",k,mutation);return 1;}return 0;}
int main(void){
 assert(sizeof(void*)>4 && (uintptr_t)&actual>UINT32_MAX);
 for(size_t i=0;i<sizeof(initial);i++)((u8*)&initial)[i]=(u8)next();
 unsigned coverage[6]={0};
 for(unsigned k=0;k<131072;k++){
  seed=k*2654435761U;mutation=k>=65536;actual=initial;
  actual.audio.left19F8=(u16)k;actual.audio.right19FA=(u16)(k*31337U+0x1234U);
  actual.audio.master08=(s32)seed;actual.audio.gain18=(s16)(u16)(k*251U);
  actual.audio.muted04=k%17?0:((s32)seed|1);
  actual.audio.bank26=(s16)(u16)seed;actual.audio.program1A04=(s16)(u16)(seed>>16);
  actual.audio.pitch19FC=seed^(seed>>13);
  actual.audio.active1A00=k%3?((s32)seed|1):0;
  actual.settings.audio.volume31=(u8)(k*17U);actual.settings.audio.mono32=k&1?(s16)(u16)(seed|1):0;
  actual.mode=k%7?(s16)(u16)k:5;expected=actual;
  bind(&actual);SoundLoopUpdate();int actual_count=count;assert(count>=0&&count<=5);coverage[count]++;memcpy(actual_entries,entries,sizeof(entries));
  bind(&expected);reference();
  if(count!=actual_count||compare(k))return 1;
 }
 assert(coverage[0]&&coverage[1]&&coverage[4]&&coverage[5]);
 const u32 voices[]={0,0x8000,0xFFFFFFFF,0x12340006,0x80000007,0xFFFF,0x7FFF0000,0x8000FFFF};
 const int levels[]={INT32_MIN,INT32_MAX,0x12345678,-0x12345678,0,1,32767,-32768};
 mutation=0;
 for(unsigned i=0;i<8;i++)for(unsigned j=0;j<8;j++){
  unsigned k=i*8+j;seed=k;actual=initial;actual.audio.master08=levels[(i+j)%8];expected=actual;
  bind(&actual);SetVoiceVolumeScaled((s32)voices[i],levels[j],levels[(j+3)%8]);memcpy(actual_entries,entries,sizeof(entries));
  bind(&expected);volume_reference(voices[i],levels[j],levels[(j+3)%8]);if(count!=1||compare(131072+k))return 1;
 }
 puts("131072 loop cases and64 promoted helper cases: OK (UBSan)");return 0;
}
'''
        with TemporaryDirectory() as temporary:
            p=Path(temporary)
            source=p/'audio.c'
            source.write_text(prefix+helper+s+reference)
            executable=p/'audio'
            subprocess.run(['clang','-std=gnu11','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I'+str(ROOT/'include'),
                            str(source),'-o',str(executable)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(executable)],check=True,capture_output=True,text=True)
            self.assertIn('131072 loop cases and64 promoted helper cases: OK (UBSan)',result.stdout)


if __name__=='__main__':
    unittest.main()
