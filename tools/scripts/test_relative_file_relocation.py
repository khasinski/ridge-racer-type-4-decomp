"""Actual file relocator, native pointer widths, wrapped counts and UBSan."""
from pathlib import Path
import re
import subprocess
import tempfile
import unittest


class RelativeFileRelocationTest(unittest.TestCase):
    def test_native_owner_and_wrapped_counts(self):
        root=Path(__file__).resolve().parents[2]
        body=(root/'tools/scripts/fixtures/views/func_8007026C.c').read_text().replace('#include "rr/relative_file.h"','')
        body=re.sub(r'\s+(?:asm|__asm__)\("\$\d+"\)', '', body)
        body=re.sub(r'__asm__\(""[^;]*;','',body)
        names=['RelFileSection00','RelFileSection01','RelFileSection02','RelFileOffsetPtr3','RelFileOffsetPtr4','RelFileSection05','RelFileOffsetPtr6','RelFileOffsetPtr7','RelFileOffsetPtr8','RelFileOffsetPtr9','RelFileOffsetPtr10','RelFileSection11','RelFileOffsetPtr12','RelFileOffsetPtr13','RelFileOffsetPtr14','RelFileSection15']
        prefix=r'''
        #include <stdint.h>
        #include <stddef.h>
        #include <stdio.h>
        #include <string.h>
        #include <assert.h>
        #include "rr/layout.h"
        #undef RR_ASSERT_SIZE
        #undef RR_ASSERT_OFFSET
        #define RR_ASSERT_SIZE(t,n)
        #define RR_ASSERT_OFFSET(t,f,n)
        #include "rr/relative_file.h"
        static struct World {
         u32 before[16];
         struct File {
          RrRelativeHeader header;
          RrRelativeSlot entries[128];
          u8 payload[128];
         } file;
         RrRelativeHeader *root;
         s32 count;
         RrRelativeSlot *entries;
         u8 *first,*second,*outputs[16];
         u32 after[16];
        } world,initial,expected;
        #define RelativeFileBase world.root
        #define RelativeFileCount world.count
        #define SegmentViewTables world.entries
        #define RelFileSlot1Ptr world.first
        #define RelFileViewRecords world.second
        '''
        prefix+=''.join('#define '+n+' world.outputs['+str(i)+']\n' for i,n in enumerate(names))
        source=prefix+body+r'''
        static u32 seed=0x7026c;
        static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
        static u32 relative(void){
         s32 v=(s32)(next()%(sizeof(world.before)+sizeof(world.file)))-(s32)sizeof(world.before);
         u32 bits;memcpy(&bits,&v,4);return bits;
        }
        static u8 *address(u32 bits){
         s32 offset;memcpy(&offset,&bits,4);
         return (u8*)&world+offsetof(struct World,file.header)+offset;
        }
        static void reference(void){
         RrRelativeHeader *header=&world.file.header;
         u32 count=header->count,n=count*8U,i;
         u8 *a=address(header->first[0].relative);
         u8 *b=address(header->first[1].relative);
         world.root=header;memcpy(&world.count,&count,4);
         header->first[0].absolute=a;
         header->first[1].absolute=b;
         world.first=b;
         header->first[2].absolute=address(header->first[2].relative);
         world.second=header->first[2].absolute;
         for(i=0;i<16;i++) world.outputs[i]=address(header->offsets[i]);
         world.entries=world.file.entries;
         for(i=0;i<n;i++) world.file.entries[i].absolute=address(world.file.entries[i].relative);
        }
        int main(void){
         u32 k,i;
         assert(sizeof(void*)==8 && sizeof(unsigned long)==sizeof(void*));
         assert((uintptr_t)&world>UINT32_MAX);
         assert(offsetof(struct File,entries)==sizeof(RrRelativeHeader));
         for(k=0;k<131072;k++){
          u32 records=k%17;
          for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();
          world.file.header.count=records|((k%8)<<29);
          for(i=0;i<3;i++) world.file.header.first[i].relative=relative();
          for(i=0;i<16;i++) world.file.header.offsets[i]=relative();
          for(i=0;i<128;i++)world.file.entries[i].relative=relative();
          world.root=&world.file.header;
          world.count=0xdeadbeef;
          world.entries=world.file.entries+128;
          world.first=world.second=world.file.payload;
          for(i=0;i<16;i++)world.outputs[i]=world.file.payload+i;
          memcpy(&initial,&world,sizeof(world));
          reference();memcpy(&expected,&world,sizeof(world));
          memcpy(&world,&initial,sizeof(world));
          RelocateRelativeFile(&world.file.header);
          if(memcmp(&world,&expected,sizeof(world))){
           fprintf(stderr,"relocation mismatch case %u\n",k);return 1;
          }
         }
         puts("131072 native relocation cases passed with UBSan");return 0;
        }
        '''
        with tempfile.TemporaryDirectory() as tmp:
         native=Path(tmp)/'relocate.c'
         native.write_text(source)
         exe=Path(tmp)/'relocate'
         subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I'+str(root/'include'),str(native),'-o',str(exe)],check=True)
         subprocess.run([str(exe)],check=True)


if __name__ == "__main__":
    unittest.main()
