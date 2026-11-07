"""Player status renderer: exhaustive signed-byte pairs and live callback state.

The text renderer is a read-only simulated service; no GPU execution is claimed.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess,unittest
ROOT=Path(__file__).resolve().parents[2]
class PlayerStatusTest(unittest.TestCase):
 def test_status_pairs_flags_and_wrapping_countdown(self):
  image=(ROOT/'assets/PAL/main.exe').read_bytes()
  declarations=[]
  for address,size in [(0x80010600,12),(0x8001060C,28),(0x80010628,20),(0x8001063C,16),(0x8001064C,12),(0x80010658,28),(0x80010674,4)]:
   offset=address-0x80010000+0x800;data=image[offset:offset+size]
   self.assertEqual(len(data),size);self.assertIn(0,data)
   declarations.append(f'u8 {__import__("rename_symbols").registry().get(f"D_{address:08X}",f"D_{address:08X}")}[{size}]={{'+','.join(map(str,data))+'};')
  body=(ROOT/'tools/scripts/fixtures/views/func_80035478.c').read_text().split('void DrawLinkEntryScreen',1)[1]
  code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/player_status.h"
'''+ '\n'.join(declarations)+r'''
typedef struct {u32 guard[4];s8 status[2][2];u16 flags;s32 budget;u32 rng,tail[4];} State;
typedef struct {State before;s32 x,y;u32 kind,length;u8 bytes[32];} Event;
typedef struct {State s;Event events[12];unsigned count;} World;
static World actual,reference,*w;
static unsigned scenario,mutation,seen[8],flagSeen[65536],pairSeen[2][65536],budgetSeen[18];
static const s32 budgets[18]={INT32_MIN,INT32_MIN+1,INT32_MAX-1,INT32_MAX,-1000,-501,-500,-50,-1,0,1,49,50,99,100,499,500,501};
static u8 *labels[7]={StrPlayerLabel,StrNowAcceptingEntries,EntryAcceptedStr,StrStarPleaseWait,StrNoPlayer,SecondsUntilCloseText,D_80010674};
static const unsigned sizes[7]={12,28,20,16,12,28,4};
#define LinkEntryState (w->s.status)
#define LinkRoleSlot (w->s.flags)
#define RaceTimeRemaining (w->s.budget)
static s32 word(u32 u){s32 n;memcpy(&n,&u,4);return n;}
static s16 half(u16 u){s16 n;memcpy(&n,&u,2);return n;}
static s8 byte(u8 u){s8 n;memcpy(&n,&u,1);return n;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static s32 wrap64(int64_t n){return word((u32)n);}
void DrawBigFontString(s32 x,s32 y,u8 *text){Event *e;unsigned i,j,n=4;assert(w->count<12);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->before=w->s;e->x=x;e->y=y;
 for(i=0;i<7;i++)if(text==labels[i]){e->kind=i+1;n=sizes[i];break;}
 if(!e->kind)assert((uintptr_t)text>UINT32_MAX);
 assert(n<=sizeof(e->bytes));memcpy(e->bytes,text,n);for(i=0;i<n&&text[i];i++);assert(i<n);e->length=i;
 if(mutation){for(i=0;i<2;i++)for(j=0;j<2;j++)w->s.status[i][j]=byte((u8)next());w->s.flags=(u16)next();w->s.budget=(next()%2)?budgets[next()%18]:word(next());}
}
void DrawLinkEntryScreen'''+body+r'''
static void budgetCoverage(void){unsigned i;for(i=0;i<18;i++)if(w->s.budget==budgets[i])budgetSeen[i]++;}
static void expected(s32 mode){u8 number[4]={0,0,0,0};unsigned row;int player;s8 status;s32 seconds;int64_t scaled,x;
 if(mode<0){seen[0]++;return;}assert(mode==0||mode==1);seen[1]++;
 for(row=0;row<2;row++){
  player=1-mode;number[0]=(u8)(49+(int)row+2*(int)half(w->s.flags^1U));
  DrawBigFontString(12,64+194*row,StrPlayerLabel);DrawBigFontString(88,64+194*row,number);
  status=w->s.status[player][row];
  if(status==0){seen[2]++;DrawBigFontString(60,120+194*row,StrNowAcceptingEntries);}
  else if(status==1){seen[3]++;DrawBigFontString(60,120+194*row,EntryAcceptedStr);DrawBigFontString(60,150+194*row,StrStarPleaseWait);}
  else if(status==-1){seen[4]++;DrawBigFontString(60,120+194*row,StrNoPlayer);}else seen[5]++;
 }
 budgetCoverage();seconds=(s32)((int64_t)w->s.budget/50);number[0]=(u8)((int64_t)seconds/10+48);number[1]=(u8)((int64_t)seconds%10+48);
 DrawBigFontString(44,244,number);DrawBigFontString(32,244,SecondsUntilCloseText);budgetCoverage();
 scaled=wrap64((int64_t)500-w->s.budget);scaled=wrap64(scaled*256);x=scaled/500+32;
 DrawBigFontString((s32)x,224,D_80010674);
}
int main(void){unsigned which,i,j;u16 pair;s32 mode;uint64_t cases=0;
 _Static_assert(sizeof(unsigned long)>=sizeof(void*),"native-width address cursor");_Static_assert(sizeof(actual.s.status)==4,"complete two-by-two status owner");
 assert((uintptr_t)&actual>UINT32_MAX);
 for(scenario=0;scenario<69632;scenario++)for(which=0;which<2;which++)for(mutation=0;mutation<2;mutation++){
  memset(&actual,0,sizeof(actual));memset(&reference,0,sizeof(reference));w=&actual;w->s.rng=0x35478U+scenario*73U+which*19U;
  w->s.status[1][0]=byte((u8)scenario);w->s.status[1][1]=byte((u8)(scenario>>8));w->s.status[0][0]=byte((u8)(scenario^128U));w->s.status[0][1]=byte((u8)((scenario>>8)^128U));
  w->s.flags=(u16)scenario;w->s.budget=budgets[scenario%18];
  if(scenario%7==0)w->s.budget=word(next());
  mode=scenario<65536?(s32)which:(which?-1:word(next()|0x80000000U));
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}
  if(mode>=0){pair=(u8)w->s.status[1-mode][0]|((u16)(u8)w->s.status[1-mode][1]<<8);pairSeen[which][pair]++;flagSeen[w->s.flags]++;}
  reference.s=actual.s;w=&actual;DrawLinkEntryScreen(mode);w=&reference;expected(mode);
  assert(!memcmp(&actual.s,&reference.s,sizeof(State)));assert(actual.count==reference.count);assert(!memcmp(actual.events,reference.events,actual.count*sizeof(Event)));cases++;
 }
 assert(cases==278528);for(i=0;i<65536;i++){assert(flagSeen[i]);for(j=0;j<2;j++)assert(pairSeen[j][i]);}for(i=0;i<6;i++)assert(seen[i]);for(i=0;i<18;i++)assert(budgetSeen[i]);return 0;
}
'''
  with TemporaryDirectory() as tmp:
   p=Path(tmp);(p/'test.c').write_text(code)
   r=subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
   self.assertEqual(r.returncode,0,r.stderr);subprocess.run([str(p/'test')],check=True)
if __name__=='__main__':unittest.main()
