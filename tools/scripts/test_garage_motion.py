"""Exhaustive garage controller movement, including neutral output preservation."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class GarageMotionTest(unittest.TestCase):
    def test_all_axes_and_controller_types(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/garage/func_8011D58C.c').read_text()
        body=body.replace('extern RrMenuItemState5C PlayerControllers[];','')
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
#include "rr/overlay662_garage.h"
#include "rr/tables.h"
static struct World {u32 guard[4]; RrMenuItemState5C rows[3]; RrGaragePoint out; u32 tail[4];} w,e;
#define PlayerControllers w.rows
static unsigned calls;
/* A0/2B BIOS memset boundary: verify arguments and execute its memory semantics. */
void *PsyqMemset(void *p,s32 value,u32 count){assert(value==0 && count==4);calls++;return memset(p,value,count);}
''' + body + r'''
int main(void){unsigned type,x,y,i;
 for(type=0;type<256;type++)for(x=0;x<256;x++)for(y=0;y<256;y++){
  int sx=x<128?(int)x:(int)x-256,sy=y<128?(int)y:(int)y-256;
  int dx=sx>=0?sx/2:-((-sx+1)/2),dy=sy;
  for(i=0;i<sizeof(w);i++)((u8*)&w)[i]=(u8)(i*37+x+y+type);
  w.rows[1].type01=type;w.rows[1].axis14=x;w.rows[1].axis15=sy;
  memcpy(&e,&w,sizeof(w));calls=0;
  if(type==18){
   if(dx < -3 || dx > 3)dx+=dx/2;
   if(dy < -7 || dy > 7)dy+=dy/2;
   if(dx || dy){e.out.x=dx;e.out.y=dy;}
  }
  assert((uintptr_t)&w.out>UINT32_MAX);
  garage_func_8011D58C(&w.out);
  assert(calls==1 && memcmp(&w,&e,sizeof(w))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
