"""Event setup, real wrapping callback and typed BIOS/driver service boundaries."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

class Overlay667EventsTest(unittest.TestCase):
    def test_fresh_handles_callbacks_and_scalar_driver_arguments(self):
        root=Path(__file__).resolve().parents[2]
        self.assertEqual((root/'build/overlays/667.bin').read_bytes()[4:9],b'sio:\0')
        bodies='\n'.join((root/f'src/overlays/PAL/serial_link/func_{a}.c').read_text() for a in ['80114D10','80114988','80114950'])
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
#include "rr/overlay667_events.h"
typedef struct {u32 head[4];s32 counter,status,pending;u32 event0,event1;s32 file0,file1;u32 tail[4];} State;
typedef struct {s32 kind;u32 args[3];const char *name;RrBiosEventCallback callback;State state;} Event;
static struct World {State s;u32 calls;RrBiosEventCallback registered;Event events[6];u32 guard[4];} world,expected;
const char serial_link_sio_device_name[]="sio:";
#define serial_link_error_count world.s.counter
#define serial_link_rx_pending world.s.pending
#define serial_link_rx_event world.s.event0
#define serial_link_error_event world.s.event1
#define LinkWriteHandle world.s.file0
#define LinkReadHandle world.s.file1
''' + bodies + r'''
static u32 seed=0x14988,scenario;
static u32 next(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
static s32 word(u32 bits){s32 v;memcpy(&v,&bits,4);return v;}
static u32 result(unsigned step){return (scenario*0x9E3779B9U)^(step*0xABCDEF01U)^0x80000001U;}
static void callback_reference(State *s){u32 bits;memcpy(&bits,&s->counter,4);bits+=1;memcpy(&s->counter,&bits,4);s->pending=0;}
static void effect(struct World *v){
 unsigned step=v->calls;State *s=&v->s;
 s->counter=word(scenario*0x10203041U+step);if(((scenario+step)&15)==0)s->counter=INT32_MAX;
 s->pending=word(scenario^step^0xFEDCBA98U);s->status=word(scenario*0x40506071U+step);
 s->event0=result(step+6);s->event1=result(step+7);s->file0=word(result(step+8));s->file1=word(result(step+9));
 if(v->registered && ((scenario+step)&1)){
  assert(v->registered==serial_link_error_callback);
  if(v==&world)assert(v->registered()==0);else callback_reference(s);
 }
}
static u32 observe(struct World *v,s32 kind,u32 a,u32 b,u32 c,const char *name,RrBiosEventCallback callback){
 Event *e;unsigned step=v->calls;assert(step<6);e=&v->events[v->calls++];memset(e,0,sizeof(*e));e->kind=kind;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->name=name;e->callback=callback;memcpy(&e->state,&v->s,sizeof(v->s));
 if(callback)v->registered=callback;effect(v);return result(step);
}
RrBiosEventHandle OpenEvent(u32 eventClass,u32 specification,u32 mode,RrBiosEventCallback callback){
 assert(world.calls<2);assert(eventClass==0xF000000BU);
 return observe(&world,0,eventClass,specification,mode,0,callback);
}
s32 FileOpen(const char *name,s32 mode){assert(world.calls==2 || world.calls==3);assert(name==serial_link_sio_device_name && strcmp(name,"sio:")==0);return word(observe(&world,1,(u32)mode,0,0,name,0));}
s32 EnableEvent(RrBiosEventHandle handle){assert(world.calls==4 || world.calls==5);return word(observe(&world,2,handle,0,0,0,0));}
s32 _sio_control(s32 command,s32 operation,unsigned long argument){assert(world.calls<2 && argument==0);return word(observe(&world,3,(u32)command,(u32)operation,0,0,0));}
static void reference_setup(void){
 expected.s.event0=observe(&expected,0,0xF000000BU,0x400,0x2000,0,0);
 expected.s.event1=observe(&expected,0,0xF000000BU,0x8000,0x1000,0,serial_link_error_callback);
 expected.s.file0=word(observe(&expected,1,2,0,0,serial_link_sio_device_name,0));
 expected.s.file1=word(observe(&expected,1,0x8001,0,0,serial_link_sio_device_name,0));
 observe(&expected,2,expected.s.event1,0,0,0,0);observe(&expected,2,expected.s.event0,0,0,0,0);
}
static void initialize(void){unsigned i;for(i=0;i<sizeof(world);i++)((u8*)&world)[i]=(u8)next();world.calls=0;world.registered=0;memcpy(&expected,&world,sizeof(world));}
int main(void){unsigned k;u32 bits;
 assert((uintptr_t)serial_link_sio_device_name>UINT32_MAX && sizeof(RrBiosEventCallback)==sizeof(void*));
 for(k=0;k<65536;k++){
  scenario=k;initialize();bits=k*65537U;
  if(k==0)bits=0x7FFFFFFFU;else if(k==1)bits=0xFFFFFFFFU;else if(k==2)bits=0x80000000U;
  world.s.counter=word(bits);memcpy(&expected,&world,sizeof(world));callback_reference(&expected.s);
  assert(serial_link_error_callback()==0);assert(memcmp(&world,&expected,sizeof(world))==0);
  initialize();reference_setup();assert(serial_link_open_sio()==0);assert(world.calls==6);assert(memcmp(&world,&expected,sizeof(world))==0);
  initialize();observe(&expected,3,0,0,0,0,0);observe(&expected,3,0,1,0,0,0);serial_link_func_80114950();assert(world.calls==2);assert(memcmp(&world,&expected,sizeof(world))==0);
 }
 return 0;
}
'''
        with TemporaryDirectory() as temp:
            p=Path(temp);(p/'test.c').write_text(source)
            subprocess.run(['cc','-std=gnu11','-O2','-fsanitize=undefined','-fno-sanitize-recover=all','-I',str(root/'include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__=='__main__':unittest.main()
