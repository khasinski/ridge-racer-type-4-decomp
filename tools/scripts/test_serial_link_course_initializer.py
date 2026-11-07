from pathlib import Path
import re,subprocess,sys
root=Path(__file__).resolve().parents[2]
import os
os.chdir(root)
p=Path('build/goal_50/parallel667_init');source=Path(sys.argv[1]) if len(sys.argv)>1 else Path('src/overlays/PAL/serial_link/func_80116918.c')
if not source.exists(): source=p/'public_ready.c'
s=source.read_text();body=s[s.index('typedef struct'):];body=re.sub(r' __asm__\("\$\d+"\)','',body)
fields={'PlayerWinCounts':'h0','CourseMenuCursor':'h1','CourseMenuPage':'h2','SelectionRows':'choices','CourseMenuChoices':'records','CourseChoiceTable':'courses','PlayerNames':'names','LinkEntryState':'pairs','NumLaps':'players','SelectionPlayer':'base','AvailableCourseCount':'count','CourseIndex':'course','CourseMenuSelection':'menu','LinkRoleSlot':'flag','ModeId':'scene','CourseMenuPhase':'small','LinkCourseMenuEntryMode':'pairflag','ModeGroup':'enabled','ModeSubState':'phase'}
head='''#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
typedef int8_t s8;typedef uint8_t u8;typedef int16_t s16;typedef uint16_t u16;typedef int32_t s32;typedef uint32_t u32;
#include "rr/course_choice_types.h"
#include "rr/menu_choice_types.h"
#include "rr/selection_types.h"
typedef struct {u32 guard[4];s16 h0[4],h1[2],h2[2];RrSelectionRows choices;RrCourseChoice4 records[2][2][9];RrCourseChoice4 courses[2][9];char names[4][12];s8 pairs[2][2];s32 players,base,count;s16 course,menu,scene,small,pairflag;u16 flag,enabled,phase;u32 tail[4];} State;
static State a,b,*w,trace[16];static int args[16][4],step,checking;static u32 rng;
static char str[4][12]={"ABC","D","EFGHI","JK"};static char *serial_link_default_player_names[4];
static u32 next(void){rng=rng*1664525u+1013904223u;return rng;}
static void hit(int tag,int x,int y,int z){if(checking){assert(!memcmp(w,&trace[step],sizeof(*w)));assert(args[step][0]==tag&&args[step][1]==x&&args[step][2]==y&&args[step][3]==z);}else{trace[step]=*w;args[step][0]=tag;args[step][1]=x;args[step][2]=y;args[step][3]=z;}step++;w->base=(next()&1)*2;w->flag=next()&1;w->courses[0][0].glyph=(s16)next();for(int i=0;i<4;i++)serial_link_default_player_names[i]=str[next()%4];}
void SetDispMask(s32 x){hit(1,x,0,0);}void SetupDrawDispEnvs(s32 x,s32 y,s16 z,u32 q){assert(q==2);hit(2,x,y,z);}s32 UpdateLinkHandshake(s32 x){hit(3,x,0,0);return 19;}void ClearScreenOverlayState(void){hit(4,0,0,0);}
'''
copy=Path('tools/scripts/fixtures/views/func_8002B75C.c').read_text().replace('CopyStringClearTail','actual_copy');decode=Path('tools/scripts/fixtures/views/func_80037040.c').read_text().replace('UnpackChoiceGlyph','actual_decode')
helpers='''void CopyStringClearTail(char*d,char*s){int di=d-(char*)w,si=0;while(str[si]!=s)si++;hit(5,di,si,0);actual_copy(d,s);}void serial_link_init_car_select(s32 x){hit(6,x,0,0);}void UnpackChoiceGlyph(s8*d,s32 x){hit(7,(u8*)d-(u8*)w,x,0);actual_decode(d,x);}void SeqStartTrack(s32 x){hit(8,x,0,0);}void SetMenuFrameOptions(s32 x,s32 y,s32 z){hit(9,x,y,z);}
'''
macros=''.join('#define '+k+' w->'+v+'\n' for k,v in fields.items())
oracle='''static void oracle(void){SetDispMask(0);SetupDrawDispEnvs(0,0,0,2);UpdateLinkHandshake(1);ClearScreenOverlayState();for(int i=0;i<2;i++)w->h0[i]=w->h1[i]=w->h2[i]=0;for(int i=0;i<4;i++){w->h0[i]=0;((u8*)&w->choices)[i*4+3]=0;}w->course=w->menu=0;w->players=3;w->base=2*(1-w->flag);for(int i=0;i<18;i++){memcpy(((u8*)&w->records)+i*4,&w->courses[i/9][i%9],4);memcpy(((u8*)&w->records)+(18+i)*4,&w->courses[0][i%9],4);}for(int i=0;i<2;i++)CopyStringClearTail(w->names[w->base+i],serial_link_default_player_names[w->base+i]);serial_link_init_car_select(2*(1-w->flag));UnpackChoiceGlyph((s8*)&w->choices+w->base*4,w->courses[0][0].glyph);UnpackChoiceGlyph((s8*)&w->choices+(w->base+1)*4,w->courses[0][0].glyph);w->count=8;SeqStartTrack(0);w->enabled=1;w->scene=60;w->phase=0;w->small=0;w->pairflag=w->pairs[w->base/2][1]==1?2:0;SetMenuFrameOptions(0,1,1);}
int main(void){for(u32 n=0;n<131072;n++){rng=n+1;for(unsigned i=0;i<sizeof(a);i++)((u8*)&a)[i]=next()>>24;a.flag=n&1;a.base=(n&2);for(int i=0;i<4;i++){a.names[i][11]=0;serial_link_default_player_names[i]=str[i];}b=a;w=&a;step=0;checking=0;rng=n+29;serial_link_init_scene();int count=step;for(int i=0;i<4;i++)serial_link_default_player_names[i]=str[i];w=&b;step=0;checking=1;rng=n+29;oracle();assert(step==count);assert(!memcmp(&a,&b,sizeof(a)));}puts("131072 cases passed");}
'''
(p/'native.c').write_text(head+copy+decode+helpers+macros+body+oracle)
subprocess.run(['cc','-O2','-Wall','-Wextra','-Iinclude',str(p/'native.c'),'-o',str(p/'native')],check=True)
subprocess.run([str(p/'native')],check=True)
