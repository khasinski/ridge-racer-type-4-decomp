"""Guarded native oracle for overlay665 audio options and mixed-width selection."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re
import subprocess

root = Path(__file__).resolve().parents[2]
body = (root/'src/overlays/PAL/options/func_801151A4.c').read_text()
body = body[body.index('void options_update_audio_menu'):]
body = re.sub(r' __asm__\("\$\d+"\)', '', body)
source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay665_audio_options.h"
_Static_assert(sizeof(RrUiRow24)==36,"UI stride");
_Static_assert(sizeof(RrOverlay665VisualState)==160,"visual owner");
_Static_assert(offsetof(RrOverlay665VisualState,exit34)==52,"selection offset");
_Static_assert(sizeof(RrMenuSelectionWord32)==4,"selection word");
_Static_assert(sizeof(RrRuntimeSettings)==60,"settings owner");
_Static_assert(offsetof(RrAudioSettings,mono32)==50,"mono offset");
_Static_assert(sizeof(RrFrameOrderingPrefix)==0xB70,"complete frame");
typedef struct {
 u32 guard[4];RrOverlay665VisualState visual;RrRuntimeSettings settings;
 s32 music,effects; s16 timer,gain;RrMenuItemState5C input[2];
 RrUiRow24 rows[2][8];RrFrameOrderingPrefix frames[2];
 unsigned rowRoot,frameRoot;u32 tail[4];
} State;
static State actual,reference,*w;
#define options_menu_state w->visual
#define options_music_volume_setting w->music
#define options_sfx_volume_setting w->effects
#define Options w->settings
#define MenuFadeTimer w->timer
#define MusicSeqVolume w->gain
#define PlayerControllers w->input
#define MenuConfirmPressed w->input[0].a
#define UiRows w->rows[w->rowRoot]
#define CurrentDrawBuf (&w->frames[w->frameRoot])
''' + body + r'''
typedef struct {unsigned kind;s32 args[5];State before;} Event;
static Event events[24];
static unsigned scenario,profile,serial,count,seen,coverage[11],states[11],inputKinds[4],fadeKinds[2],fadeTaken[2],audioTaken[3];
static int referenceRun;
static s32 word(u32 a){s32 b;memcpy(&b,&a,4);return b;}
static s16 half(u16 a){s16 b;memcpy(&b,&a,2);return b;}
static void record(unsigned kind,const s32 *args){assert(kind<11);
 if(referenceRun){assert(count<24);Event *e=&events[count++];e->kind=kind;memcpy(e->args,args,20);e->before=*w;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(args,e->args,20)&&!memcmp(w,&e->before,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*kind+31*n)*0x9E3779B9U;
 w->visual.layout0C=word(v);w->visual.scene00=word(~v);w->visual.exit34.word=(v>>4)&3;
 w->settings.audio.mono32=(v>>3)&1;w->settings.bytes[48]=(u8)v;w->settings.bytes[49]=(u8)(v>>7);
 w->timer=half(scenario+31*n);w->gain=half(v);w->music=word(v);w->effects=word(v>>1);
 w->rowRoot=(scenario+n)&1;w->frameRoot=(scenario+n+1)&1;
 w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<8;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+n+i+j)%9;
}
static void service(unsigned kind){const s32 args[5]={0};record(kind,args);effect(kind);}
void options_init_audio_rows(void){service(0);}
void AddScreenFadeTiles(u32 *p,s32 b,u32 c){unsigned k,slot;int found=0;for(k=0;k<2&&!found;k++)for(slot=0;slot<2;slot++)if(p==&w->frames[k].ordering70[slot?702:0]){found=1;goto selected;}
selected:assert(found&&(uintptr_t)p>UINT32_MAX);fadeKinds[slot]++;const s32 args[5]={k,slot,b,(s32)c};record(1,args);effect(1);}
void SetMotionControllerVisible(s32 a){const s32 args[5]={a};record(2,args);effect(2);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){unsigned kind;
 if(p==(char*)&w->visual.exit34.halves[0]){kind=0;assert(b==2&&d==4);}
 else if(p==(char*)&w->settings.bytes[48]){kind=1;assert(b==-1&&d==17);}
 else if(p==(char*)&w->settings.bytes[49]){kind=2;assert(b==-1&&d==17);}
 else{assert(p==(char*)&w->settings.audio.mono32);kind=3;assert(b==2&&d==2);}
 assert((uintptr_t)p>UINT32_MAX);inputKinds[kind]++;const s32 args[5]={a,b,kind,d,e};record(3,args);effect(3);
 /* Execute a width-correct edit through the actual subobject passed by C.
  * Only the low half of the32-bit index changes; the upper half remains0. */
 if(kind==0){s16 value=(scenario+serial)&3;memcpy(p,&value,2);assert(w->visual.exit34.word==value);}
 else if(kind==3){s16 value=(scenario+serial)&1;memcpy(p,&value,2);}
 else *p=(char)(u8)(scenario+serial);
}
void PlaySfx(s32 a){const s32 args[5]={a};record(4,args);effect(4);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){const s32 args[5]={a,b,c};record(5,args);effect(5);}
void SeqSetVolume(s16 a){const s32 args[5]={a};record(6,args);effect(6);}
void SetCdMixMode(s32 a){const s32 args[5]={a};record(7,args);effect(7);}
void DrawUiRow(s32 a){const s32 args[5]={a};record(8,args);effect(8);}
void options_init_main_menu(void){service(9);}
void UpdateScreenFade(s32 a){const s32 args[5]={a};record(10,args);effect(10);}
static void expectedInput(unsigned kind,s32 mode,s32 width,s32 limit){unsigned char *p=kind==0?(unsigned char*)&w->visual.exit34.halves[0]:kind==3?(unsigned char*)&w->settings.audio.mono32:&w->settings.bytes[kind==1?48:49];
 const s32 args[5]={mode,width,kind,limit,0};inputKinds[kind]++;record(3,args);effect(3);
 if(kind==0)w->visual.exit34.word=(scenario+serial)&3;else if(kind==3)w->settings.audio.mono32=(scenario+serial)&1;else *p=(u8)(scenario+serial);
}
static void expected(void){s32 phase=w->visual.layout0C;states[phase>=0&&phase<10?phase:10]++;
 if(phase==0){service(0);w->timer=64;w->visual.layout0C=word((u32)w->visual.layout0C+1);
 }else if(phase==1){if(w->timer>0){s16 timer=half((u16)w->timer-1);w->timer=timer;AddScreenFadeTiles(&w->frames[w->frameRoot].ordering70[702],timer*4,69);fadeTaken[0]++;}
  SetMotionControllerVisible(1);if(w->rows[w->rowRoot][0].byte14==2&&w->timer==0)w->visual.layout0C=word((u32)w->visual.layout0C+1);
 }else if(phase==2){w->rows[w->rowRoot][w->visual.exit34.word].cells[0]=0;expectedInput(0,0,2,4);w->rows[w->rowRoot][w->visual.exit34.word].cells[0]=2;
  if(w->input[0].a){PlaySfx(1);w->rows[w->rowRoot][w->visual.exit34.word].cells[0]=1;w->visual.layout0C=w->visual.exit34.word+3;SetMenuFrameOptions(1,1,0);}
  else if(w->input[0].b){PlaySfx(2);w->timer=64;w->visual.layout0C=6;}
 }else if(phase==3||phase==4){unsigned row=phase+1;if(w->rows[w->rowRoot][row].byte14==2){w->rows[w->rowRoot][row].cells[0]=2;expectedInput(phase==3?1:2,1,-1,17);
   if(phase==3){w->music=w->settings.bytes[48];SeqSetVolume(w->gain);}else w->effects=w->settings.bytes[49];
   if(w->input[0].a){audioTaken[phase-3]++;PlaySfx(1);w->rows[w->rowRoot][row].cells[0]=1;w->visual.layout0C=7;SetMenuFrameOptions(1,1,1);}}
  DrawUiRow(row);
 }else if(phase==5){if(w->rows[w->rowRoot][6].byte14==2){w->rows[w->rowRoot][6+w->settings.audio.mono32].cells[0]=0;expectedInput(3,0,2,2);w->rows[w->rowRoot][6+w->settings.audio.mono32].cells[0]=2;
   if(w->input[0].a){audioTaken[2]++;PlaySfx(1);w->rows[w->rowRoot][6+w->settings.audio.mono32].cells[0]=1;w->visual.layout0C=7;SetMenuFrameOptions(1,1,1);SetCdMixMode(w->settings.audio.mono32);}}
  DrawUiRow(6);DrawUiRow(7);
 }else if(phase==6){for(unsigned i=0;i<4;i++)if(w->rows[w->rowRoot][i].byte14==2)w->rows[w->rowRoot][i].byte14=3;w->timer=64;w->visual.layout0C=9;
 }else if(phase==7){for(unsigned i=4;i<8;i++){u8 state=w->rows[w->rowRoot][i].byte14;if(state==2)w->rows[w->rowRoot][i].byte14=3;else if(state==6)w->visual.layout0C=8;
   if(w->rows[w->rowRoot][i].byte14>=2)DrawUiRow(i);}
 }else if(phase==8){for(unsigned i=4;i<8;i++)w->rows[w->rowRoot][i].byte14=0;w->visual.layout0C=1;
 }else if(phase==9){s16 timer=half((u16)w->timer-1);w->timer=timer;AddScreenFadeTiles(&w->frames[w->frameRoot].ordering70[0],(64-(s32)timer)*4,69);
  if(w->timer<=0){fadeTaken[1]++;service(9);w->visual.layout0C=0;w->visual.scene00=word((u32)w->visual.scene00-1);}
 }
 for(unsigned i=0;i<4;i++)DrawUiRow(i);UpdateScreenFade(0x1006A);
}
int main(void){assert(sizeof(void*)==8);u16 endian=1;assert(*(u8*)&endian==1);
 for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<22;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;w->visual.layout0C=profile<20?(s32)(profile/2):profile==20?(s32)scenario:word(scenario*0x9E3779B9U);
 w->visual.scene00=word(scenario*0x1234567U);w->visual.exit34.word=scenario&3;w->settings.audio.mono32=(scenario>>2)&1;w->settings.bytes[48]=(u8)scenario;w->settings.bytes[49]=(u8)(scenario>>8);
 w->timer=half(scenario);w->gain=half(scenario*13);w->rowRoot=scenario&1;w->frameRoot=(scenario>>1)&1;w->input[0].a=(scenario>>6)&1;w->input[0].b=(scenario>>7)&1;
 for(unsigned i=0;i<8;i++)for(unsigned j=0;j<2;j++)w->rows[j][i].byte14=(scenario/256+i+j)%9;
 reference=actual;serial=count=seen=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;options_update_audio_menu();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<11;i++)assert(coverage[i]);for(unsigned i=0;i<11;i++)assert(states[i]);for(unsigned i=0;i<4;i++)assert(inputKinds[i]);assert(fadeKinds[0]&&fadeKinds[1]&&fadeTaken[0]&&fadeTaken[1]);for(unsigned i=0;i<3;i++)assert(audioTaken[i]);
 puts("1441792 cases passed; all ten layouts, complete timer/layout sweeps, mixed-width index edits, both order heads, three audio paths and fresh UI/frame/settings state");
}
'''
with TemporaryDirectory() as d:
    c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
    subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
