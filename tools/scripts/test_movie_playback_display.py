"""Actual STR configuration and full-width buffer publication, including aliases."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayStrDisplayTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        path = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{name}.c').read_text()
                           .replace('#include "rr/overlay668_display.h"', '')
                           for name in ('80114E48', '80114E90'))
        source = r'''
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay668_display.h"
#define PREFIX(j) (offsetof(RrStrContextPrefix,activeB8)-offsetof(RrStrBufferSet,buffers)-(j)*sizeof(void*))
typedef union {
 RrStrContextPrefix context;
 struct {u8 before[PREFIX(0)];RrStrBufferSet set;} from0;
 struct {u8 before[PREFIX(1)];RrStrBufferSet set;} from1;
 struct {u8 before[PREFIX(2)];RrStrBufferSet set;} from2;
 struct {u8 before[PREFIX(3)];RrStrBufferSet set;} from3;
 struct {u8 before[PREFIX(4)];RrStrBufferSet set;} from4;
} RootViews;
/* The real partition addresses each contain a complete possible buffer owner. */
typedef struct {
 RrStrBufferSet at0;
 u8 gap0[0xA000-sizeof(RrStrBufferSet)];
 RrStrBufferSet atA000;
 u8 gap1[0xA000-sizeof(RrStrBufferSet)];
 RrStrBufferSet at14000;
 u8 gap2[0x2D00-sizeof(RrStrBufferSet)];
 RrStrBufferSet at16D00;
 u8 gap3[0x2D00-sizeof(RrStrBufferSet)];
 RrStrBufferSet at19A00;
} PartitionArena;
static struct BufferWorld {
 u32 guards0[8];RootViews root;RrStrBufferSet sets[2];
 PartitionArena arena;u32 guards1[8];
} bworld,binitial,bexpected;
static struct DisplayWorld {
 u32 guards0[8];RrStrDisplayConfig configs[3];s32 format;u32 guards1[8];
} dworld,dinitial,dexpected;
#define RaceContext bworld.root.context
#define DrawBufIndex dworld.format
''' + bodies + r'''
static u32 seed=0x14e90;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 signed32(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static void fill(void *object,size_t size){size_t i;for(i=0;i<size;i++)((u8*)object)[i]=(u8)next();}
static void half(s16 *dest,s32 value){u16 bits=(u16)(u32)value;memcpy(dest,&bits,2);}
static int display(void){
 u32 k,i;
 for(k=0;k<65536;k++){
  s32 args[4];RrStrDisplayConfig *config;
  fill(&dworld,sizeof(dworld));
  args[0]=signed32((next()&0xffff0000U)|k);
  args[1]=signed32((next()&0xffff0000U)|((k*109U+17U)&65535));
  args[2]=signed32((next()&0xffff0000U)|((~k)&65535));
  args[3]=signed32((next()&0xffff0000U)|((k*251U+73U)&65535));
  dworld.format=signed32(next());
  memcpy(&dinitial,&dworld,sizeof(dworld));config=&dworld.configs[k%3];
  for(i=0;i<3;i++){
   half(&config->rectangles[i].w,i==2?24:480);
   half(&config->rectangles[i].h,240);
  }
  half(&config->rectangles[0].x,args[0]);half(&config->rectangles[0].y,args[1]);
  half(&config->rectangles[1].x,args[2]);half(&config->rectangles[1].y,args[3]);
  half(&config->rectangles[2].x,args[0]);half(&config->rectangles[2].y,args[1]);
  config->alternate04=((u32)dworld.format%2U)==0;
  memcpy(&dexpected,&dworld,sizeof(dworld));memcpy(&dworld,&dinitial,sizeof(dworld));
  movie_playback_set_dec_env(&dworld.configs[k%3],args[0],args[1],args[2],args[3]);
  if(memcmp(&dworld,&dexpected,sizeof(dworld))){fprintf(stderr,"display case %u\n",k);return 1;}
 }
 puts("65536 STR display cases with UBSan passed");return 0;
}
static int buffers(void){
 static const size_t offsets[]={0,0xA000,0x14000,0x16D00,0x19A00};
 u32 k,i;RrStrBufferSet *owners[5];
 assert(sizeof(void*)==8 && (uintptr_t)&bworld.arena>UINT32_MAX);
 assert(offsetof(PartitionArena,atA000)==0xA000);
 assert(offsetof(PartitionArena,at14000)==0x14000);
 assert(offsetof(PartitionArena,at16D00)==0x16D00);
 assert(offsetof(PartitionArena,at19A00)==0x19A00);
 assert(&bworld.root.context.activeB8==&bworld.root.from0.set.buffers[0]);
 assert(&bworld.root.context.activeB8==&bworld.root.from1.set.buffers[1]);
 assert(&bworld.root.context.activeB8==&bworld.root.from2.set.buffers[2]);
 assert(&bworld.root.context.activeB8==&bworld.root.from3.set.buffers[3]);
 assert(&bworld.root.context.activeB8==&bworld.root.from4.set.buffers[4]);
 memset(&bworld,0xA5,sizeof(bworld));
 owners[0]=&bworld.arena.at0;owners[1]=&bworld.arena.atA000;
 owners[2]=&bworld.arena.at14000;owners[3]=&bworld.arena.at16D00;owners[4]=&bworld.arena.at19A00;
 for(k=0;k<4096;k++){
  fill(&bworld.root,sizeof(bworld.root));fill(bworld.sets,sizeof(bworld.sets));
  fill(bworld.guards0,sizeof(bworld.guards0));fill(bworld.guards1,sizeof(bworld.guards1));
  for(i=0;i<5;i++)fill(owners[i],sizeof(*owners[i]));
  switch(k%7){
   case 0:bworld.root.context.activeB8=&bworld.sets[0];break;
   case 1:bworld.root.context.activeB8=&bworld.sets[1];break;
   case 2:bworld.root.context.activeB8=&bworld.root.from0.set;break;
   case 3:bworld.root.context.activeB8=&bworld.root.from1.set;break;
   case 4:bworld.root.context.activeB8=&bworld.root.from2.set;break;
   case 5:bworld.root.context.activeB8=&bworld.root.from3.set;break;
   default:bworld.root.context.activeB8=&bworld.root.from4.set;break;
  }
  memcpy(&binitial,&bworld,sizeof(bworld));
  for(i=0;i<5;i++){
   RrStrBufferSet *target=bworld.root.context.activeB8;
   target->buffers[i]=(u8*)&bworld.arena+offsets[i];
  }
  memcpy(&bexpected,&bworld,sizeof(bworld));memcpy(&bworld,&binitial,sizeof(bworld));
  movie_playback_set_buffers((u8*)&bworld.arena);
  if(memcmp(&bworld,&bexpected,sizeof(bworld))){fprintf(stderr,"buffers case %u\n",k);return 1;}
 }
 puts("4096 STR buffer cases with UBSan passed");return 0;
}
int main(int argc,char **argv){return argc>1 && argv[1][0]=='b'?buffers():display();}
'''
        (path / 'str.c').write_text(source)
        cls.executable = path / 'str'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                        '-fno-sanitize-recover=all', '-I' + str(root / 'include'),
                        str(path / 'str.c'), '-o', str(cls.executable)],
                       check=True, capture_output=True)

    def run_case(self, mode, expected):
        result = subprocess.run([str(self.executable), mode], check=True,
                                capture_output=True, text=True)
        self.assertIn(expected, result.stdout)

    def test_display_coordinates_and_format(self):
        self.run_case('display', '65536 STR display cases')

    def test_full_pointers_and_active_owner_aliases(self):
        self.run_case('buffers', '4096 STR buffer cases')


if __name__ == '__main__':
    unittest.main()
