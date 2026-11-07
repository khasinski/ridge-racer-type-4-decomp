"""Ordered matrix/color preparation using genuine matrix and input owners.

GTE multiply/install functions are typed simulated boundaries. The test observes
only initialized temporary data:32 source bytes and nine auxiliary halfwords.
It does not execute the GTE arithmetic or coprocessor register writes.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest


class Overlay665MatrixTest(unittest.TestCase):
    def test_matrix_capture_order_halfword_input_and_aliasing(self):
        root = Path(__file__).resolve().parents[2]
        body = (root / 'src/overlays/PAL/options/func_80116358.c').read_text()
        source = r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
#include "rr/overlay665_matrix.h"
typedef union {
 RrPreviewMatrixWorkspace matrix;
 RrPreviewLightInput36 input;
} SharedInput;
struct Prepared {
 u32 basis[8],source[8];u16 colors[9];
};
static struct World {
 u32 guard[4];RrPreviewMatrixWorkspace source;
 struct {u16 prefix;RrPreviewLightInput36 input;} halfAligned;
 SharedInput basis,overlap;u32 tail[4];
 struct Prepared captured;u32 installed[5];unsigned calls;
} actual,expected,initial,*w;
#define LightMatrix w->basis.matrix
static unsigned scenario,mode;
static s32 *multiply(s32 *basis,u16 *matrix) {
 RrPreviewMatrixWorkspace *pair=(RrPreviewMatrixWorkspace *)matrix;
 unsigned i;u32 b[5],m[5];
 assert(w->calls++==0);assert(basis==(s32*)w->basis.matrix.words);
 assert((uintptr_t)basis>UINT32_MAX && (uintptr_t)matrix>UINT32_MAX);
 assert(((uintptr_t)matrix&3)==0);
 memcpy(w->captured.basis,basis,32);memcpy(w->captured.source,matrix,32);
 for(i=0;i<9;i++)w->captured.colors[i]=pair[1].halves[i];
 /* Simulate only the actual helper's20-byte output footprint. */
 memcpy(b,basis,20);memcpy(m,matrix,20);
 for(i=0;i<5;i++)pair[0].words[i]=(b[i]^m[i])+(scenario*0x9E3779B9U+i*0x1234567U);
 return (s32*)matrix;
}
s32 *MulMatrix2(s32 *basis,u16 *matrix){return multiply(basis,matrix);}
void SetLightMatrix(s32 *matrix) {
 assert(w->calls++==1);assert(((uintptr_t)matrix&3)==0);
 memcpy(w->installed,matrix,20);
}
'''+body+r'''
static const RrPreviewMatrixWorkspace *source_matrix(void) {
 if(mode==1 || mode==3)return &w->basis.matrix;
 if(mode==4)return &w->overlap.matrix;
 return &w->source;
}
static const RrPreviewLightInput36 *input_data(void) {
 if(mode==2 || mode==3)return &w->basis.input;
 if(mode==4)return &w->overlap.input;
 return &w->halfAligned.input;
}
static void reference(void) {
 RrPreviewMatrixWorkspace pair[2];unsigned column,row;
 const RrPreviewLightInput36 *input=input_data();
 memcpy(pair[0].words,source_matrix()->words,32);
 for(column=0;column<3;column++) {
  /* Preserve interleaved source reads and basis writes before RGB capture. */
  for(row=0;row<3;row++) {
   u16 value=input->halves[6+column*4+row];
   w->basis.matrix.halves[column*3+row]=value;
  }
  for(row=0;row<3;row++)pair[1].halves[row*3+column]=(u16)((unsigned)input->bytes[column*4+row]*16);
 }
 (void)multiply((s32*)w->basis.matrix.words,pair[0].halves);
 SetLightMatrix((s32*)pair[0].words);
}
static u32 rng=0x16358;
static u32 next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
int main(void) {
 unsigned i;
 _Static_assert(_Alignof(RrPreviewMatrixWorkspace)==4,"word-aligned matrix");
 _Static_assert(_Alignof(RrPreviewLightInput36)==2,"halfword-aligned input");
 _Static_assert(offsetof(struct World,halfAligned.input)%4==2,"actual unaligned-for-word input owner");
 for(scenario=0;scenario<65536;scenario++) {
  memset(&initial,0,sizeof(initial));
  for(i=0;i<offsetof(struct World,captured);i++)((u8*)&initial)[i]=(u8)next();
  for(mode=0;mode<5;mode++) {
   actual=initial;expected=initial;
   w=&actual;options_set_preview_light_matrix(source_matrix(),input_data());
   w=&expected;reference();
   assert(actual.calls==2 && memcmp(&actual,&expected,sizeof(actual))==0);
  }
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            c = Path(temp) / 'matrix.c'
            exe = Path(temp) / 'matrix'
            c.write_text(source)
            subprocess.run(['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                            '-fsanitize=undefined', '-fno-sanitize-recover=all',
                            '-I', str(root / 'include'), str(c), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
