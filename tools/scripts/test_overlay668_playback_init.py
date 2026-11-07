"""Playback initializer: complete native allocation and mutable setup services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class PlaybackInitTest(unittest.TestCase):
    def test_captured_allocation_and_fresh_resource(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/668/func_801149E8.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4||sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4||offsetof(t,f)==n,"PAL offset")
#include "rr/overlay668_playback_init.h"
typedef struct {u32 guard[4],rng;RrStrContextPrefix context;void *base,*cursor;s16 channel,mode,status;u32 tail[4];} State;
typedef struct {s32 kind,args[4];unsigned owner;u32 hash;} Event;
typedef struct {State s;Event events[8];unsigned count;} World;
typedef struct {u32 before[4];RrStrPlaybackState player;u32 after[4];} Owner;
static World actual,reference,initial,*w;
static Owner owners[3],expectedOwners[3],initialOwners[3];
static unsigned scenario,mutation,selectedSeen[3],resourceSeen[128],clearSeen;
#define RaceContext w->s.context
#define MenuArenaBase w->s.base
#define LoadCursor w->s.cursor
#define ModeGroup w->s.channel
#define ModeId w->s.mode
#define D_800F4D8A w->s.status
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static u32 next(void){u32 n=w->s.rng;n^=n<<13;n^=n>>17;n^=n<<5;return w->s.rng=n;}
static unsigned ownerIndex(const void *p){unsigned i;assert((uintptr_t)p>UINT32_MAX);for(i=0;i<3;i++)if(p==&owners[i].player)return i;assert(0);return 0;}
static Event *event(s32 kind){unsigned i;u32 h=2166136261U;Owner *p=w==&actual?owners:expectedOwners;Event *e;
 assert(w->count<8);e=&w->events[w->count++];memset(e,0,sizeof(*e));e->kind=kind;
 for(i=0;i<sizeof(w->s);i++)h=(h^((u8*)&w->s)[i])*16777619U;
 for(i=0;i<sizeof(owners);i++)h=(h^((u8*)p)[i])*16777619U;e->hash=h;return e;
}
static void change(void){unsigned i;Owner *p=w==&actual?owners:expectedOwners;if(!mutation)return;
 w->s.base=&owners[next()%3].player;w->s.cursor=&owners[next()%3].player;RaceContext.activeB8=&owners[next()%3].player;
 RaceContext.resourceB0=next()%128;RaceContext.phaseB4=word(next());RaceContext.stopD0=word(next());
 w->s.channel=(s16)next();w->s.mode=(s16)next();w->s.status=(s16)next();
 for(i=0;i<3;i++)p[i].player.width00=word(next());
}
void SetDispMask(s32 mode){event(1)->args[0]=mode;change();}
void SetupDrawDispEnvs(s32 r,s32 g,s16 b,u32 flags){Event *e=event(2);e->args[0]=r;e->args[1]=g;e->args[2]=b;e->args[3]=word(flags);change();}
void PsyqBzero(void *p,u32 count){unsigned i=ownerIndex(p);Event *e=event(3);Owner *o=w==&actual?owners:expectedOwners;
 assert(count==sizeof(RrStrPlaybackState));assert(RaceContext.activeB8==p);assert(w->s.cursor==(u8*)p+sizeof(RrStrPlaybackState));
 e->owner=i;e->args[0]=word(count);selectedSeen[i]++;clearSeen++;memset(&o[i].player,0,count);change();
}
s32 ResetGraph(s32 mode){event(4)->args[0]=mode;change();return word(0xF0123456U);}
s32 CdSync(s32 mode,u8 *status){Event *e=event(5);e->args[0]=mode;assert(status==0);change();return -1;}
s32 CdControl(s32 command,const u8 *parameters,u8 *result){Event *e=event(6);e->args[0]=command;assert(parameters==0&&result==0);change();return scenario%3;}
void ovl668_func_80114910(s32 resource,RrStrCdFileInfo *file,u32 *sectors){Event *e=event(7);uintptr_t f=(uintptr_t)file,s=(uintptr_t)sectors;
 assert(f>UINT32_MAX&&s>UINT32_MAX);assert(s+sizeof(*sectors)<=f||f+sizeof(*file)<=s);assert(resource>=0&&resource<128);resourceSeen[resource]++;
 e->args[0]=resource;
 /* Like sector-to-location conversion, only the three address bytes are
  * outputs. The initializer must never inspect the remaining local bytes. */
 file->location.c[0]=12;file->location.c[1]=34;file->location.c[2]=56;file->byteSize=0x12345000U;*sectors=0x2468AU;change();
}
''' + body + r'''
static void expected(void){RrStrCdFileInfo file;u32 sectors;void *captured;
 SetDispMask(0);SetupDrawDispEnvs(0,0,0,5);captured=w->s.base;
 w->s.cursor=captured;RaceContext.activeB8=captured;w->s.cursor=(u8*)captured+sizeof(RrStrPlaybackState);
 PsyqBzero(captured,sizeof(RrStrPlaybackState));ResetGraph(3);CdSync(0,0);CdControl(9,0,0);
 RaceContext.phaseB4=0;RaceContext.stopD0=0;ovl668_func_80114910(RaceContext.resourceB0,&file,&sectors);
 w->s.channel=2;w->s.mode=4;w->s.status=0;
}
int main(void){unsigned i;uint64_t cases=0;
 for(scenario=0;scenario<16384;scenario++)for(mutation=0;mutation<2;mutation++){
  memset(&initial,0,sizeof(initial));w=&initial;w->s.rng=0x149E8+scenario*73;
  for(i=0;i<sizeof(initialOwners);i++)((u8*)initialOwners)[i]=(u8)next();
  for(i=0;i<4;i++){w->s.guard[i]=next();w->s.tail[i]=next();}
  w->s.base=&owners[scenario%3].player;w->s.cursor=&owners[(scenario+1)%3].player;RaceContext.activeB8=&owners[(scenario+2)%3].player;
  RaceContext.resourceB0=scenario%128;RaceContext.phaseB4=word(next());RaceContext.stopD0=word(next());
  w->s.channel=(s16)next();w->s.mode=(s16)next();w->s.status=(s16)next();
  actual=reference=initial;memcpy(owners,initialOwners,sizeof(owners));memcpy(expectedOwners,initialOwners,sizeof(owners));
  w=&actual;ovl668_func_801149E8();w=&reference;expected();
  assert(actual.count==7&&!memcmp(&actual,&reference,sizeof(actual)));assert(!memcmp(owners,expectedOwners,sizeof(owners)));cases++;
 }
 assert(cases==32768&&clearSeen==65536);for(i=0;i<3;i++)assert(selectedSeen[i]);for(i=0;i<128;i++)assert(resourceSeen[i]);return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(p / 'test.c'), '-o', str(p / 'test')], check=True)
            subprocess.run([str(p / 'test')], check=True)
