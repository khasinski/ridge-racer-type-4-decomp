from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import sys
root=next(parent for parent in Path(__file__).resolve().parents if (parent/'Makefile').exists() and (parent/'tools/scripts').exists())
body_path=Path(sys.argv[1]) if len(sys.argv)>1 else root/'src/overlays/PAL/663/func_801151B8.c'
body=read_c_source(body_path);body=body[body.index('void ovl663_update_selection'):]
mapper=read_c_source(root/'tools/scripts/fixtures/views/func_80032E9C.c').replace('DecodeCourseIndex','actual_mapper');decode=read_c_source(root/'tools/scripts/fixtures/views/func_80037040.c').replace('UnpackChoiceGlyph','actual_decode')
src=r'''
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "rr/layout.h"
#include "rr/frame_storage_types.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/menu_transition.h"
#include "rr/current_selection_types.h"
#include "rr/overlay_menu.h"
#include "rr/menu_selection.h"
#include "rr/menu_glyph.h"
#include "rr/icon_sprites.h"
#include "rr/time_record_types.h"
#include "rr/course_choice_types.h"
#include "rr/tables.h"
_Static_assert(sizeof(RrUiRow24)==36,"UI stride");
_Static_assert(sizeof(RrBadgeRow1C)==28,"badge stride");
_Static_assert(sizeof(RrMenuItemState5C)==92,"input stride");
_Static_assert(sizeof(RrTimeRecord16)==16,"record stride");
_Static_assert(sizeof(RrCourseChoice4)==4,"course stride");
_Static_assert(sizeof(RrMenuChoice4)==4,"choice size");
_Static_assert(offsetof(RrTimeRecord16,car)==10,"packed car offset");
_Static_assert(offsetof(RrUiRow24,cells)==12,"cell offset");
void AddScreenFadeTiles(u32 *,s32,u32);void UpdateScreenFade(s32);void DrawUiRow(s32);void UpdateBadgeRowAnim(s32);
s32 LoadCarRangeStep(s32,s32);void ovl663_init_menu(void);void ovl663_init_result_preview(void);
static struct State {u32 guard[4];s32 frame,tick,selected,limit,mode; s16 scene,state,timer,course,menu,track,style;u16 enable;
 RrMenuItemState5C input[2];RrUiRow24 rows[2][4];RrBadgeRow1C badges[2][2];RrFrameStorage contexts[2];
 RrCurrentSelection choice;RrMenuGridPoint grid[4];RrCourseChoice4 courses[2][9];RrTimeRecord16 banks[2][2][8][5];
 u8 mapping[8];unsigned root,badgeRoot,contextRoot;void *published,*buffer;u32 tail[4];} actual,reference,*w;
static const u32 counterBits[7]={0,1,0x7FFFFFFFU,0x80000000U,0xFFFFFFFEU,0xFFFFFFFFU,0xA6A6A6A6U};
static u8 buffers[3][32];static const s32 limits[3]={4,8,16};static const s32 modes[6]={-1,0,1,2,0x10001,(s32)0x80000001U};
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define ModeSubState w->state
#define ModeId w->scene
#define ModeGroup w->enable
#define MenuFadeTimer w->timer
#define CourseMenuSelection w->course
#define MenuCursor w->menu
#define CourseIndex w->track
#define ReverseCourse w->style
#define PlayerControllers w->input
#define UiRows (w->rows[w->root])
#define BadgeRows (w->badges[w->badgeRoot])
#define CurrentDrawBuf (&w->contexts[w->contextRoot])
#define SelectionRows w->choice
#define PlayerCourseChoices w->grid
#define CourseChoiceTable w->courses
#define RaceRecords w->banks[0]
#define LapRecords w->banks[1]
#define LoadCursor w->published
#define MenuArenaBase w->buffer
#define CourseIndexMap w->mapping
#define ovl663_selected_record w->selected
#define ovl663_course_count w->limit
#define ovl663_record_kind w->mode
'''+body+mapper+decode+r'''
static struct Event{unsigned kind;s32 a[7];struct State before;}events[20];
static unsigned scenario,profile,serial,count,seen,repliesSeen[8],coverage[14],actions[5],special[3],fadeOut[2];static int referenceRun;
static s8 byte(u8 v){s8 r;memcpy(&r,&v,1);return r;}
static s16 half(u16 v){s16 r;memcpy(&r,&v,2);return r;}static s32 word(u32 v){s32 r;memcpy(&r,&v,4);return r;}
static void record(unsigned k,s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){s32 args[]={a,b,c,d,e,f,g};assert(k<14);
 if(referenceRun){assert(count<20);struct Event *v=&events[count++];v->kind=k;memcpy(v->a,args,sizeof(args));v->before=*w;}
 else{coverage[k]++;assert(seen<count);struct Event *v=&events[seen++];assert(v->kind==k&&!memcmp(v->a,args,sizeof(args))&&!memcmp(w,&v->before,sizeof(*w)));}
}
static void effect(unsigned k){unsigned n=serial++;if(!(profile&1))return;u32 v=(scenario+13*n+17*k)*0x9E3779B9U;
 w->frame=word((u32)w->frame+v);w->tick=word((u32)w->tick-v);w->root=(scenario+n)&1;w->badgeRoot=(scenario/2+n)&1;w->contextRoot=(scenario/4+n)&1;
 w->state=half(v);w->course=(scenario+n)%16;w->track=(scenario/16+n)%8;w->menu=(scenario/32+n)%2;w->style=half(v);w->selected=(scenario/64+n)%5;w->limit=limits[(scenario/128+n)%3];w->mode=modes[(scenario/256+n)%6];
 w->timer=half(scenario+31*n);w->buffer=buffers[(scenario+n)%3];w->published=buffers[(scenario+n+1)%3];
 w->input[0].a=(scenario>>9)&1;w->input[0].b=(scenario>>10)&1;
 for(unsigned j=0;j<18;j++){w->courses[j/9][j%9].first=byte(scenario+n*71+j*17);w->courses[j/9][j%9].second=byte(~(scenario+n*31+j));}
 for(unsigned j=0;j<2;j++){w->badges[j][1].state10=((scenario>>11)&1)?3:4;w->rows[j][0].byte14=((scenario>>12)&1)?5:2;w->rows[j][2].byte02=(scenario+n)%9;}
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){record(0,a,b,c,0,0,0,0);effect(0);}
void StepMenuValue(s32 a,s32 b,char *p,s32 d,s32 e){unsigned target;if(p==(char*)&w->course)target=0;else if(p==(char*)&w->menu)target=1;else{assert(p==(char*)&w->selected);target=2;}
 assert((uintptr_t)p>UINT32_MAX);record(1,a,b,target,d,e,0,0);effect(1);if(target==0)w->course=(scenario+serial)%16;else if(target==1)w->menu=(scenario+serial)%2;else w->selected=(scenario+serial)%5;
}
void DrawMenuStepArrows(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g){record(2,a,b,c,d,e,f,g);effect(2);}
s32 DecodeCourseIndex(s32 a){record(3,a,0,0,0,0,0,0);if(referenceRun){w->style=a>=8;return w->mapping[a&7];}return actual_mapper(a);}
void PlaySfx(s32 a){record(4,a,0,0,0,0,0,0);effect(4);}
void ResetScreenCounters(void){record(5,0,0,0,0,0,0,0);effect(5);}
void UnpackChoiceGlyph(s8 *p,s32 a){assert(p==(s8*)&w->choice&&(uintptr_t)p>UINT32_MAX);record(6,a,0,0,0,0,0,0);
 if(referenceRun){w->choice.choice.bytes[0]=a/160;w->choice.choice.bytes[1]=(a%160)/40;w->choice.choice.bytes[2]=a%40;}else actual_decode(p,a);
}
s32 LoadCarRangeStep(s32 a,s32 b){record(7,a,b,0,0,0,0,0);effect(7);static const u32 replies[]={0,1,2,0xFFFFFFFFU,0x10001,0xFFFF0001,0x80000000U,0x80000001U};if(!referenceRun)repliesSeen[scenario&7]++;return word(replies[scenario&7]);}
void ovl663_init_result_preview(void){record(8,0,0,0,0,0,0,0);effect(8);}
void ovl663_init_menu(void){record(9,0,0,0,0,0,0,0);effect(9);}
void AddScreenFadeTiles(u32 *p,s32 a,u32 b){assert(p==&w->contexts[w->contextRoot].ordering.ordering70&&(uintptr_t)p>UINT32_MAX);record(10,w->contextRoot,a,b,0,0,0,0);effect(10);}
void DrawUiRow(s32 a){record(11,a,0,0,0,0,0,0);effect(11);}
void UpdateBadgeRowAnim(s32 a){record(12,a,0,0,0,0,0,0);effect(12);}
void UpdateScreenFade(s32 a){record(13,a,0,0,0,0,0,0);effect(13);}
static void closeRows(void){for(unsigned i=0;i<3;i++)w->rows[w->root][i].byte14=3;w->badges[w->badgeRoot][1].state10=4;}
static void expected(void){unsigned phase=(u16)w->state;w->frame=word((u32)w->frame+1);w->tick=word((u32)w->tick+1);
 if(phase==0){if(w->badges[w->badgeRoot][1].state10==3){w->state=1;SetMenuFrameOptions(1,1,1);}}
 else if(phase==1){DrawMenuStepArrows(160,264,118,0,w->course,0,w->limit);StepMenuValue(1,2,(char*)&w->course,w->limit,0);w->track=DecodeCourseIndex(w->course);
  for(int i=0;i<w->rows[w->root][2].byte02;i++){
   if(i==((u16)w->course&7)){w->rows[w->root][2].cells[i]=(u8)w->style+2;w->rows[w->root][2].byte16=i;}
   else w->rows[w->root][2].cells[i]=i>=w->limit?1:(u8)w->style;
  }
  w->rows[w->root][w->menu].cells[0]=0;StepMenuValue(0,2,(char*)&w->menu,2,0);w->rows[w->root][w->menu].cells[0]=2;w->selected=-1;int cancel=0;
  if(w->input[0].a){PlaySfx(1);w->rows[w->root][w->menu].cells[0]=1;if(w->menu==0){actions[0]++;w->selected=0;w->state=2;}else if(w->menu==1){actions[1]++;cancel=1;}}
  else if(w->input[0].b){actions[2]++;PlaySfx(2);cancel=1;}
  if(cancel){SetMenuFrameOptions(0,1,1);w->timer=64;w->state=8;closeRows();}
 }else if(phase==2){StepMenuValue(0,4,(char*)&w->selected,5,0);
  if(w->input[0].b){actions[3]++;PlaySfx(2);w->state=1;}
  else if(w->input[0].a){actions[4]++;SetMenuFrameOptions(0,1,1);w->state=3;ResetScreenCounters();w->published=w->buffer;
   unsigned group=w->course>=8,bank=w->mode==1;int glyph=w->banks[bank][group][w->track][w->selected].car;UnpackChoiceGlyph((s8*)&w->choice,glyph);
   if(w->choice.choice.bytes[2]==39){unsigned key=w->choice.choice.bytes[0]==2?0:w->choice.choice.bytes[0]*4+w->choice.choice.bytes[1];special[w->choice.choice.bytes[0]==2?1:0]++;w->grid[0].second=0;w->grid[0].first=w->courses[0][key].first;}
   else{special[2]++;w->grid[0].first=w->grid[0].second=0;}closeRows();
  }
 }else if(phase==3){int result=LoadCarRangeStep(0,1);if(result==1&&w->rows[w->root][0].byte14==5){ovl663_init_result_preview();w->enable=result;w->scene=77;w->state=0;}}
 else if(phase==8){s16 timer=half((u16)w->timer-1);w->timer=timer;AddScreenFadeTiles(&w->contexts[w->contextRoot].ordering.ordering70,(64-(s32)timer)*4,69);
  if(w->timer<=0){fadeOut[0]++;ovl663_init_menu();w->enable=1;w->scene=74;w->state=0;return;}fadeOut[1]++;
 }
 for(int i=0;i<3;i++)DrawUiRow(i);UpdateBadgeRowAnim(1);UpdateScreenFade(w->mode==1?0x1005F:0x10063);
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<12;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;w->frame=word(counterBits[scenario%7]);w->tick=word(counterBits[(scenario/7)%7]);w->state=profile<2?0:profile<4?1:profile<6?2:profile<8?3:profile<10?8:half(scenario);
 w->root=scenario&1;w->badgeRoot=(scenario>>1)&1;w->contextRoot=(scenario>>2)&1;w->course=scenario%16;w->track=scenario%8;w->menu=(scenario>>4)&1;w->style=half(scenario);w->selected=scenario%5;w->limit=limits[(scenario>>5)%3];w->mode=modes[(scenario>>7)%6];w->timer=half(scenario);w->buffer=buffers[scenario%3];w->published=buffers[(scenario+1)%3];
 w->input[0].a=(scenario>>9)&1;w->input[0].b=(scenario>>10)&1;for(unsigned i=0;i<8;i++)w->mapping[i]=(scenario+i)%8;
 for(unsigned j=0;j<2;j++){w->badges[j][1].state10=((scenario>>11)&1)?3:4;w->rows[j][0].byte14=((scenario>>12)&1)?5:2;w->rows[j][2].byte02=scenario%9;}
 for(unsigned j=0;j<18;j++){w->courses[j/9][j%9].first=byte(scenario+j*17);w->courses[j/9][j%9].second=byte(~(scenario+j));}
 for(unsigned b=0;b<2;b++)for(unsigned g=0;g<2;g++)for(unsigned t=0;t<8;t++)for(unsigned i=0;i<5;i++)w->banks[b][g][t][i].car=(scenario+b*79+g*39+t*40+i*80)%480;
 reference=actual;count=seen=serial=0;referenceRun=1;w=&reference;expected();serial=0;referenceRun=0;w=&actual;ovl663_update_selection();assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<14;i++)assert(coverage[i]);for(unsigned i=0;i<8;i++)assert(repliesSeen[i]);for(unsigned i=0;i<5;i++)assert(actions[i]);for(unsigned i=0;i<3;i++)assert(special[i]);assert(fadeOut[0]&&fadeOut[1]);puts("786432 cases passed; guarded states,canonical2x9 owner,actual mapper/decoder,all record actions,signed16 fade encodings and eight full-word readiness replies");
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';c.write_text(src);exe=Path(d)/'test'
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
