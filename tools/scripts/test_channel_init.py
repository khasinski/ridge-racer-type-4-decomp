"""Actual channel constructor, real callback registrar, native pointer and alias cases."""
from pathlib import Path
import re,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[2]
def native_source(text):
    text=re.sub(r'^#include[^\n]*\n','',text,flags=re.M)
    text=re.sub(r'\s+__asm__\("\$\d+"\)','',text)
    return re.sub(r'__asm__\s*(?:volatile\s*)?\("".*?\);','',text)
def header(name):
    text=(ROOT/'include/rr'/name).read_text()
    return re.sub(r'^#include[^\n]*\n|^RR_ASSERT_[^\n]*\n','',text,flags=re.M)
body=native_source((ROOT/'tools/scripts/fixtures/views/func_80099CD4.c').read_text())
registrar=native_source((ROOT/'tools/scripts/fixtures/views/func_8009A334.c').read_text()).replace('_padInitDirSeq','native_registrar')
registrar,n=re.subn(r'\(u8 \*\)0x800B0000U','(callback_page.bytes+0x4380)',registrar)
assert n==1
code=r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "types.h"
'''+''.join(header(n) for n in ['channels.h','channel_advance.h','bios_devices.h','channel_init.h'])+r'''
typedef struct {
 u8 prefix[32];RrChannelRecordF0 records[2];u8 inputs[2][16];
 u8 buffers_a[70],buffers_b[70],suffix[32];
} World;
static World actual,expected;
static RrChannelRecordF0 external_record;
static union {void *align;u8 bytes[0x4388];} callback_page;
#define PadChannelRecordArray actual.records
#define PadPacketBuffers actual.buffers_a
#define PadTxBuffers actual.buffers_b
#define PadPacketDoneCallback (*(RrChannelUpdateCallback *)(callback_page.bytes+8))
RrChannelRecordF0 *PadChannelRecords;
RrChannelErrorCallback PadAdvanceChannelFn;
RrChannelUpdateCallback PadResetChannelFn,PadAllocGatesFn;
u32 (*PadTxByteCallback)(const RrChannelTransferView *);
s32 (*PadChannelToPortFn)(void *);
RrChannelRecordCallback PadPortToChannelPtr;
void (*PadSaveCommandCallback)(RrChannelTransferView *);
RrChannelStatusCallback PadChannelDispatchPtr,PadChannelBusyFn;
s32 PadComActive,PadMultitapEnabled;
static int stage,seed;
void _padAdvanceChannel(s32 n) {(void)n;}
void _padResetChannel(void *p) {(void)p;}
u32 _padGetTxByte(const RrChannelTransferView *p) {(void)p;return 0;}
void _padAllocGates(void *p) {(void)p;}
s32 _padChannelToPort(void *p) {(void)p;return 0;}
void *_padPortToChannel(s32 n) {(void)n;return 0;}
void _padSaveCommand(RrChannelTransferView *p) {(void)p;}
s32 _padChannelDispatch(void *p) {(void)p;return 0;}
s32 _padChannelBusy(void *p) {(void)p;return 0;}
void _padPacketDone(void *p) {(void)p;}
'''+registrar+r'''
static void check_callbacks(void) {
 assert(PadAdvanceChannelFn==_padAdvanceChannel && PadResetChannelFn==_padResetChannel);
 assert(PadTxByteCallback==_padGetTxByte && PadAllocGatesFn==_padAllocGates);
 assert(PadChannelToPortFn==_padChannelToPort && PadPortToChannelPtr==_padPortToChannel);
 assert(PadSaveCommandCallback==_padSaveCommand);
 assert(PadChannelDispatchPtr==_padChannelDispatch && PadChannelBusyFn==_padChannelBusy && PadPacketDoneCallback==_padPacketDone);
}
void _padInitDirSeq(void) {
 assert(stage==0 && PadComActive==0 && PadMultitapEnabled==0);
 native_registrar();stage=1;PadComActive=-17;PadMultitapEnabled=seed;
}
void PsyqBzero(void *p,s32 length) {
 assert(stage==1 && p==actual.records && length==sizeof(actual.records));
 assert(PadChannelRecords==actual.records && PadComActive==-17 && PadMultitapEnabled==seed);
 check_callbacks();memset(p,0,length);
 actual.records[0].responseE8=seed;actual.records[1].responseE8=seed+1;
 /* The local arena pointer must survive an opaque helper replacing this global. */
 PadChannelRecords=&external_record;stage=2;
}
void _padSetVsyncParam(void) {
 assert(stage==2 && PadComActive==-17 && PadMultitapEnabled==seed);
 assert(!memcmp(&actual,&expected,sizeof(actual)));
 check_callbacks();PadComActive=-42;stage=3;
}
'''+body+r'''
static u8 *mapped(u8 *pointer) {
 size_t offset=(size_t)(pointer-(u8 *)&actual);
 assert(offset+1<sizeof(actual));return (u8 *)&expected+offset;
}
static u8 *input(int which) {
 switch(which) {
 case 0:return actual.inputs[0];case 1:return actual.inputs[1];
 case 2:return actual.inputs[0]+4;
 case 3:return actual.records[0].status.payload5D;
 case 4:return actual.records[1].status.payload5D;
 case 5:return (u8 *)&actual.records[0].status.reset0C;
 case 6:return (u8 *)&actual.records[1].status.reset0C;
 default:return actual.buffers_a+12;
 }
}
static void reference(u8 *first,u8 *second) {
 int i,j;
 memset(expected.records,0,sizeof(expected.records));
 expected.records[0].responseE8=seed;expected.records[1].responseE8=seed+1;
 expected.records[0].status.input30=first;expected.records[1].status.input30=second;
 for(i=0;i<2;i++) {
  RrChannelStatusView *r=&expected.records[i].status;
  r->reset0C=0;r->owner10=&actual.records[i].status;
  *mapped(r->input30)=255;*mapped(r->input30+1)=0;
  r->packet=actual.buffers_a+i*35;r->buffer40=actual.buffers_b+i*35;
  for(j=0;j<6;j++)r->payload5D[j]=255;
 }
}
int main(void) {
 int a,b,n;unsigned j;u8 *first,*second;
 for(n=0;n<256;n++)for(a=0;a<8;a++)for(b=0;b<8;b++) {
  seed=n;stage=0;
  for(j=0;j<sizeof(actual);j++)((u8 *)&actual)[j]=(u8)(j*17+n);
  memset(&external_record,0xB7,sizeof(external_record));
  memset(&callback_page,0xA5,sizeof(callback_page));
  memcpy(&expected,&actual,sizeof(actual));first=input(a);second=input(b);
  reference(first,second);
  PadComActive=123;PadMultitapEnabled=456;PadChannelRecords=0;
  PadInitDirect(first,second);
  assert(stage==3 && PadComActive==1 && PadMultitapEnabled==seed);
  assert(PadChannelRecords==&external_record);
  assert(!memcmp(&actual,&expected,sizeof(actual)));
  for(j=0;j<sizeof(external_record);j++)assert(((u8 *)&external_record)[j]==0xB7);
  assert(callback_page.bytes[0]==0xA5 && callback_page.bytes[7]==0xA5 && callback_page.bytes[16]==0xA5 && callback_page.bytes[0x4387]==0xA5);
 }
 return 0;
}
'''
class ChannelInitTests(unittest.TestCase):
    def test_actual_constructor_and_registrar_with_ubsan(self):
        with tempfile.TemporaryDirectory() as tmp:
            c=Path(tmp)/'init.c';exe=Path(tmp)/'init';c.write_text(code)
            subprocess.run(['cc','-std=gnu99','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(c),'-o',str(exe)],check=True,capture_output=True)
            subprocess.run([str(exe)],check=True,capture_output=True)
if __name__=='__main__':unittest.main()
