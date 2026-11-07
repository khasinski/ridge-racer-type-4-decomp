"""Full save/map/palette initialization with mutable typed service boundaries."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class GaragePaletteInitializerTest(unittest.TestCase):
    def test_mapping_and_fresh_roots(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_80119B44.c').read_text()
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
#include "rr/garage_palette_initializer.h"
typedef struct {
 u32 guard[4];RrSaveProgressRecord7C save;RrCourseChoice4 courses[2][9];RrBadgeRow1C badges[2][2];
 RrMenuArenas arenas;void *published,*palette0,*palette1;
 s32 frame,pulse,local,mode;s16 flag,layout,current,count,selection,course,index,reset;
 u8 map[25];unsigned root;u32 tail[4];
} State;
typedef struct {s32 kind,args[10];u32 hash;} Event;
typedef struct {State s;Event events[5];unsigned calls;} World;
static World actual,reference,initial,*w;
static u8 arena[3][128],palettes[3][25*0x840];
static unsigned scenario,mutation,seenMap,seenFlags;
#define SaveProgress w->s.save
#define CourseChoiceTable w->s.courses
#define BadgeRows (w->s.badges[w->s.root])
#define MenuArenas w->s.arenas
#define LoadCursor w->s.published
#define DefaultCarClut w->s.palette0
#define CarClutData w->s.palette1
#define ModeFrame w->s.frame
#define ScreenFrame w->s.pulse
#define MenuCursor w->s.flag
#define MenuItemCount w->s.layout
#define garage_color_preview_state w->s.current
#define garage_select_arrows_unused_arg w->s.local
#define garage_returned_from_logo_editor w->s.mode
#define garage_clut_list_count w->s.count
#define garage_image_fade_phase w->s.reset
#define garage_clut_choice w->s.selection
#define garage_car_index w->s.course
#define garage_clut_list_index w->s.index
#define garage_car_color_map w->s.map
static s16 half(unsigned bits){u16 u=bits;s16 v;memcpy(&v,&u,2);return v;}
static void effect(unsigned n){unsigned i,g;State *s=&w->s;
 s->frame=(s32)(scenario+n*71);s->pulse=~s->frame;s->flag=half(scenario+n*51);s->layout=half(scenario+n*37);
 s->current=half(scenario+n*3);s->local=(s32)(scenario+n*11);s->mode=(s32)((scenario+n)%4)-1;
 s->count=half(scenario+n*5);s->reset=half(scenario+n*7);s->selection=half(scenario+n*9);s->course=(scenario+n)%9;s->index=half(scenario+n*13);
 s->arenas.first=arena[(scenario+n)%3];s->arenas.second=arena[(scenario+n+1)%3];s->published=arena[(scenario+n+2)%3];
 s->palette0=palettes[(scenario+n)%3];s->palette1=palettes[(scenario+n+1)%3];s->root=(scenario+n)%2;
 for(i=0;i<sizeof(s->save);i++)((u8*)&s->save)[i]=(u8)(scenario+n*13+i*31);
 for(i=0;i<4;i++)s->save.prefix.reserved50.bytes[8+i]=((scenario>>i)&1)?(u8)(1+(scenario+n+i)%255):0;
 for(g=0;g<2;g++)for(i=0;i<9;i++){s->courses[g][i].glyph=half(scenario+n+i*101);s->courses[g][i].first=(s8)(scenario+n+i);s->courses[g][i].second=(s8)((int)((scenario/4+n+i)%154)-128);}
 for(i=0;i<25;i++)s->map[i]=(u8)(scenario+n+i*17);
 for(i=0;i<sizeof(s->badges);i++)((u8*)s->badges)[i]=(u8)(scenario+n*19+i*23);
}
static Event *event(s32 kind){unsigned i;u32 h=2166136261U;Event *e;assert(w->calls<5);
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 e=w->events+w->calls++;memset(e,0,sizeof(*e));e->kind=kind;e->hash=h;return e;
}
static void change(void){if(mutation)effect(w->calls);}
void InitRect16Row(s32 a,s16 b,s16 c,s16 d,s16 e,s16 f,s16 g){Event *v=event(1);s32 args[]={a,b,c,d,e,f,g};memcpy(v->args,args,sizeof(args));change();}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){Event *v=event(2);s32 args[]={a,b,c,d,e,f,g,h,i,j};memcpy(v->args,args,sizeof(args));change();}
void InitBadgeRow(s32 a,s32 b,s16 c,s32 d,u16 e,u16 f,u16 g){Event *v=event(3);s32 args[]={a,b,c,d,e,f,g};memcpy(v->args,args,sizeof(args));change();}
void UploadCarClut(void *bank,s32 selected,s32 destination){Event *v=event(4);unsigned i,flags=0;
 assert((uintptr_t)bank>UINT32_MAX);for(i=0;i<3&&bank!=palettes[i];i++){}assert(i<3);assert(selected>=0&&selected<25);
 v->args[0]=i;v->args[1]=selected;v->args[2]=destination;
 assert(w->s.selection>=0&&w->s.selection<=25);if(w==&actual){seenMap|=1U<<w->s.selection;for(i=0;i<4;i++)if(w->s.save.prefix.reserved50.bytes[8+i])flags|=1U<<i;seenFlags|=1U<<flags;}
 change();
}
''' + body + r'''
static void expected(void){State *s=&w->s;unsigned i;int position;
 s->frame=0;s->pulse=0;s->current=0;s->local=0;s->flag=0;s->published=s->arenas.second;s->layout=s->mode?1:2;
 InitRect16Row(0,128,128,64,128,0,0);InitUiRow(0,12,64,68,32,0,0,1,0,83);InitUiRow(1,12,104,68,32,0,0,1,0,85);InitBadgeRow(0,108,320,112,34,0,1537);
 s->count=25;s->badges[s->root][0].half02=1;
 for(i=0;i<21;i++)s->map[i]=i;
 for(i=21;i<25;i++)s->map[i]=((u8*)&s->save)[67+i]?i:25;
 s->reset=0;position=s->mode==1?0:s->courses[1][s->course].second;
 s->selection=position;s->index=position>0?position-1:0;
 s->selection=(s8)s->map[s->index];
 UploadCarClut(s->selection?s->palette1:s->palette0,s->selection?s->selection-1:0,0);s->current=0;
}
int main(void){unsigned i;
 for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(0);for(i=0;i<4;i++){initial.s.guard[i]=scenario+i;initial.s.tail[i]=~(scenario+i);}
  actual=reference=initial;w=&actual;garage_init_color_select();w=&reference;expected();assert(!memcmp(&actual,&reference,sizeof(actual)));assert(actual.calls==5);
 }
 assert(seenMap==0x3FFFFFF);assert(seenFlags==0xFFFF);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
