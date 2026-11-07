"""Scene/selection dispatch with complete visual state and simulated service callbacks."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class OptionsDispatchTest(unittest.TestCase):
    def test_scene_capture_and_modular_increment(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/options/func_80118EB8.c').read_text()
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/options_dispatch.h"
typedef struct {u32 guard[4];RrOptionsVisualState visual;u16 scene;s16 next,phase;u16 pad;u32 tail[4];} State;
typedef struct {s32 kind,arg;State before;} Event;
typedef struct {State s;Event events[8];unsigned calls;} World;
static World actual,reference,initial,*w;
static unsigned scenario,mutation,seenScenes,seenSelections;
static const s32 scenes[]={0,1,2,-1,3,INT32_MIN,INT32_MAX};
static const s32 selections[]={0,1,2,3,4,5,6,-1,7,8,INT32_MIN,INT32_MAX};
#define options_menu_state w->s.visual
#define ModeGroup w->s.scene
#define ModeId w->s.next
#define ModeSubState w->s.phase
static s32 word(u32 bits){s32 value;memcpy(&value,&bits,4);return value;}
static void event(s32 kind,s32 arg){unsigned i,n=w->calls;assert(n<8);w->events[n].kind=kind;w->events[n].arg=arg;w->events[n].before=w->s;w->calls++;
 if(mutation){for(i=0;i<sizeof(w->s.visual);i++)((u8*)&w->s.visual)[i]=(u8)(scenario+n*17+i*31);
 w->s.visual.scene00=word(scenario&1?0x7FFFFFFFU:0xFFFFFFFFU);w->s.visual.selection08=selections[(scenario+n)%12];
 w->s.scene=(u16)(scenario+n*51);w->s.next=(s16)(scenario+n*37);w->s.phase=(s16)(scenario+n*11);}
}
void SetDispMask(s32 a){event(1,a);}void options_init_main_menu(void){event(2,0);}
void InitMotionController(void){event(3,0);}void SetMotionControllerVisible(s32 a){event(4,a);}void SeqStartTrack(s32 a){event(5,a);}
void options_update_main_menu(void){event(6,0);}void options_update_controller_config(s32 a){event(7,a);}
void options_update_audio_menu(void){event(8,0);}void options_update_screen_adjust(void){event(9,0);}
void options_update_brightness_test(void){event(10,0);}void StartAttractDemo(s32 a){event(11,a);}void StepCameraScript(void){event(12,0);}
''' + body + r'''
static void expected(void){s32 scene=w->s.visual.scene00;
 if(scene==0){SetDispMask(0);options_init_main_menu();w->s.visual.scene00=word((u32)w->s.visual.scene00+1U);InitMotionController();SetMotionControllerVisible(1);SeqStartTrack(0);}
 else if(scene==1)options_update_main_menu();
 else if(scene==2){s32 selection=w->s.visual.selection08;
  if(selection==0||selection==1)options_update_controller_config(selection);
  else if(selection==2)options_update_audio_menu();else if(selection==3)options_update_screen_adjust();
  else if(selection==4)options_update_brightness_test();else if(selection==5)StartAttractDemo(1);
  else if(selection==6){w->s.scene=1;w->s.next=3;w->s.phase=0;}
 }
 StepCameraScript();
}
int main(void){unsigned i;
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));for(i=0;i<sizeof(initial.s);i++)((u8*)&initial.s)[i]=(u8)(scenario+i*31);
  initial.s.visual.scene00=scenes[scenario%7];initial.s.visual.selection08=selections[(scenario/7)%12];
  actual=reference=initial;w=&actual;options_dispatch_menu();w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));
  seenScenes|=1U<<(scenario%7);if(initial.s.visual.scene00==2)seenSelections|=1U<<((scenario/7)%12);
 }
 assert(seenScenes==127&&seenSelections==4095);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
