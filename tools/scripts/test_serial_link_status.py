"""Unsigned threshold decisions and shared status/scene-cell widths."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay667StatusTest(unittest.TestCase):
    def test_word_arguments_and_canonical_storage_widths(self):
        root=Path(__file__).resolve().parents[2]
        bodies=[]
        for address in ['8011492C','80115058','80117EE8']:
            body=(root/f'src/overlays/PAL/serial_link/func_{address}.c').read_text()
            body=body.replace(' __asm__("$2")','')
            body=body.replace('__asm__("");','')
            body=body.replace('__asm__("" : "=r"(ret) : "0"(ret));','')
            bodies.append(body)
        source=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay667_status.h"
_Static_assert(__builtin_types_compatible_p(__typeof__(SceneId),u16),"scene cell is unsigned16");
static struct {u32 head;u16 before,marker,after,pad;s32 status;u32 tail;} world,snapshot;
#define SceneId world.marker
#define serial_link_D_80118EE4 world.status
''' + '\n'.join(bodies) + r'''
static u32 seed=0x1492C;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
int main(void){unsigned i,b;u32 calls=0;
 static const u32 elapsed[]={0,1,24999,25000,25001,65535,65536,0x7FFFFFFFU,0x80000000U,0xFFFFFFFFU};
 static const s32 outcome[]={1,1,1,0,0,0,0,0,0,0};
 for(i=0;i<10;i++){
  assert(serial_link_func_8011492C(2,elapsed[i])==outcome[i]);calls++;
  for(b=0;b<32;b++){s32 kind=word(2U^(1U<<b));assert(serial_link_func_8011492C(kind,elapsed[i])==1);calls++;}
 }
 for(i=0;i<65536;i++){
  u32 shifted=(i<<16)|i;unsigned j;
  assert(serial_link_func_8011492C(2,i)==(i<=24999));calls++;
  assert(serial_link_func_8011492C(2,0x80000000U|i)==0);calls++;
  assert(serial_link_func_8011492C(word(0x10002U),shifted)==1);calls++;
  for(j=0;j<sizeof(world);j++)((u8*)&world)[j]=(u8)next();
  world.marker=(u16)i;world.status=word(shifted);
  memcpy(&snapshot,&world,sizeof(world));
  assert(serial_link_func_80115058()==snapshot.status);
  assert(memcmp(&world,&snapshot,sizeof(world))==0);
  serial_link_func_80117EE8();assert(world.marker==315);
  world.marker=snapshot.marker;assert(memcmp(&world,&snapshot,sizeof(world))==0);
 }
 assert(calls==196938);return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
