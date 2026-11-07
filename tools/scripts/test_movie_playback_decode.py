"""Actual STR table/header helpers and callback exchange with full native pointers."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayDecodeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        path = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        bodies = '\n'.join((root / f'src/overlays/PAL/movie_playback/func_{name}.c').read_text()
                           .replace('#include "rr/overlay668_decode.h"', '')
                           for name in ('80116298', '801162CC', '80116358', '801163F0', '80116534'))
        import re
        bodies = re.sub(r'\s+(?:asm|__asm__)\("\$\d+"\)', '', bodies)
        resident = (root / 'tools/scripts/fixtures/legacy/func_8008B3C4.c').read_text()
        resident += '\n' + (root / 'tools/scripts/fixtures/legacy/func_8008B364.c').read_text()
        resident = resident.replace('#include "rr/library.h"', '')
        source = r'''
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
#include "rr/overlay668_decode.h"
typedef union {
 u32 words[192];
 struct {
  u32 before[31];RrStrQuantizationCommand quant;
  RrStrMatrixCommand matrix;u32 after[95];
 } tables;
} TableViews;
static struct WordWorld {
 u32 before[8];TableViews image;u32 after[8];
 u32 command_calls,packets[2][33],counts[2],mutate;
 const void *addresses[2];
} wworld,winitial,wexpected;
static struct CallbackWorld {
 u32 before[8];RrLibInterruptDriverPrefix drivers[2];
 RrLibInterruptDriverPrefix *active;
 RrLibInterruptCallback callbacks[8],last;
 s32 channel;u32 calls,tag,mutate;u32 after[8];
} cworld,cinitial,cexpected;
#define movie_playback_dct_quant_command wworld.image.tables.quant
#define movie_playback_dct_matrix_command wworld.image.tables.matrix
#define IntrDriverTablePtr cworld.active
''' + resident + bodies + r'''
static u32 seed=0x162cc;
static int callback_touches;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static void init_words(void){
 unsigned i;
 for(i=0;i<8;i++){wworld.before[i]=next();wworld.after[i]=next();}
 for(i=0;i<192;i++)wworld.image.words[i]=next();
 wworld.command_calls=wworld.mutate=0;
 memset(wworld.packets,0,sizeof(wworld.packets));
 for(i=0;i<2;i++){wworld.addresses[i]=0;wworld.counts[i]=0;}
}
static int table_copy(void){
 unsigned k,i,position;
 assert(sizeof(void*)==8 && (uintptr_t)&wworld>UINT32_MAX);
 assert(sizeof(RrStrQuantizationCommand)==132 && sizeof(RrStrMatrixCommand)==132);
 assert(offsetof(TableViews,tables.quant.luma)==32*4);
 assert(offsetof(TableViews,tables.quant.chroma)==48*4);
 assert(offsetof(TableViews,tables.matrix.matrix)==65*4);
 for(k=0;k<65536;k++){
  u32 *output;init_words();position=k%129;
  memcpy(&winitial,&wworld,sizeof(wworld));
  for(i=0;i<16;i++)wworld.image.words[position+i]=wworld.image.words[32+i];
  for(i=0;i<16;i++)wworld.image.words[position+16+i]=wworld.image.words[48+i];
  for(i=0;i<32;i++)wworld.image.words[position+32+i]=wworld.image.words[65+i];
  memcpy(&wexpected,&wworld,sizeof(wworld));memcpy(&wworld,&winitial,sizeof(wworld));
  output=wworld.image.words+position;
  if(movie_playback_dec_dct_get_env(output)!=output || memcmp(&wworld,&wexpected,sizeof(wworld))){
   fprintf(stderr,"table-copy case %u\n",k);return 1;
  }
 }
 puts("65536 native STR table-copy cases with UBSan passed");return 0;
}

void movie_playback_mdec_in(const void *command,u32 count){
 u32 slot=wworld.command_calls++;
 assert(slot<2 && count==32);
 wworld.addresses[slot]=command;wworld.counts[slot]=count;
 memcpy(wworld.packets[slot],command,33*sizeof(u32));
 if(slot==0 && wworld.mutate){
  wworld.image.tables.quant.luma[0]^=0x13572468U;
  wworld.image.tables.matrix.header.word^=0x01234567U;
  wworld.image.tables.matrix.matrix[0]^=0x89abcdefU;
 }
}
static int table_publish(void){
 unsigned k,i,position;
 assert(sizeof(void*)==8 && (uintptr_t)&wworld>UINT32_MAX);
 for(k=0;k<65536;k++){
  u32 *input;init_words();position=k%161;wworld.mutate=k&1;
  memcpy(&winitial,&wworld,sizeof(wworld));
  for(i=0;i<16;i++)wworld.image.words[32+i]=wworld.image.words[position+i];
  for(i=0;i<16;i++)wworld.image.words[48+i]=wworld.image.words[position+16+i];
  wworld.command_calls=2;
  wworld.addresses[0]=&wworld.image.tables.quant;wworld.counts[0]=32;
  memcpy(wworld.packets[0],wworld.image.words+31,33*sizeof(u32));
  if(wworld.mutate){wworld.image.words[32]^=0x13572468U;wworld.image.words[64]^=0x01234567U;wworld.image.words[65]^=0x89abcdefU;}
  wworld.addresses[1]=&wworld.image.tables.matrix;wworld.counts[1]=32;
  memcpy(wworld.packets[1],wworld.image.words+64,33*sizeof(u32));
  memcpy(&wexpected,&wworld,sizeof(wworld));memcpy(&wworld,&winitial,sizeof(wworld));
  input=wworld.image.words+position;
  if(movie_playback_dec_dct_put_env(input)!=input || memcmp(&wworld,&wexpected,sizeof(wworld))){
   fprintf(stderr,"table-publish case %u\n",k);return 1;
  }
 }
 puts("65536 native STR table-publish cases with UBSan passed");return 0;
}
static int header_read(void){
 unsigned k;u32 endian=0x12345678;
 assert(*(u8*)&endian==0x78);
 for(k=0;k<65536;k++){
  RrStrCommandHeader *header;init_words();
  header=k&1?&wworld.image.tables.quant.header:&wworld.image.tables.matrix.header;
  header->word=(next()&0xffff0000U)|k;
  memcpy(&wexpected,&wworld,sizeof(wworld));
  if(movie_playback_dec_dct_buf_size(header)!=k || memcmp(&wworld,&wexpected,sizeof(wworld))){
   fprintf(stderr,"header-read case %u\n",k);return 1;
  }
 }
 puts("65536 native STR header cases with UBSan passed");return 0;
}
static void callback_a(void){callback_touches+=1;}
static void callback_b(void){callback_touches+=2;}
static void callback_c(void){callback_touches+=4;}
static RrLibInterruptCallback exchange(u32 tag,s32 channel,RrLibInterruptCallback callback){
 RrLibInterruptCallback old;
 assert(channel==1);
 old=cworld.callbacks[channel];cworld.callbacks[channel]=callback;
 cworld.calls++;cworld.channel=channel;cworld.last=callback;cworld.tag=tag;
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
  RrLibInterruptCallback input,old,result;u32 tag;
  for(i=0;i<8;i++){cworld.before[i]=next();cworld.after[i]=next();cworld.callbacks[i]=choices[(k+i)%4];}
  cworld.drivers[0]=(RrLibInterruptDriverPrefix){callback_a,exchange_a,exchange_b,callback_c};
  cworld.drivers[1]=(RrLibInterruptDriverPrefix){callback_b,exchange_b,exchange_a,callback_c};
  if(k&64){cworld.drivers[0].exchange=exchange_b;cworld.drivers[1].exchange=exchange_a;}
  cworld.active=&cworld.drivers[(k>>2)&1];
  cworld.calls=k&128?UINT32_MAX:next();cworld.channel=-1;cworld.last=0;cworld.tag=0;
  cworld.mutate=(k>>3)&1;input=choices[(k>>4)&3];old=cworld.callbacks[1];
  tag=cworld.active->exchange==exchange_a?1:2;
  memcpy(&cinitial,&cworld,sizeof(cworld));
  cworld.callbacks[1]=input;cworld.calls++;cworld.channel=1;cworld.last=input;cworld.tag=tag;
  if(cworld.mutate)cworld.active=&cworld.drivers[tag==1?1:0];
  memcpy(&cexpected,&cworld,sizeof(cworld));memcpy(&cworld,&cinitial,sizeof(cworld));
  callback_touches=0;result=movie_playback_dec_dct_out_callback(input);
  if(result!=old || callback_touches || memcmp(&cworld,&cexpected,sizeof(cworld))){
   fprintf(stderr,"callback-exchange case %u\n",k);return 1;
  }
  if(result)result();
  assert(callback_touches==(old==callback_a?1:old==callback_b?2:old==callback_c?4:0));
 }
 puts("16384 native STR callback cases with UBSan passed");return 0;
}

static u32 reset_calls,mode_calls;
static s32 observed_mode;
static int touches_at_mode;
static RrLibInterruptDriverPrefix *driver_at_mode;
static void reset_a(void){reset_calls++;callback_touches+=1;cworld.active=&cworld.drivers[1];}
static void reset_b(void){reset_calls++;callback_touches+=2;cworld.active=&cworld.drivers[0];}
void movie_playback_mdec_reset(s32 mode){
 mode_calls++;observed_mode=mode;touches_at_mode=callback_touches;driver_at_mode=cworld.active;
}
static int modes(void){
 unsigned k,i;
 for(k=0;k<65536;k++){
  s32 mode;unsigned driver=(k>>3)&1;
  switch(k%8){
   case 0:mode=0;break;case 1:mode=1;break;case 2:mode=-1;break;
   case 3:mode=65536;break;case 4:mode=INT32_MIN;break;case 5:mode=INT32_MAX;break;
   default:{u32 bits=next();memcpy(&mode,&bits,4);break;}
  }
  for(i=0;i<8;i++){cworld.before[i]=next();cworld.after[i]=next();cworld.callbacks[i]=callback_c;}
  cworld.drivers[0]=(RrLibInterruptDriverPrefix){callback_a,exchange_a,exchange_b,reset_a};
  cworld.drivers[1]=(RrLibInterruptDriverPrefix){callback_b,exchange_b,exchange_a,reset_b};
  cworld.active=&cworld.drivers[driver];cworld.calls=next();cworld.channel=-1;
  cworld.last=callback_c;cworld.tag=0;cworld.mutate=0;
  memcpy(&cexpected,&cworld,sizeof(cworld));
  if(mode==0)cexpected.active=&cworld.drivers[1-driver];
  reset_calls=mode_calls=0;callback_touches=touches_at_mode=0;
  movie_playback_dec_dct_reset(mode);
  if(memcmp(&cworld,&cexpected,sizeof(cworld)) || mode_calls!=1 || observed_mode!=mode ||
     reset_calls!=(mode==0) || touches_at_mode!=(mode==0?(int)driver+1:0) ||
     driver_at_mode!=cexpected.active){fprintf(stderr,"mode case %u\n",k);return 1;}
 }
 puts("65536 native STR mode cases with UBSan passed");return 0;
}
int main(int argc,char **argv){
 if(argc>1 && argv[1][0]=='p')return table_publish();
 if(argc>1 && argv[1][0]=='m')return modes();
 if(argc>1 && argv[1][0]=='h')return header_read();
 if(argc>1 && argv[1][0]=='c')return callbacks();
 return table_copy();
}
'''
        (path / 'decode.c').write_text(source)
        cls.executable = path / 'decode'
        subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                        '-fno-sanitize-recover=all', '-I' + str(root / 'include'),
                        str(path / 'decode.c'), '-o', str(cls.executable)],
                       check=True, capture_output=True)

    def run_case(self, mode, expected):
        result = subprocess.run([str(self.executable), mode], check=True,
                                capture_output=True, text=True)
        self.assertIn(expected, result.stdout)

    def test_ordered_table_copy_and_overlapping_owners(self):
        self.run_case('tables', '65536 native STR table-copy cases')

    def test_ordered_table_publication_and_command_mutations(self):
        self.run_case('publish', '65536 native STR table-publish cases')

    def test_reset_and_mode_dispatch(self):
        self.run_case('modes', '65536 native STR mode cases')

    def test_header_low_halfword(self):
        self.run_case('header', '65536 native STR header cases')

    def test_typed_callback_and_actual_resident_dispatch(self):
        self.run_case('callbacks', '16384 native STR callback cases')


if __name__ == '__main__':
    unittest.main()
