"""Both record renderers with actual time formatter and mutable typed drawing services."""
from overlay_units import read_c_source
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay663RecordsTest(unittest.TestCase):
    def test_live_records_widths_and_captured_footer(self):
        root=Path(__file__).resolve().parents[2]
        bodies='\n'.join(read_c_source(root/f'src/overlays/PAL/records/func_{a}.c') for a in ['8011602C','801163B8'])
        formatter=read_c_source(root/'src/overlays/PAL/records/func_80115EE8.c')
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
#include "rr/overlay663_records.h"
typedef struct {u32 guard[4];RrTimeRecord16 banks[2][2][8][5];RrBadgeRow1C badges[2][8];RrUiUvRect glyphs[5],footer[2];u8 widths[2][4],text[10];s16 group,track;s32 tick,selected;unsigned root;u32 tail[4];} State;
typedef struct {s32 kind,args[9];unsigned cursor;u8 data[10];u32 hash;} Event;
typedef struct {State s;Event events[40];unsigned calls;void *result;} World;
static World actual,reference,initial,*w;
static u32 heads[704],arena[4096];static unsigned scenario,bank,mutation,seenTracks,seenClut;
#define RaceRecords w->s.banks[0]
#define LapRecords w->s.banks[1]
#define BadgeRows (w->s.badges[w->s.root])
#define CourseMenuSelection w->s.group
#define CourseIndex w->s.track
#define ScreenFrame w->s.tick
#define records_selected_record w->s.selected
#define RecordRankGlyphs w->s.glyphs
#define records_record_badges w->s.footer
#define records_race_column_widths w->s.widths[0]
#define records_lap_column_widths w->s.widths[1]
#define records_time_text w->s.text
static s32 word(u32 u){s32 v;memcpy(&v,&u,4);return v;}
static s16 half(u32 u){u16 bits=u;s16 v;memcpy(&v,&bits,2);return v;}
static void effect(unsigned n){unsigned b,g,t,r,j;static const u32 times[]={0,1,999,1000,59999,60000,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU};
 w->s.group=half(scenario+n*7919U);w->s.track=(scenario+n)%8;w->s.selected=(s32)((scenario+n)%7)-1;w->s.tick=word(scenario*0x9E3779B9U+n*0x71F1U);w->s.root=(scenario+n)%2;
 for(b=0;b<2;b++)for(g=0;g<2;g++)for(t=0;t<8;t++)for(r=0;r<5;r++){
  RrTimeRecord16 *v=&w->s.banks[b][g][t][r];for(j=0;j<9;j++)v->name[j]='A'+(scenario+n+b+g+t+r+j)%26;v->name[9]=0;
  v->car=half(scenario+n+b+t+r);v->time=word(times[(scenario+n+b+g+t+r)%9]);
 }
 for(b=0;b<2;b++)for(r=0;r<8;r++){
  for(j=0;j<sizeof(RrBadgeRow1C);j++)((u8*)&w->s.badges[b][r])[j]=(u8)(scenario+n*17+b*51+r*11+j*7);
  w->s.badges[b][r].x04=half(scenario*17U+n*7117U+b*3323+r*991);w->s.badges[b][r].y06=half(scenario*31U+n*3571U+b*1741+r*1337);
 }
 for(j=0;j<sizeof(w->s.glyphs);j++)((u8*)w->s.glyphs)[j]=(u8)(scenario+n+j*13);
 for(j=0;j<sizeof(w->s.footer);j++)((u8*)w->s.footer)[j]=(u8)(scenario+n+j*19);
 for(b=0;b<2;b++)for(j=0;j<4;j++)w->s.widths[b][j]=(u8)(scenario+n+b*37+j*17);
 memcpy(w->s.text,"00'00\"000",10);
}
static Event *event(s32 kind){unsigned i;u32 hash=2166136261U;Event *e;assert(w->calls<40);
 for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;
 e=w->events+w->calls++;memset(e,0,sizeof(*e));e->kind=kind;e->hash=hash;return e;
}
static void change(void){if(mutation)effect(w->calls);}
static void *render(Event *e,u32 *order,void *cursor){assert(order==heads);assert((uintptr_t)cursor>UINT32_MAX);assert((u8*)cursor>=(u8*)arena&&(u8*)cursor<(u8*)(arena+4096));e->cursor=(u8*)cursor-(u8*)arena;change();return (u8*)cursor+4*(w->calls%7+1);}
s32 rcos(s32 angle){Event *e=event(1);e->args[0]=angle;change();return (u32)angle&128?-4096:4096;}
void *AddSpritePrim(u32 *o,void *p,s32 x,s32 y,RrUiUvRect *glyph,u16 clut,s32 color){Event *e=event(2);unsigned i,id=99;
 for(i=0;i<5;i++)if(glyph==w->s.glyphs+i)id=i;for(i=0;i<2;i++)if(glyph==w->s.footer+i)id=5+i;assert(id<7);
 e->args[0]=x;e->args[1]=y;e->args[2]=id;e->args[3]=clut;e->args[4]=color;memcpy(e->data,glyph,4);if(id<5)seenClut|=1U<<(clut-32760);return render(e,o,p);}
