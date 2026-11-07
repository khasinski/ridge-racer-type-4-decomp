"""STR command flags, DMA forwarding, status routing and native callback pointers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayControlTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        path = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{name}.c').read_text()
                           .replace('#include "rr/overlay668_control.h"', '')
                           for name in ('801163FC', '80116478', '80116498', '801164D4', '80116510'))
        resident = (root / 'tools/scripts/fixtures/legacy/func_8008B3C4.c').read_text()
        resident = resident.replace('#include "rr/library.h"', '')
        query = (root / 'src/overlays/PAL/movie_playback/func_8011688C.c').read_text()
        query = query.replace('#include "types.h"', '').replace('movie_playback_mdec_status', 'rr_real_status')
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n)
#define RR_ASSERT_OFFSET(t,f,n)
#include "rr/overlay668_control.h"
static struct World {
 u32 before[8];RrStrCommandHeader header;u32 output[64],status;
 s32 input_result,output_result;u32 mutate;
 u32 command_calls,output_calls,input_waits,output_waits,reads;
 const void *last_pointer;u32 last_count,observed_header;u32 after[8];
} world,initial,expected;
static struct CallbackWorld {
 RrLibInterruptDriverPrefix drivers[2],*active;
 RrLibInterruptCallback slots[8],last;
 u32 calls,tag,mutate;u32 guards[8];
} cworld,cinitial,cexpected;
#define IntrDriverTablePtr cworld.active
u32 *movie_playback_mdec_ctrl_reg;
''' + resident + query + bodies + r'''
static u32 seed=0x163fc;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 signed32(u32 value){s32 v;memcpy(&v,&value,4);return v;}
static void initialize(u32 k){
 unsigned i;
 memset(&world,0,sizeof(world));
 for(i=0;i<8;i++){world.before[i]=next();world.after[i]=next();}
 for(i=0;i<64;i++)world.output[i]=next();
 world.header.word=(next()&0xffff0000U)|k;
 world.status=(next()&~0x21000000U)|((k&4)?0x01000000U:0)|((k&8)?0x20000000U:0);
 world.input_result=k%16==0?0:k%16==4?-1:signed32(next());
 world.output_result=k%16==0?-1:k%16==4?0:signed32(next());
 world.mutate=k&1;movie_playback_mdec_ctrl_reg=&world.status;
}
void movie_playback_mdec_in(const void *command,u32 words){
 const RrStrCommandHeader *header=command;
 assert(command==&world.header);
 world.command_calls++;world.last_pointer=command;world.last_count=words;
 world.observed_header=header->word;
 if(world.mutate)world.header.word^=0xa5a55a5aU;
}
void movie_playback_mdec_out(void *output,u32 words){
 world.output_calls++;world.last_pointer=output;world.last_count=words;
 *(u32*)output^=0x13579bdfU;
}
s32 movie_playback_mdec_in_sync(void){world.input_waits++;world.status^=0x31415926U;return world.input_result;}
s32 movie_playback_mdec_out_sync(void){world.output_waits++;world.status^=0x27182818U;return world.output_result;}
u32 movie_playback_mdec_status(void){u32 value=rr_real_status();world.reads++;world.status^=0xffffffffU;return value;}
static int flags(void){
 u32 k,endian=0x12345678;
 assert(*(u8*)&endian==0x78 && sizeof(void*)==8 && (uintptr_t)&world>UINT32_MAX);
 for(k=0;k<65536;k++){
  u32 options,word;initialize(k);options=next();
  memcpy(&initial,&world,sizeof(world));
  word=(world.header.word&~0x0a000000U)|((options&1)?0:0x08000000U)|((options&2)?0x02000000U:0);
  world.header.word=word;world.command_calls=1;world.last_pointer=&world.header;
  world.last_count=k;world.observed_header=word;
  if(world.mutate)world.header.word^=0xa5a55a5aU;
  memcpy(&expected,&world,sizeof(world));memcpy(&world,&initial,sizeof(world));
  movie_playback_dec_dct_in(&world.header,options);
  if(memcmp(&world,&expected,sizeof(world))){fprintf(stderr,"flags case %u\n",k);return 1;}
 }
 puts("65536 native STR flag cases with UBSan passed");return 0;
}
static int output(void){
 u32 k;
 assert(sizeof(void*)==8 && (uintptr_t)world.output>UINT32_MAX);
 for(k=0;k<65536;k++){
  u32 words,index;initialize(k);index=next()%64;words=(next()&0xffff0000U)|k;
  memcpy(&initial,&world,sizeof(world));
  world.output_calls=1;world.last_pointer=world.output+index;world.last_count=words;
  world.output[index]^=0x13579bdfU;
  memcpy(&expected,&world,sizeof(world));memcpy(&world,&initial,sizeof(world));
  movie_playback_dec_dct_out(world.output+index,words);
  if(memcmp(&world,&expected,sizeof(world))){fprintf(stderr,"output case %u\n",k);return 1;}
 }
 puts("65536 native STR output-forwarding cases with UBSan passed");return 0;
}
static int status(int input){
 u32 k;
 for(k=0;k<65536;k++){
  s32 mode,result,wanted;initialize(k);
  mode=k%4==0?0:k%4==1?1:k%4==2?-1:65536;
  memcpy(&initial,&world,sizeof(world));
  if(mode==0){
   if(input){world.input_waits=1;world.status^=0x31415926U;wanted=world.input_result;}
   else {world.output_waits=1;world.status^=0x27182818U;wanted=world.output_result;}
  }else {
   wanted=(world.status>>(input?29:24))&1U;world.reads=1;world.status^=0xffffffffU;
  }
  memcpy(&expected,&world,sizeof(world));memcpy(&world,&initial,sizeof(world));
  result=input?movie_playback_dec_dct_in_sync(mode):movie_playback_dec_dct_out_sync(mode);
  if(result!=wanted || memcmp(&world,&expected,sizeof(world))){fprintf(stderr,"status case %u type %d\n",k,input);return 1;}
 }
 puts(input?"65536 native STR input-status cases with UBSan passed":"65536 native STR output-status cases with UBSan passed");return 0;
}
static int touches;
static void callback_a(void){touches+=1;}
static void callback_b(void){touches+=2;}
static void callback_c(void){touches+=4;}
static RrLibInterruptCallback exchange(u32 tag,s32 channel,RrLibInterruptCallback callback){
 RrLibInterruptCallback old;
 assert(channel==0);old=cworld.slots[0];cworld.slots[0]=callback;
 cworld.calls++;cworld.last=callback;cworld.tag=tag;
 if(cworld.mutate)cworld.active=&cworld.drivers[tag==1?1:0];
 return old;
}
static RrLibInterruptCallback exchange_a(s32 c,RrLibInterruptCallback f){return exchange(1,c,f);}
static RrLibInterruptCallback exchange_b(s32 c,RrLibInterruptCallback f){return exchange(2,c,f);}
static int callbacks(void){
 RrLibInterruptCallback choices[]={0,callback_a,callback_b,callback_c};
 unsigned k,i;
 assert(sizeof(void*)==8 && (uintptr_t)callback_a>UINT32_MAX);
 for(k=0;k<16384;k++){
  RrLibInterruptCallback input,old,got;u32 tag;
  for(i=0;i<8;i++){cworld.guards[i]=next();cworld.slots[i]=choices[(k+i)%4];}
  cworld.drivers[0]=(RrLibInterruptDriverPrefix){callback_a,exchange_a,exchange_b,callback_c};
  cworld.drivers[1]=(RrLibInterruptDriverPrefix){callback_b,exchange_b,exchange_a,callback_c};
  cworld.active=&cworld.drivers[(k>>2)&1];cworld.calls=next();cworld.last=0;cworld.tag=0;cworld.mutate=(k>>3)&1;
  input=choices[(k>>4)&3];old=cworld.slots[0];tag=cworld.active->exchange==exchange_a?1:2;
  memcpy(&cinitial,&cworld,sizeof(cworld));
  cworld.slots[0]=input;cworld.calls++;cworld.last=input;cworld.tag=tag;
  if(cworld.mutate)cworld.active=&cworld.drivers[tag==1?1:0];
  memcpy(&cexpected,&cworld,sizeof(cworld));memcpy(&cworld,&cinitial,sizeof(cworld));
  touches=0;got=movie_playback_dec_dct_in_callback(input);
  if(got!=old || touches || memcmp(&cworld,&cexpected,sizeof(cworld))){fprintf(stderr,"callback0 case %u\n",k);return 1;}
  if(got)got();
  assert(touches==(old==callback_a?1:old==callback_b?2:old==callback_c?4:0));
 }
 puts("16384 native STR callback0 cases with UBSan passed");return 0;
}
int main(int argc,char **argv){
 if(argc>1 && argv[1][0]=='b')return output();
 if(argc>1 && argv[1][0]=='i')return status(1);
 if(argc>1 && argv[1][0]=='s')return status(0);
 if(argc>1 && argv[1][0]=='c')return callbacks();
 return flags();
}
'''
        (path / 'control.c').write_text(source)
        cls.executable = path / 'control'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                        '-fno-sanitize-recover=all', '-I' + str(root / 'include'),
                        str(path / 'control.c'), '-o', str(cls.executable)],
                       check=True, capture_output=True)

    def run_case(self, mode, expected):
        result = subprocess.run([str(self.executable), mode], check=True,
                                capture_output=True, text=True)
        self.assertIn(expected, result.stdout)

    def test_command_flags_and_low16_count(self):
        self.run_case('flags', '65536 native STR flag cases')

    def test_output_pointer_and_unsigned_word_count(self):
        self.run_case('buffer', '65536 native STR output-forwarding cases')

    def test_input_wait_result_and_status_bit29(self):
        self.run_case('input', '65536 native STR input-status cases')

    def test_output_wait_result_and_status_bit24(self):
        self.run_case('status', '65536 native STR output-status cases')

    def test_callback0_and_actual_resident_dispatch(self):
        self.run_case('callbacks', '16384 native STR callback0 cases')


if __name__ == '__main__':
    unittest.main()
