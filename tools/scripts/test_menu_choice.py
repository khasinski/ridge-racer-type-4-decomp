"""Menu choice: mutable helpers, repeated byte records and native buffer publication."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT=Path(__file__).resolve().parents[2]


class MenuChoiceTest(unittest.TestCase):
    def test_actual_body_and_complete_mutable_owners(self):
        s=(ROOT/'tools/scripts/fixtures/views/func_80036D40.c').read_text()
        s=re.sub(r'^#include.*\n','',s,flags=re.M)
        prefix=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_OFFSET
#undef RR_ASSERT_SIZE
#define RR_ASSERT_OFFSET(t,f,o)
#define RR_ASSERT_SIZE(t,s)
#include "rr/menu_choice.h"
_Static_assert(sizeof(RrUiRow24)==36,"UI row");
_Static_assert(offsetof(RrUiRow24,cells)==12,"cells");
_Static_assert(offsetof(RrUiRow24,byte14)==20,"accept byte");
_Static_assert(offsetof(RrUiRow24,byte16)==22,"selection byte");
_Static_assert(sizeof(RrMenuGlyphRow1C)==28,"glyph stride");
_Static_assert(offsetof(RrMenuGlyphRow1C,flags00)==0,"flags");
_Static_assert(offsetof(RrMenuGlyphRow1C,control02)==2,"control");
_Static_assert(offsetof(RrMenuGlyphRow1C,phase18)==24,"phase");
_Static_assert(sizeof(unsigned long)>=sizeof(void*),"full pointer arithmetic");
_Static_assert(sizeof(RrMenuChoice4)==4,"packed choice");
_Static_assert(_Alignof(RrMenuChoice4)==1,"byte alignment");
_Static_assert(sizeof(RrMenuSettings2C)==44,"settings owner");
_Static_assert(offsetof(RrMenuSettings2C,mode1A)==26,"mode");
_Static_assert(offsetof(RrMenuSettings2C,copies)==28,"records");
_Static_assert(sizeof(RrBadgeRow1C)==28,"control row");
_Static_assert(offsetof(RrBadgeRow1C,state10)==16,"state");
typedef union {uint64_t align;u8 bytes[288];} UiOwner;
typedef struct {RrMenuChoice4 choice;RrMenuSettings2C settings;RrBadgeRow1C rows[2];UiOwner ui[2];RrMenuGlyphRow1C slots[4];u8 highlight[2][8];s16 selected;u16 buttons;s32 event,accepted;int rowbank,uibank,hintbank;void *resource,*current;u8 canary[24];} World;
typedef struct {int kind;s32 args[3];World snapshot;} Entry;
static World actual,expected,*w;
static Entry entries[8],actual_entries[8];
static int count,mutation;
static unsigned seed;
static uint64_t arena[32][8];
#define D_800F4D99 (w->choice.bytes[1])
#define SelectionRows (w->choice)
#define CourseListIndex (w->selected)
#define D_800F3A42 (w->buttons)
#define MenuStepDirection (w->event)
#define MenuConfirmPressed (w->accepted)
#define SelectionState (w->settings)
#define BadgeRows (&w->rows[w->rowbank])
#define UiRows ((RrUiRow24 *)w->ui[w->uibank].bytes)
#define GlyphRows (w->slots)
#define Dialogue (w->highlight[w->hintbank])
#define LoadCursor (w->current)
#define MenuArenaBase (w->resource)
'''
        reference=r'''
static void emit(int kind,s32 a,s32 b,s32 c){
 assert(count<8);Entry *e=&entries[count++];memset(e,0,sizeof(*e));e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->snapshot=*w;
 if(!mutation)return;
 if(kind==0){
  w->selected=(s16)(u16)(seed*109U);w->rowbank^=1;w->uibank^=1;w->hintbank^=1;
  w->accepted=(seed&8)?0:1;
  w->rows[w->rowbank].state10=(seed&4)?3:2;
  w->buttons^=65535;
 }
 if(kind==1){
  w->rowbank^=1;w->uibank^=1;
  for(int i=0;i<4;i++)w->choice.bytes[i]=(u8)((seed>>(i*8))^0xA5);
  w->rows[w->rowbank].state10=-1;
 }
 if(kind==2)w->resource=arena[(seed+3)%32];
 if(kind==3)w->resource=arena[(seed+7)%32];
}
void PlaySfx(s32 mode){assert(mode==0||mode==1);emit(mode,mode,0,0);}
void func_80039920(void){emit(2,0,0,0);}
void ResetScreenCounters(void){emit(3,0,0,0);}
void SetMenuFrameOptions(s32 a,s32 b,s32 c){emit(4,a,b,c);}
static void put16(u8 *p,u16 v){memcpy(p,&v,2);}
static void reference(void){
 int previous=w->choice.bytes[1];w->selected=previous;
 w->ui[w->uibank].bytes[12+previous]=0;
 int changed=0;
 if((w->buttons&0x2000) && w->selected<3){w->selected++;w->event=2;changed=1;}
 else if((w->buttons&0x8000) && w->selected>0){w->selected--;w->event=1;changed=1;}
 else w->event=0;
 if(changed){
  emit(0,0,0,0);
  if(w->selected<0)w->selected=0;else if(w->selected>=4)w->selected=3;
  w->highlight[w->hintbank][4]=(u8)(w->selected+34);
  w->choice.bytes[1]=(u8)w->selected;
  ((u8*)&w->slots[w->selected])[2]=250;((u8*)&w->slots[w->selected])[0]=0;
  w->ui[w->uibank].bytes[22]=(u8)(w->selected+4);
 }
 w->ui[w->uibank].bytes[12+w->selected]=2;
 if(w->accepted && w->rows[w->rowbank].state10==3){
  int captured=3;emit(1,1,0,0);
  for(int i=0;i<4;i++){u8 copy[4];memcpy(copy,w->choice.bytes,4);memcpy((u8*)&w->settings+28+i*4,copy,4);}
  w->rows[w->rowbank].state10=4;
  w->ui[w->uibank].bytes[20]=(u8)captured;
  put16((u8*)&w->settings+26,2);
  ((u8*)&w->settings)[29]=w->choice.bytes[1];
  emit(2,0,0,0);emit(3,0,0,0);w->current=w->resource;emit(4,0,1,0);
 }
}
static uint32_t state=0x71C81925;
static uint32_t next(void){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
static void bind(World *owner){w=owner;count=0;memset(entries,0,sizeof(entries));}
int main(void){
 assert(sizeof(void*)>4 && (uintptr_t)arena>UINT32_MAX);
 for(unsigned k=0;k<131072;k++){
  seed=k*2654435761U;mutation=k>=65536;
  for(size_t i=0;i<sizeof(actual);i++)((u8*)&actual)[i]=(u8)next();
  actual.rowbank=actual.uibank=actual.hintbank=0;
  actual.choice.bytes[1]=(u8)(k*17U);actual.buttons=(u16)k;actual.accepted=k&4;
  actual.rows[0].state10=k%5;actual.rows[1].state10=3;
  actual.resource=arena[k%32];actual.current=arena[(k+1)%32];expected=actual;
  bind(&actual);func_80036D40();int actual_count=count;memcpy(actual_entries,entries,sizeof(entries));
  bind(&expected);reference();
  if(count!=actual_count || memcmp(entries,actual_entries,sizeof(entries)) || memcmp(&actual,&expected,sizeof(actual))){fprintf(stderr,"case%u mutation%d failed actual calls%d expected%d\n",k,mutation,actual_count,count);return 1;}
 }
 puts("131072 native choice cases: OK (UBSan)");return 0;
}
'''
        with TemporaryDirectory() as temporary:
            p=Path(temporary)
            source=p/'choice.c'
            source.write_text(prefix+s+reference)
            executable=p/'choice'
            subprocess.run(['clang','-std=gnu11','-O2','-fsanitize=undefined',
                            '-fno-sanitize-recover=all','-I'+str(ROOT/'include'),
                            str(source),'-o',str(executable)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(executable)],check=True,capture_output=True,text=True)
            self.assertIn('131072 native choice cases: OK (UBSan)',result.stdout)


if __name__=='__main__':
    unittest.main()
