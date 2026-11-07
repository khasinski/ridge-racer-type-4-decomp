"""Execute the actual directory initializer with CD/BIOS boundary stubs."""
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class CdDirectoryInitTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = TemporaryDirectory()
        cls.addClassCleanup(cls.temporary.cleanup)
        header = (ROOT/'include/rr/cd.h').read_text()
        header = re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n', '', header, flags=re.M)
        body = (ROOT/'tools/scripts/fixtures/views/func_80012290.c').read_text()
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$\d+"\)', '', body)
        # Only the physical PAL diagnostic page is mapped to a native object.
        assert body.count('(u8 *)0x80010000U') == 1
        body = body.replace('(u8 *)0x80010000U', 'message_storage')
        prelude = r'''
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
''' + header + r'''
static u8 message_storage[256];
''' + body + r'''
const char NowLoadingFmt[]="start", R4BinPath[]="R4.BIN";
const char StrFileNotFoundFmt[]="missing", StrReadSectorFmt[]="done";
u32 *CdDirectoryBuffer;
RrCdFileEntry CdFileTable[952];
static u32 directory_a[1024], directory_b[1024], directory_c[1024];
static int found, scenario, reads, controls, polls, logs, conversions;
static int statuses[16], status_count, status_index, expected_count;
static jmp_buf exited;
static const RrCdLocation location={{0x12,0x34,0x56,0x78}};
static const int base=20000;

s32 CdSearchFile(SearchResult *file, const char *name) {
    assert(logs==1 && name==R4BinPath);
    if(found){file->location=location;file->size=1234;memset(file->name,0,sizeof(file->name));}
    return found;
}
void PsyqExit(s32 code) {assert(code==1 && !found && logs==2);longjmp(exited,1);}
void DebugPrintStub(const char *format, ...) {
    va_list ap;va_start(ap,format);
    if(logs==0) {
        assert(format==NowLoadingFmt);
        assert(va_arg(ap,const char *)==R4BinPath);
        assert(va_arg(ap,u32 *)==directory_a);
    } else if(!found) {
        assert(format==StrFileNotFoundFmt);
        assert(va_arg(ap,const char *)==R4BinPath);
    } else {
        assert(format==(const char *)(message_storage+0x68));
        assert(va_arg(ap,int)==2);
        assert(status_index==status_count);
        if(scenario==2)CdDirectoryBuffer=directory_b;
    }
    logs++;va_end(ap);
}
s32 CdControl(s32 command,const u8 *parameters,u8 *result) {
    assert(command==2 && result==0 && memcmp(parameters,&location,4)==0);
    assert(controls==reads);controls++;
    if(scenario==1 && controls==2)CdDirectoryBuffer=directory_b;
    return -1; /* Its return value is deliberately ignored. */
}
s32 CdRead(s32 count,void *buffer,s32 mode) {
    assert(count==2 && mode==0x80 && buffer==CdDirectoryBuffer);
    assert(controls==reads+1);reads++;return -123;
}
s32 CdReadSync(s32 mode,void *result) {
    assert(mode==1 && !result && status_index<status_count);
    polls++;return statuses[status_index++];
}
s32 CdPosToInt(const RrCdLocation *value) {
    assert(logs==2 && !memcmp(value,&location,4));
    if(scenario==3)CdDirectoryBuffer=directory_b;
    return base;
}
u8 *CdIntToPos(s32 sector,u8 *out) {
    assert(conversions<expected_count);
    assert(out==CdFileTable[conversions].location.c);
    assert(sector==base+2+conversions*3);
    out[0]=(u8)sector;out[1]=(u8)(sector>>8);out[2]=0xAB;out[3]=0xCD;
    conversions++;
    /* Once traversal starts, directory storage must remain captured. */
    if(scenario==4)CdDirectoryBuffer=directory_c;
    return out;
}
static void reset(int count,int mode) {
    found=1;scenario=mode;reads=controls=polls=logs=conversions=0;
    status_count=1;status_index=0;statuses[0]=0;expected_count=count;
    memset(CdFileTable,0xA5,sizeof(CdFileTable));
    memset(directory_a,0x51,sizeof(directory_a));
    memset(directory_b,0x62,sizeof(directory_b));
    memset(directory_c,0x73,sizeof(directory_c));
    u32 *selected=(mode==1||mode==2||mode==3)?directory_b:directory_a;
    selected[0]=count;
    for(int i=0;i<=count;i++)selected[i+1]=2+i*3;
    CdDirectoryBuffer=directory_a;
}
static void verify(int count) {
    assert(conversions==count && logs==2 && status_index==status_count);
    for(int i=0;i<count;i++) {
        int sector=base+2+i*3;
        assert(CdFileTable[i].location.c[0]==(u8)sector);
        assert(CdFileTable[i].location.c[1]==(u8)(sector>>8));
        assert(CdFileTable[i].location.c[2]==0xAB);
        assert(CdFileTable[i].location.c[3]==0xCD);
        assert(CdFileTable[i].sectors==3);
    }
    const u8 *bytes=(const u8 *)CdFileTable;
    for(unsigned i=count*sizeof(RrCdFileEntry);i<sizeof(CdFileTable);i++)assert(bytes[i]==0xA5);
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(!strcmp(argv[1],"directories")) {
        for(int count=0;count<=950;count++) {
            reset(count,0);LoadR4Directory();verify(count);
            assert(reads==1 && controls==1 && polls==1);
        }
    } else if(!strcmp(argv[1],"retries")) {
        for(int mode=0;mode<=4;mode++) {
            reset(7,mode);
            int sequence[]={3,1,-1,2,-7,1,0};
            memcpy(statuses,sequence,sizeof(sequence));status_count=7;
            LoadR4Directory();verify(7);
            assert(reads==3 && controls==3 && polls==7);
        }
    } else {
        reset(4,0);found=0;
        if(!setjmp(exited)){LoadR4Directory();assert(0);}
        assert(logs==2 && reads==0 && controls==0 && polls==0 && conversions==0);
        const u8 *bytes=(const u8 *)CdFileTable;
        for(unsigned i=0;i<sizeof(CdFileTable);i++)assert(bytes[i]==0xA5);
    }
    return 0;
}
'''
        directory = Path(cls.temporary.name)
        source = directory/'directory.c'
        source.write_text(prelude)
        cls.executable = directory/'directory'
        result = subprocess.run(['cc','-std=c99','-O2','-Wall','-Wextra','-Werror','-Wno-error=deprecated-non-prototype',
                                 '-fsanitize=undefined','-I',str(ROOT/'include'),
                                 str(source),'-o',str(cls.executable)],capture_output=True,text=True)
        if result.returncode:
            raise AssertionError(result.stderr)

    def run_case(self, case):
        result = subprocess.run([str(self.executable),case],capture_output=True,text=True)
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertEqual(result.stderr,'')

    def test_directory_sizes_and_guards(self): self.run_case('directories')
    def test_retries_and_live_buffer_changes(self): self.run_case('retries')
    def test_missing_file_exits_before_reads(self): self.run_case('missing')


if __name__ == '__main__':
    unittest.main()
