"""Exhaustive byte classifications and signed-halfword truth values over real rows."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest


class OverlayClassifyTest(unittest.TestCase):
    def test_classification_and_read_only_rows(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/options/func_8011629C.c').read_text()
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(t)==n,"size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(offsetof(t,f)==n,"offset")
#include "rr/menu_pulse.h"
static struct World {u32 before[8];RrMenuItemState5C rows[16];u32 after[8];} world,initial;
#define PlayerControllers (world.rows+8)
''' + body + r'''
static u32 seed=0x1629c;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s16 half(u32 raw){u16 bits=raw;s16 value;memcpy(&value,&bits,2);return value;}
static int model(unsigned type,s16 kind,unsigned flag){
 if(type==0x23)return 2;
 if(type==0xe3)return 3;
 if(type==0x41 || type==0x73){
  if(kind!=0)return flag==0?1:3;
  return type==0x73?1:0;
 }
 return -1;
}
static void check(unsigned k,unsigned type,s16 kind,unsigned flag){
 s32 index=(s32)(k&15)-8;RrMenuItemState5C *row=&world.rows[index+8];s32 value;
 row->type01=type;row->kind1C=kind;row->flag40=flag;
 memcpy(&initial,&world,sizeof(world));
 value=options_get_controller_kind(index);
 assert(value==model(type,kind,flag));assert(memcmp(&world,&initial,sizeof(world))==0);
 assert((uintptr_t)row>UINT32_MAX);
}
int main(void){unsigned i,t,f,k,m;const s16 kinds[]={0,1,-1};
 for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
 for(m=0;m<3;m++)for(t=0;t<256;t++)for(f=0;f<256;f++)check((t<<8)|f,t,kinds[m],f);
 for(k=0;k<65536;k++)for(t=0;t<2;t++)for(f=0;f<2;f++)check(k, t?0x73:0x41,half(k),f?255:0);
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            path = Path(temp)
            (path / 'test.c').write_text(source)
            subprocess.run(['cc', '-std=gnu11', '-O2', '-fsanitize=undefined',
                            '-fno-sanitize-recover=all', '-I', str(root / 'include'),
                            str(path / 'test.c'), '-o', str(path / 'test')], check=True)
            subprocess.run([str(path / 'test')], check=True)


if __name__ == '__main__':
    unittest.main()
