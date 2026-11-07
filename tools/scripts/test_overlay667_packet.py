"""Serial packet payload bounds, unsigned bytes and mutating service ABI."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay667PacketTest(unittest.TestCase):
    def test_packet_and_driver_write_boundaries(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/667/func_80114D30.c').read_text()
        source=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay667_packet.h"
RrOverlay667SerialStorage ovl667_D_80118F80;
RrOverlay667ChoicePair LinkLocalInputs;
u8 ovl667_D_80118F09,ovl667_D_80118EEF;
s32 RaceState,ovl667_D_80118EEC,LinkWriteHandle;
static u8 expected[12],after[12],choice_after[8];
static s32 after_words[3];
static unsigned case_id,mode,calls;
static u32 rng=0x14D30;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static s32 word(u32 n){s32 v;memcpy(&v,&n,4);return v;}
static void mutate(void){unsigned j;
 for(j=0;j<12;j++)((u8*)&ovl667_D_80118F80)[j]=(u8)next();
 for(j=0;j<8;j++)((u8*)&LinkLocalInputs)[j]=(u8)next();
 LinkWriteHandle=word(next());RaceState=word(next());ovl667_D_80118EEC=word(next());
 ovl667_D_80118F09=(u8)next();ovl667_D_80118EEF=(u8)next();
}
static void save(void){memcpy(after,&ovl667_D_80118F80,12);memcpy(choice_after,&LinkLocalInputs,8);
 after_words[0]=LinkWriteHandle;after_words[1]=RaceState;after_words[2]=ovl667_D_80118EEC;}
s32 _sio_control(s32 command,s32 operation,unsigned long argument){unsigned j;u8 x=0;
 static const u8 peers[]={0,1,2,255};
 assert(calls++==0);assert(command==3&&operation==1&&argument==0);
 mutate();ovl667_D_80118F09=(u8)case_id;ovl667_D_80118EEF=peers[(case_id>>8)&3];
 expected[0]=(u8)(ovl667_D_80118F09|(ovl667_D_80118EEF==1?128:0));
 for(j=0;j<8;j++){expected[4+j]=((u8*)&LinkLocalInputs)[j];x^=expected[4+j];}
 expected[1]=x;expected[2]=(u8)RaceState;expected[3]=(u8)ovl667_D_80118EEC;
 save();return mode==0?0:mode==1?-1:word(0x80000000U|case_id);
}
s32 FileWrite(s32 descriptor,const void *buffer,s32 count){
 static const s32 result[]={0,12,0,-1,13};
 assert(calls++==1);assert(descriptor==LinkWriteHandle&&count==12);
 assert(buffer==&ovl667_D_80118F80);assert((uintptr_t)buffer>UINT32_MAX);
 assert(memcmp(buffer,expected,12)==0);
 mutate();save();return result[mode];
}
'''+body+r'''
int main(void){
 for(case_id=0;case_id<65536;case_id++)for(mode=0;mode<5;mode++){
  calls=0;mutate();
  assert(ovl667_func_80114D30()==(mode==0?0:mode==1?1:2));
  assert(calls==(mode==0?1:2));
  assert(memcmp(&ovl667_D_80118F80,after,12)==0);
  assert(memcmp(&LinkLocalInputs,choice_after,8)==0);
  assert(LinkWriteHandle==after_words[0]&&RaceState==after_words[1]&&ovl667_D_80118EEC==after_words[2]);
 }
 return 0;
}
'''
        with TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
