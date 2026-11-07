"""Preview renderer: real preparation/scaling; simulated GTE/drawing services."""
from pathlib import Path
from tempfile import TemporaryDirectory
import subprocess
import unittest

class Overlay665RenderTest(unittest.TestCase):
    def test_modes_and_service_mutations(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'src/overlays/PAL/665/func_8011643C.c').read_text()
        prepare=(root/'src/overlays/PAL/665/func_80116358.c').read_text()
        scale=(root/'tools/scripts/fixtures/views/func_8002E828.c').read_text()
        source=r'''
        #include <assert.h>
        #include <stddef.h>
        #include <stdint.h>
        #include <string.h>
        #include "rr/layout.h"
        #undef RR_ASSERT_SIZE
        #undef RR_ASSERT_OFFSET
        #define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"PAL size")
        #define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"PAL offset")
        #include "rr/overlay665_render.h"
        struct State {u32 guard[4];RrOverlay665VisualState visual;RrPreviewRenderScratch scratch;
         RrPreviewMatrixWorkspace basis;s32 meshes;u32 tail[4];};
        struct Event {s32 kind,args[3];u32 hash,first[8],second[8];};
        static struct World {struct State s;unsigned calls;struct Event events[64];} actual,expected,initial,*w;
        static unsigned scenario,mutation;
        static u8 pixels[4][256];static u32 heads[4][32],programs[10][6];
        static const u32 *table[10];
        #define RR_PREVIEW_RENDER_SCRATCH (&w->s.scratch)
        #define ovl665_D_80119768 w->s.visual
        #define ModelBankFormat w->s.meshes
        #define LightMatrix w->s.basis
        static const s32 kinds[]={INT32_MIN,-1,0,1,2,3,4,INT32_MAX};
        static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
        static s16 half(u32 bits){u16 v=(u16)bits;s16 s;memcpy(&s,&v,2);return s;}
        static void effect(unsigned n) {
         unsigned i;u32 v=scenario*0x9E3779B9U+n*47;
         for(i=0;i<sizeof(w->s.visual);i++)((u8*)&w->s.visual)[i]=(u8)(v+i*73);
         w->s.visual.kind18=kinds[(scenario+n)%8];
         w->s.visual.layout0C=kinds[(scenario+n*3+1)%8];
         w->s.visual.detail10=kinds[(scenario+n*5+2)%8];
         w->s.meshes=(s32)((scenario+n)%14)-2;
         w->s.scratch.projection.pointers.cursor=pixels[(scenario+n)%4];
         w->s.scratch.projection.pointers.order=heads[(scenario+n+1)%4];
         w->s.scratch.commands50=table;w->s.scratch.primitive74=programs[(scenario+n)%10];
         w->s.scratch.projection.word08=word(v);w->s.scratch.projection.word0C=word(v+1);
         w->s.scratch.projection.word10=word(v+2);w->s.scratch.projection.word14=word(v+3);
         w->s.scratch.projection.word18=word(v+4);w->s.scratch.projection.word1C=word(v+5);
         w->s.scratch.projection.word20=word(v+6);
         w->s.scratch.depth60=word(v^0xA5A5A5A5U);w->s.scratch.flags64=word(v+7);
         w->s.scratch.count54=(s32)((scenario+n)%10);w->s.scratch.format58=word(v+8);
         w->s.scratch.counter78=word(v+9);
         for(i=0;i<8;i++){w->s.scratch.matrix28.words[i]=v+i*0x1234567U;w->s.basis.words[i]=v+i*0x7654321U;}
         for(i=0;i<14;i++)w->s.scratch.tail7C.projection.words[i]=word(v+i*59);
        }
        static struct Event *event(s32 kind,s32 a,s32 b,s32 c,const void *first,const void *second) {
         unsigned i,n=w->calls;u32 hash=2166136261U;struct Event *e;
         assert(n<64);e=&w->events[w->calls++];e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;
         for(i=0;i<sizeof(w->s);i++)hash=(hash^((u8*)&w->s)[i])*16777619U;e->hash=hash;
         if(first)memcpy(e->first,first,32);if(second)memcpy(e->second,second,32);
         if(mutation)effect(n);return e;
        }
        void BuildCameraMatrices(void) {
         assert(w->calls==0 && w->s.scratch.depth60==3);
         assert(w->s.scratch.projection.word08==0 && w->s.scratch.projection.word0C==0 && w->s.scratch.projection.word10==0);
         assert(w->s.scratch.projection.word18==0 && w->s.scratch.projection.word1C==0 && w->s.scratch.projection.word20==0);
         (void)event(0,0,0,0,0,0);
        }
        void SetupViewport(s32 mode){assert(mode==6);(void)event(1,mode,0,0,0,0);}
        void SetGeomOffset(s32 x,s32 y){(void)event(2,x,y,0,0,0);}
        void SetupObjectLightMatrices(char *view,char *lighting,RrMatrixWords32 *first,RrMatrixWords32 *second) {
         unsigned i,n=w->calls;
         assert(view==(char*)&w->s.visual.view74 && lighting==(char*)&w->s.visual.lighting4C);
         assert((uintptr_t)first>UINT32_MAX && (uintptr_t)second>UINT32_MAX && first!=second);
         (void)event(3,0,0,0,0,0);
         for(i=0;i<8;i++){first->w[i]=word(scenario*0x1234567U+n*31+i*17);second->w[i]=word(scenario*0x7654321U+n*37+i*19);}
        }
        u32 *ScaleMatrix(u32 *matrix,s32 scales[3]) {
         unsigned i;u32 captured[3];memcpy(captured,scales,12);
         (void)event(4,scales[0],scales[1],scales[2],matrix,0);
         for(i=0;i<5;i++)matrix[i]=(matrix[i]^captured[i%3])+scenario+i;
         return matrix;
        }
        void SetGeomScreen(s32 depth){(void)event(5,depth,0,0,0,0);}
        void SelectModelBank(s32 object){assert(object==8);(void)event(6,object,0,0,0,0);}
        s32 *MulMatrix2(s32 *basis,u16 *matrix) {
         unsigned i;u32 b[8],m[8];memcpy(b,basis,32);memcpy(m,matrix,32);
         assert(((uintptr_t)basis&3)==0 && ((uintptr_t)matrix&3)==0);
         (void)event(7,basis==(s32*)w->s.basis.words,0,0,basis,matrix);
         for(i=0;i<5;i++){u32 v=(b[i]^m[i])+scenario+i*0x11111111U;memcpy(matrix+i*2,&v,4);}
         return (s32*)matrix;
        }
        void SetLightMatrix(s32 *matrix){(void)event(8,0,0,0,matrix,0);}
        void SetGteObjectTransform(s16 *output,s16 *view,s32 *matrix) {
         unsigned i;u32 m[8];memcpy(m,matrix,32);
         assert(output==w->s.scratch.tail7C.projection.halves);
         assert(view==w->s.visual.view74.position00.halves);
         (void)event(9,view[0],view[2],view[4],matrix,0);
         for(i=0;i<3;i++) {
          w->s.scratch.tail7C.projection.fields.relative00[i]=half(m[i]);
          w->s.scratch.tail7C.projection.fields.transformed08[i]=word(m[i]^scenario);
          w->s.scratch.tail7C.projection.fields.saved2C[i]=word(m[i]^scenario);
         }
        }
        void BuildRotMatrixZ(s16 *matrix,s32 angle) {
         unsigned i;(void)event(10,angle,0,0,matrix,0);
         for(i=0;i<9;i++)matrix[i]=half((u32)angle+scenario+i*13);
        }
        void BuildRotMatrixX(void *matrix,s32 angle) {
         unsigned i;u16 *halves=matrix;(void)event(11,angle,0,0,matrix,0);
         for(i=0;i<9;i++)halves[i]=(u16)((u32)angle+scenario+i*17);
        }
        void SubmitModelTwoTri(RrPreviewRenderScratch *scratch,s32 object,s32 blend,s32 depth) {
         assert(scratch==&w->s.scratch && object>=0 && object<=8 && blend==0 && depth==0);
         assert(scratch->counter78==0 && (uintptr_t)scratch->commands50>UINT32_MAX);
         (void)event(12,object,blend,depth,0,0);
        }
        '''+scale+prepare+body+r'''
        static void setup(RrPreviewMatrixWorkspace *first,RrPreviewMatrixWorkspace *second) {
         s32 scales[3];unsigned i;
         for(i=0;i<3;i++)scales[i]=w->s.visual.scale3C;
         w->s.visual.view74.rotation10=(s16)w->s.visual.rotation44;
         w->s.visual.view74.position00.words[2]=w->s.visual.position48;
         SetupObjectLightMatrices((char*)&w->s.visual.view74,(char*)&w->s.visual.lighting4C,&first->matrix,&second->matrix);
         (void)ScaleMatrix(first->words,scales);SetGeomScreen(w->s.visual.depth40);
        }
        static void light(RrPreviewMatrixWorkspace *matrix){ovl665_func_80116358(matrix,&w->s.visual.lighting4C.matrix);}
        static void project(RrPreviewMatrixWorkspace *matrix){SetGteObjectTransform(w->s.scratch.tail7C.projection.halves,w->s.visual.view74.position00.halves,(s32*)matrix->words);}
        static void draw(s32 threshold,s32 object) {
         s32 chosen=w->s.meshes>=threshold?object:1;
         w->s.scratch.counter78=0;SubmitModelTwoTri(&w->s.scratch,chosen,0,0);
        }
        static void reference(void) {
         RrPreviewMatrixWorkspace first,second;s32 saved=w->s.scratch.depth60,kind;
         w->s.scratch.depth60=3;
         w->s.scratch.projection.word10=0;w->s.scratch.projection.word0C=0;w->s.scratch.projection.word08=0;
         w->s.scratch.projection.word20=0;w->s.scratch.projection.word1C=0;w->s.scratch.projection.word18=0;
         BuildCameraMatrices();SetupViewport(6);
         SetGeomOffset((s16)w->s.visual.centerX38,(s16)w->s.visual.centerY3A);
         func_8002E828(2,w->s.scratch.matrix28.halves);kind=w->s.visual.kind18;
         if(kind>=0 && kind<=3) {
          setup(&first,&second);
          if(kind!=2)light(&first);
          SelectModelBank(8);
          if(kind==0 || kind==1) {
           s32 object;project(&first);w->s.scratch.counter78=0;
           object=kind==0?1:w->s.meshes<1;
           SubmitModelTwoTri(&w->s.scratch,object,0,0);
          } else if(kind==2) {
           BuildRotMatrixX(second.halves,w->s.visual.view74.rotation18);
           (void)MulMatrix2((s32*)first.words,second.halves);light(&second);project(&second);draw(3,2);
           if(w->s.visual.layout0C>=2 && w->s.visual.detail10>=2){project(&second);draw(7,6);}
           BuildRotMatrixX(second.halves,w->s.visual.view74.rotation20);
           (void)MulMatrix2((s32*)first.words,second.halves);light(&second);project(&second);draw(4,3);
           if(w->s.visual.layout0C>=2 && w->s.visual.detail10>=2){project(&second);draw(8,7);}
          } else {
           project(&first);draw(5,4);
           BuildRotMatrixZ((s16*)second.halves,(s32)w->s.visual.view74.rotation1C+1024);
           (void)MulMatrix2((s32*)first.words,second.halves);light(&second);project(&second);draw(6,5);
           if(w->s.visual.layout0C>=2 && w->s.visual.detail10>=2 && w->s.visual.detail10<4){project(&second);draw(9,8);}
          }
         }
         w->s.scratch.depth60=saved;
        }
        int main(void) {
         unsigned i,n;
         for(i=0;i<10;i++)table[i]=programs[i];
         for(scenario=0;scenario<65536;scenario++)for(mutation=0;mutation<2;mutation++) {
          memset(&initial,0,sizeof(initial));w=&initial;effect(7);
          for(i=0;i<4;i++){w->s.guard[i]=0x12345678U+i;w->s.tail[i]=0x87654321U-i;}
          w->s.visual.kind18=kinds[scenario%8];
          w->s.scratch.depth60=word(scenario*0x98765431U);
          actual=initial;expected=initial;w=&actual;ovl665_func_8011643C();w=&expected;reference();
          assert(memcmp(&actual,&expected,sizeof(actual))==0);
          for(n=0;n<4;n++){assert(actual.s.guard[n]==initial.s.guard[n]);assert(actual.s.tail[n]==initial.s.tail[n]);}
         }
         return 0;
        }
        '''
        with TemporaryDirectory() as tmp:
         p=Path(tmp);(p/'render.c').write_text(source)
         subprocess.run(['cc','-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'render.c'),'-o',str(p/'render')],check=True)
         subprocess.run([str(p/'render')],check=True)
        print('131072 renderer cases with real matrix preparation and scale helper passed under UBSan')

if __name__ == "__main__":
    unittest.main()
