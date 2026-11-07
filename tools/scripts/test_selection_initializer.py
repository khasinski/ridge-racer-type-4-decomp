"""Guarded native oracle for the two-player overlay667 selection initializer."""
from pathlib import Path
from tempfile import TemporaryDirectory
import re,subprocess,sys,os,json,hashlib
root=next(p for p in Path(__file__).resolve().parents if (p/'tools/scripts/cc.sh').is_file())
stage=root/'tools/scripts'
source_path=Path(sys.argv[1]) if len(sys.argv)>1 else root/'src/overlays/PAL/serial_link/func_801161A8.c'
body=source_path.read_text();macro='\n'.join(l for l in body.splitlines() if l.startswith('#define KEEP_GLYPH'))
body=body[body.index('void serial_link_init_car_select'):];body=re.sub(r' __asm__\("\$\d+"\)','',body)
encoder=(root/'tools/scripts/fixtures/views/func_80037040.c').read_text().replace('void UnpackChoiceGlyph','static void encode_original')
source=r'''
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
#include "rr/selection_initializer.h"
_Static_assert(sizeof(RrRaceVehicle)==0x320,"vehicle owner");
_Static_assert(sizeof(RrSelectionRows)==32,"complete selection owner");
_Static_assert(sizeof(RrCourseChoice4)==4,"choice stride");
_Static_assert(sizeof(RrMenuGridPoint)==4,"point stride");
static u8 resources[3][0x30000];static u32 scratchOrder;
typedef struct {
 u32 guard[4];s32 frame,tick,player;s16 selection,limit,course,active;
 u8 *base;void *buffer,*published;RrMenuArenas arenas;
 RrRaceVehicle vehicles[2];RrProjectionScratchPrefix scratch;RrSelectionRows selectionRows;
 RrCourseChoice4 table[2][2][9];s16 counts[2][2],group[2],index[2];RrMenuGridPoint point[4];u32 tail[4];
} State;
static State actual,reference,*w;
#define ModeFrame w->frame
#define ScreenFrame w->tick
#define SelectionPlayer w->player
#define MenuCursor w->selection
#define MenuItemCount w->limit
#define CourseMenuSelection w->course
#define CourseMenuPhase w->active
#define CdDirectoryBuffer w->base
#define MenuArenaBase w->buffer
#define LoadCursor w->published
#define MenuArenas w->arenas
#define PlayerCars w->vehicles
#define SelectionRows w->selectionRows
#define CourseMenuChoices w->table
#define ChoiceGlyphCounts w->counts
#define CourseMenuPage w->group
#define CourseMenuCursor w->index
#define PlayerCourseChoices w->point
#undef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH (&w->scratch)
''' + macro + '\n' + body + '\n' + encoder + r'''
typedef struct {unsigned kind;s32 args[10];State before;} Event;
static Event events[40];
static unsigned scenario,profile,serial,count,seen,coverage[8],groupReset[2],resourceBranch[2],oldGroup[2],destChanged[2];
static int referenceRun;
static s32 word(u32 a){s32 b;memcpy(&b,&a,4);return b;}
static s16 half(u16 a){s16 b;memcpy(&b,&a,2);return b;}
static s8 byte(u8 a){s8 b;memcpy(&b,&a,1);return b;}
static void tableState(unsigned seed){for(unsigned g=0;g<2;g++)for(unsigned r=0;r<2;r++){
 unsigned length=(scenario+seed+g*2+r)%10;
 for(unsigned j=0;j<9;j++){s16 glyph=half(scenario*31+seed*137+j*97);if(glyph==-1)glyph=0;
 if(j==8)glyph=length==9?0x167:0;if(j==length)glyph=-1;
 w->table[g][r][j]=(RrCourseChoice4){glyph,byte(scenario+seed+j+g),byte(scenario/256+seed+j+r)};}
}}
static void record(unsigned kind,const s32 *args){assert(kind<8);
 if(referenceRun){assert(count<40);Event *e=&events[count++];e->kind=kind;memcpy(e->args,args,40);e->before=*w;}
 else{coverage[kind]++;assert(seen<count);Event *e=&events[seen++];assert(e->kind==kind&&!memcmp(args,e->args,40)&&!memcmp(w,&e->before,sizeof(*w)));}
}
static void effect(unsigned kind){unsigned n=serial++;if(!(profile&1))return;
 u32 v=(scenario+17*kind+31*n)*0x9E3779B9U;
 w->frame=word(v);w->tick=word(~v);w->player=(scenario+n)%4;w->course=half(v);w->selection=half(v>>7);w->limit=half(v>>16);w->active=half(v);
 w->base=resources[(scenario+n)%3];w->buffer=resources[(scenario+n+1)%3]+128;w->published=resources[(scenario+n+2)%3]+256;
 w->arenas.first=resources[(scenario+n)%3]+512;w->arenas.second=resources[(scenario+n+1)%3]+768;
 for(unsigned i=0;i<2;i++){w->group[i]=(scenario+n+i)&1;w->index[i]=(scenario/4+n+i)%9;
  for(unsigned j=0;j<2;j++)w->counts[i][j]=(scenario+n+i+j)%10;
  for(unsigned j=0;j<3;j++)w->vehicles[i].pos20[j]=word(v+i+j);w->vehicles[i].rot54=word(v+i);}
 w->scratch.word08=word(v);w->scratch.word0C=word(~v);w->scratch.word10=word(v>>1);w->scratch.word18=word(v+1);w->scratch.word1C=word(v+2);w->scratch.word20=word(v+3);
 tableState(n);
}
static void service(unsigned kind){const s32 args[10]={0};record(kind,args);effect(kind);}
void ResetScreenCounters(void){service(0);}
void InitMenuArenas(void){service(1);}
void InitUiRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g,s32 h,s32 i,s32 j,...){const s32 args[10]={a,b,c,d,e,f,g,h,i,j};record(2,args);effect(2);}
void SetupRenderer(s32 a){const s32 args[10]={a};record(3,args);effect(3);}
void InitPreviewImageRows(void){service(4);}
void ResetCarState(RrRaceVehicle *p){unsigned i;for(i=0;i<2&&p!=&w->vehicles[i];i++){}assert(i<2&&(uintptr_t)p>UINT32_MAX);const s32 args[10]={i};record(5,args);effect(5);}
s32 CountChoiceGlyphs(const RrCourseChoice4 *p){unsigned g,r;for(g=0;g<2;g++)for(r=0;r<2;r++)if(p==w->table[g][r])goto found;
found:assert(g<2&&r<2&&(uintptr_t)p>UINT32_MAX);const s32 args[10]={g,r};record(6,args);s32 count=0;for(;count<9;count++){if(p[count].glyph==-1)break;if(count==8&&p[8].glyph!=0x167)break;}effect(6);return count;}
void UnpackChoiceGlyph(s8 *p,s32 glyph){unsigned i;for(i=0;i<4&&p!=(s8*)&w->selectionRows+4*i;i++){}assert(i<4&&(uintptr_t)p>UINT32_MAX);const s32 args[10]={i,glyph};record(7,args);encode_original(p,glyph);effect(7);}
static void expected(s32 original){w->frame=0;w->tick=0;w->selection=0;w->limit=3;ResetScreenCounters();w->arenas.second=w->base+0x2D000;InitMenuArenas();w->player=original;w->published=w->arenas.second;
 static const s32 rows[24][10]={
 {0,24,88,68,32,0,0,1,0,83},{1,24,128,68,32,0,0,1,0,54},{2,24,168,68,32,0,0,1,0,85},
 {3,24,88,68,32,0,0,1,0,83},{4,24,128,68,32,0,0,1,0,101},{5,24,168,68,32,0,0,1,0,53},{6,24,208,68,32,0,0,1,0,85},{7,116,88,28,32,0,0,1,0,55},{8,116,128,28,32,0,0,1,0,56},
 {9,24,88,68,32,0,0,1,0,83},{10,24,128,68,32,0,0,1,0,54},{11,24,168,68,32,0,0,1,0,85},
 {12,24,88,68,32,0,0,1,0,83},{13,24,128,68,32,0,0,1,0,101},{14,24,168,68,32,0,0,1,0,53},{15,24,208,68,32,0,0,1,0,85},{16,116,88,28,32,0,0,1,0,55},{17,116,128,28,32,0,0,1,0,56},
 {18,12,64,68,32,0,0,1,0,84},{19,12,104,68,32,0,0,1,0,3},{20,12,144,68,32,0,0,1,0,85},{21,12,368,208,32,0,0,8,4,0},{23,104,104,68,32,0,0,1,5,3},{24,104,64,68,32,0,0,0,126,0}};
 for(unsigned i=0;i<24;i++){s32 args[10];memcpy(args,rows[i],40);if(args[0]==21)args[9]=w->course;record(2,args);effect(2);}
 SetupRenderer(5);w->selection=0;w->limit=3;ResetScreenCounters();InitPreviewImageRows();w->active=0;w->published=w->buffer;resourceBranch[w->player&1]++;
 if((w->player&1)==0)w->arenas.first=w->buffer;else w->published=w->arenas.first;
 w->scratch.word18=120;w->scratch.word08=-16;w->scratch.word0C=-248;w->scratch.word1C=0;w->scratch.word20=0;w->scratch.word10=-96;
 for(unsigned i=0;i<2;i++){ResetCarState(&w->vehicles[i]);w->vehicles[i].pos20[1]=10;w->vehicles[i].pos20[2]=308;w->vehicles[i].rot54=2048;}
 for(unsigned g=0;g<2;g++)for(unsigned r=0;r<2;r++)w->counts[g][r]=CountChoiceGlyphs(w->table[g][r]);
 unsigned old=original&1;oldGroup[old!=(w->player&1)]++;int reset=w->counts[old][1]==0;groupReset[reset]++;if(reset)w->group[old]=0;
 unsigned dest=w->player,g=dest&1;UnpackChoiceGlyph((s8*)&w->selectionRows+4*dest,w->table[g][w->group[g]][w->index[g]].glyph);
 destChanged[dest!=(unsigned)w->player]++;unsigned current=w->player;g=current&1;
 w->point[current].first=w->table[g][w->group[g]][w->index[g]].first;
 w->point[current].second=w->table[g][w->group[g]][w->index[g]].second;
}
int main(void){assert(sizeof(void*)==8);for(scenario=0;scenario<65536;scenario++)for(profile=0;profile<8;profile++){
 memset(&actual,0xA6,sizeof(actual));w=&actual;w->frame=word(scenario*0x1234567U);w->tick=word(~(scenario*0x2345678U));w->player=(scenario+1)%4;w->course=half(scenario);
 w->base=resources[scenario%3];w->buffer=resources[(scenario+1)%3]+128;w->published=resources[(scenario+2)%3]+256;w->arenas.first=resources[scenario%3]+512;w->arenas.second=resources[(scenario+1)%3]+768;
 w->scratch.pointers.cursor=resources[0];w->scratch.pointers.order=&scratchOrder;
 for(unsigned i=0;i<2;i++){w->group[i]=(scenario+i)&1;w->index[i]=(scenario/4+i)%9;for(unsigned j=0;j<2;j++)w->counts[i][j]=(scenario+i+j)%10;}tableState(0);
 reference=actual;serial=count=seen=0;referenceRun=1;w=&reference;expected(profile/2);serial=0;referenceRun=0;w=&actual;serial_link_init_car_select(profile/2);assert(count==seen&&!memcmp(&actual,&reference,sizeof(actual)));
 }
 for(unsigned i=0;i<8;i++)assert(coverage[i]);assert(groupReset[0]&&groupReset[1]&&resourceBranch[0]&&resourceBranch[1]&&oldGroup[0]&&oldGroup[1]&&destChanged[0]&&destChanged[1]);
 puts("524288 cases passed;24 UI rows,2 vehicles,4 groups,original-player reset,live-player publication,complete signed coordinate/glyph patterns and guarded pre-service state");
}
'''
with TemporaryDirectory() as d:
 c=Path(d)/'test.c';exe=Path(d)/'test';c.write_text(source)
 subprocess.run(['cc','-std=gnu11','-O2','-funsigned-char','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I'+str(stage),'-I'+str(root/'include'),str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)

if os.environ.get('RR_AUDIT_JSON'):
 deps=[source_path,Path(__file__),root/'include/rr/selection_initializer.h',root/'tools/scripts/fixtures/views/func_80037040.c']+sorted((root/'include/rr').glob('*.h'))
 a=dict(cases=524288,engine='independent expected state model/native C',service_kinds=8,ui_rows=24,full_guarded_state=True,mutating_callbacks=True,full_host_pointers=True,sanitizers=['undefined','bounds'],source_sha256=hashlib.sha256(source_path.read_bytes()).hexdigest(),dependencies={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in deps})
 Path(os.environ['RR_AUDIT_JSON']).write_text(json.dumps(a,indent=2)+'\n')