void *AddTilePrim(u32 *o,void *p,s32 x,s32 y,s32 width,s32 height,s32 r,s32 g,s32 b){Event *e=event(3);s32 args[]={x,y,width,height,r,g,b};memcpy(e->args,args,sizeof(args));return render(e,o,p);}
void *AddTextSprites(u32 *o,void *p,s32 x,s32 y,const char *text,s32 clut,s32 spacing,s32 color){Event *e=event(4);unsigned b,g,t,r,valid=0;
 if(text==(char*)w->s.text){e->args[5]=-1;valid=1;}else for(b=0;b<2;b++)for(g=0;g<2;g++)for(t=0;t<8;t++)for(r=0;r<5;r++)if(text==w->s.banks[b][g][t][r].name){e->args[5]=b;e->args[6]=g;e->args[7]=t;e->args[8]=r;valid=1;}
 assert(valid);e->args[0]=x;e->args[1]=y;e->args[2]=clut;e->args[3]=spacing;e->args[4]=color;memcpy(e->data,text,10);return render(e,o,p);}
void *AddDrawModePrim(u32 *o,void *p,u16 mode){Event *e=event(5);e->args[0]=mode;return render(e,o,p);}
void *DrawCourseIconSprite(u32 *o,void *p,s32 x,s32 y,s32 icon){Event *e=event(6);e->args[0]=x;e->args[1]=y;e->args[2]=icon;return render(e,o,p);}
''' + formatter + bodies + r'''
static char *format(s32 value){int64_t v=value,minutes=v/60000,seconds=v/1000%60,millis=v%1000;u8 *s=w->s.text;
 s[0]=(u8)(minutes/10+48);s[1]=(u8)(minutes%10+48);s[3]=(u8)(seconds/10+48);s[4]=(u8)(seconds%10+48);s[6]=(u8)(millis/100+48);s[7]=(u8)(millis%100/10+48);s[8]=(u8)(millis%10+48);return (char*)s;
}
static void *expected(unsigned row,void *cursor){State *s=&w->s;unsigned g=s->group>=8,i;int x=0,y=s->badges[s->root][row].y06+40,rowY;
 seenTracks|=1U<<(g*8+s->track);
 for(i=0;i<5;i++){
  int clut=32760;if((s32)i==s->selected){int angle=word((u32)s->tick*128U-682U);if(rcos(angle)<0)clut=32761;}
  x=s->badges[s->root][row].x04+8;cursor=AddSpritePrim(heads,cursor,x,y,s->glyphs+i,clut,0);
  x+=s->widths[bank][0];cursor=AddTilePrim(heads,cursor,x,y-2,1,20,255,255,255);
  x+=s->widths[bank][1];cursor=AddTextSprites(heads,cursor,x,y,format(s->banks[bank][g][s->track][i].time),clut,0,0);
  x+=s->widths[bank][2];cursor=AddTilePrim(heads,cursor,x,y-2,1,20,255,255,255);
  x+=s->widths[bank][3];cursor=AddTextSprites(heads,cursor,x,y,s->banks[bank][g][s->track][i].name,clut,0,0);y+=24;
 }
 x+=52;rowY=s->badges[s->root][row].y06;y=rowY+40;
 cursor=AddTilePrim(heads,cursor,x,y-2,1,116,255,255,255);cursor=AddDrawModePrim(heads,cursor,56);
 x+=16;cursor=DrawCourseIconSprite(heads,cursor,x,y,(u16)s->group%8);
 cursor=AddSpritePrim(heads,cursor,x+8,rowY+112,s->footer+g,30180+g,0);return AddDrawModePrim(heads,cursor,89);
}
int main(void){unsigned i,row;
 for(scenario=0;scenario<4096;scenario++)for(bank=0;bank<2;bank++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;effect(0);row=(scenario/16)%8;for(i=0;i<4;i++){initial.s.guard[i]=scenario+i;initial.s.tail[i]=~(scenario+i);}
  actual=reference=initial;w=&actual;actual.result=bank?records_draw_lap_records(heads,arena,row):records_draw_race_records(heads,arena,row);
  w=&reference;reference.result=expected(row,arena);assert(!memcmp(&actual,&reference,sizeof(actual)));
 }
 assert(seenTracks==65535&&seenClut==3);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
