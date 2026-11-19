"""Current shared camera initializer/controller with full native owners.

Independent transitions and original MIPS check live camera-root replacement,
the actual bounds pointer array, curve aliases and arithmetic wrap. Curve and
model-page services are instrumented; this is not an interpolation/render test.
"""
import ctypes as C
import hashlib
import os
from pathlib import Path
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest

from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_MIPS, UC_MODE_LITTLE_ENDIAN, UC_MODE_MIPS32

from c_functions import address_of, definition_source, source_owners
from test_vehicle_initializer import Cpu

ROOT = Path(__file__).resolve().parents[2]
NAMES = ['InitCameraState', 'UpdateCarShowcaseCamera']
SERVICES = ['PsyqBzero', 'SetModelTexturePage', 'InterpCurveEase', 'FindCurveSegment', 'InterpCurveLinear']
STACK, STOP = 0x803F0000, 0x803FF000
PREFIX = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "vehicle_wire.h"
#include "scene_camera_wire.h"
#include "scene_context_wire.h"
#include "rr/scene_camera_state.h"
#include "game/car/bounds.h"
#include "game/gfx/scene1.h"
#include "game/camera/camfx.h"
#include "game/camera/camview.h"
typedef struct {u32 before[4];SceneRaceContext value;u32 after[4];} ContextOwner;
typedef struct {u32 before[4];SceneCameraState160 value;u32 after[4];} CameraOwner;
typedef struct {u32 before[4];RaceVehicle values[2];u32 after[4];} CarOwner;
typedef struct {u32 before[4];CarBounds value;u32 after[4];} BoundsOwner;
typedef struct {u32 before[4];CarBounds *values[3];u32 after[4];} BoundsPointers;
#include "scene_camera_legacy_inputs.h"
typedef struct {
 ContextOwner context;CameraOwner cameras[2];CarOwner cars;BoundsOwner bounds[3];BoundsPointers table;
 PairS16 curves[25];s32 masks[4];s16 index,selector;s32 draw;
} World;
typedef struct {World entry,after;u32 operation,args[4],result;} Event;
typedef struct {World final;unsigned count;Event events[12];} Capture;
enum {WIRE_SIZE=4112};
static World live,initial;static Capture actual,expected,*capture;
static unsigned scenario,kind,serial,recordWire;
static unsigned serviceSeen[6],rootSeen[3],branchSeen[2],shiftSeen[32],tableSeen[4],edgeSeen[12],wrapSeen;
static u8 firstWire[WIRE_SIZE],lastWire[WIRE_SIZE],beforeWire[12][WIRE_SIZE],afterWire[12][WIRE_SIZE];
static const s32 edges[]={INT32_MIN,INT32_MIN+1,-65537,-32768,-4,-1,0,1,4,32767,65536,INT32_MAX};
#define RaceContext live.context.value
#define PlayerCars live.cars.values
#define CarBoundsTable live.table.values
#define CarCameraHeightOffsets (&live.curves[0])
#define FinishCamFollowTargets (&live.curves[3])
#define ShowcaseCamDepthCurve (&live.curves[6])
#define FinishCamSplineWeights (&live.curves[9])
#define IntroCamDollyKeyframes (&live.curves[12])
#define ReplayCamBankWeights live.masks
#define SelectedMakerIndex live.index
#define SelectedMakerCarIndex live.selector
#define DrawSubOtFlag live.draw
static u32 next(u32 *seed){u32 n=*seed;n^=n<<13;n^=n>>17;n^=n<<5;return *seed=n;}
static s32 word(u32 n){s32 r;memcpy(&r,&n,4);return r;}
static u32 pointer(const void *p){
 if(p==&RaceContext)return @RACE@;
 if(p==&RaceContext.cameraEC)return @RACE@+0xEC;
 for(unsigned i=0;i<2;i++)if(p==&live.cameras[i].value)return 0x80180010U+i*4096U;
 for(unsigned i=0;i<3;i++)if(p==&live.bounds[i].value)return 0x801A0010U+i*4096U;
 for(unsigned i=0;i<25;i++)if(p==&live.curves[i])return @CURVES@+4U*i;
 assert(!"pointer outside complete owned camera/bounds/curve objects");return 0;
}
static SceneCameraState160 *camera(unsigned n){assert(n<3);return n?&live.cameras[n-1].value:&RaceContext.cameraEC;}
static void put32(u8 *p,u32 n){memcpy(p,&n,4);}
static void wire(const World *w,u8 *out){
 u8 *p=out;
 memcpy(p,w->context.before,16);p+=16;rr_scene_context_prefix_store(p,&w->context.value);p+=0xEC;
 rr_scene_camera_store(p,&w->context.value.cameraEC);p+=0x160;
 put32(p,pointer(w->context.value.camera24C));p+=4;
 rr_scene_context_tail_store(p,&w->context.value);p+=0x314;memcpy(p,w->context.after,16);p+=16;
 for(unsigned i=0;i<2;i++){
  memcpy(p,w->cameras[i].before,16);p+=16;rr_scene_camera_store(p,&w->cameras[i].value);p+=0x160;
  memcpy(p,w->cameras[i].after,16);p+=16;
 }
 memcpy(p,w->cars.before,16);p+=16;
 for(unsigned i=0;i<2;i++){rr_vehicle_wire_store_fields(p,&w->cars.values[i]);p+=800;}
 memcpy(p,w->cars.after,16);p+=16;
 memcpy(p,w->bounds,sizeof(w->bounds));p+=sizeof(w->bounds);
 memcpy(p,w->table.before,16);p+=16;
 for(unsigned i=0;i<3;i++){put32(p,pointer(w->table.values[i]));p+=4;}
 memcpy(p,w->table.after,16);p+=16;
 memcpy(p,w->curves,100);p+=100;memcpy(p,w->masks,16);p+=16;
 memcpy(p,&w->index,2);p+=2;memcpy(p,&w->selector,2);p+=2;
 memcpy(p,&w->draw,4);p+=4;assert(p-out==WIRE_SIZE);
}
static void mutate(unsigned operation){
 unsigned n=serial++;if(!(scenario&4))return;
 unsigned root=(scenario+n)%3;RaceContext.camera24C=camera(root);rootSeen[root]++;
 SceneCameraState160 *c=RaceContext.camera24C;
 c->extentA0=edges[(scenario+n)%12];c->distance100=edges[(scenario+n+3)%12];
 c->offsetC4=edges[(scenario+n+6)%12];c->offsetB0=word(scenario*0x17171717U+n);
 c->offsetAC=word(~(scenario*0x1234567U+n));c->offsetC8=edges[(scenario+n+7)%12];
 live.index=(s16)((scenario+n)%4);live.selector=(s16)((scenario/4+n)%32);PlayerCars[0].halfSpan10E=(s16)(scenario*37U+n);
 live.masks[live.index]=word((scenario+n)&1?UINT32_MAX:0);
 live.table.values[0]=&live.bounds[(scenario+n)%3].value;
 live.bounds[(scenario+n)%3].value.unk08=(s16)(scenario*73U+n);
 live.curves[(scenario+n)%25].a=(s16)(scenario+n);
 live.draw=word(0xABCDEF00U|operation);
}
static u32 service(unsigned operation,u32 a,u32 b,u32 c,u32 d){
 unsigned i=capture->count++;assert(i<12);Event *e=&capture->events[i];
 memcpy(&e->entry,&live,sizeof(live));e->operation=operation;e->args[0]=a;e->args[1]=b;e->args[2]=c;e->args[3]=d;
 unsigned index=(scenario/3+serial+operation)%12;edgeSeen[index]++;
 e->result=operation==4?(scenario+serial)%(a==@CURVES@+48U?13U:3U):(u32)edges[index];
 serviceSeen[operation]++;if(recordWire)wire(&live,beforeWire[i]);
 if(operation==1)memset(&RaceContext.cameraEC,0,sizeof(RaceContext.cameraEC));
 mutate(operation);if(recordWire)wire(&live,afterWire[i]);return e->result;
}
void PsyqBzero(void *p,s32 bytes){
 assert(p==&RaceContext.cameraEC&&bytes==sizeof(SceneCameraState160));
 /* Native sizeof includes its full pointer; the PAL call clears one0x160-byte camera. */
 (void)service(1,pointer(p),RR_SCENE_CAMERA_WIRE_SIZE,0,0);
}
/* Observe the four existing call slots; the real model-page service consumes
 * only its first argument. This adapter does not redefine its public API. */
static void traceModelTexturePage(s32 page,s32 distance,SceneRaceContext *context,SceneCameraState160 *selected){
 assert(page==25&&context==&RaceContext);(void)service(2,(u32)page,(u32)distance,pointer(context),pointer(selected));
}
/* The historical PAL trace retains three unused register observations.
 * The corrected C service has one argument; its native observation adapter
 * reads the equivalent already-published camera state. The old oracle and
 * all PAL register checks remain unchanged. */
static void traceCurrentModelTexturePage(s32 page){
 SceneCameraState160 *selected=RaceContext.camera24C;
 traceModelTexturePage(page,selected->distance100,&RaceContext,selected);
}
s32 InterpCurveEase(s32 x){return word(service(3,(u32)x,0,0,0));}
s32 FindCurveSegment(PairS16 *curve,s32 x){
 u32 p=pointer(curve);assert(p==@CURVES@+12U||p==@CURVES@+24U||p==@CURVES@+36U||p==@CURVES@+48U);
 tableSeen[(p-@CURVES@)/12U-1U]++;return word(service(4,p,(u32)x,0,0));
}
s32 InterpCurveLinear(PairS16 *curve,s32 index,s32 x){return word(service(5,pointer(curve),(u32)index,(u32)x,0));}
'''
REFERENCE = r'''
static s32 add(s32 a,s32 b){int64_t n=(int64_t)a+b;wrapSeen+=n<INT32_MIN||n>INT32_MAX;return word((u32)(uint64_t)n);}
static void reference(s32 first,s32 second){
 if(!kind){
  RaceContext.camera24C=&RaceContext.cameraEC;PsyqBzero(&RaceContext.cameraEC,sizeof(SceneCameraState160));
  SceneCameraState160 *c=RaceContext.camera24C;
  static const s16 directions[3][3]={{449,-3576,-1946},{2970,-2779,482},{-2637,-1602,2694}};
  static const u8 colors[3]={128,64,32};
  for(unsigned i=0;i<3;i++){
   c->lighting78.matrix.fields.vectors[i].x=directions[i][0];c->lighting78.matrix.fields.vectors[i].y=directions[i][1];c->lighting78.matrix.fields.vectors[i].z=directions[i][2];
   c->lighting78.matrix.fields.colors[i].red=colors[i];c->lighting78.matrix.fields.colors[i].green=colors[i];c->lighting78.matrix.fields.colors[i].blue=colors[i];
  }
  for(unsigned i=0;i<3;i++)c->lighting78.ambient[i]=64;
  c->position110.xyz.x=0;c->position110.xyz.y=-240;c->position110.xyz.z=0;
  c->rotation120.fields.x=c->rotation120.fields.y=c->rotation120.fields.z=0;
  c->value12C=2048;c->value130=440;c->parts18[0].position00.xyz.x=-192;c->parts18[1].position00.xyz.x=192;
  unsigned on=((u32)live.masks[live.index]>>live.selector)&1;branchSeen[on]++;shiftSeen[live.selector]++;
  c->extentA0=on?202:169;c->mirrorA4=(s32)on;
  live.curves[6].a=(s16)(c->extentA0*4);
  s32 distance=c->extentA0-4-live.table.values[0]->unk08-PlayerCars[0].halfSpan10E/2;
  c->distance100=distance;c->mode13C=first;c->mode140=second;
  traceModelTexturePage(25,distance,&RaceContext,c);live.draw=0;
 }else{
  s32 angle=InterpCurveEase(first);RaceContext.camera24C->parts18[0].rotation10.y=(s16)angle;
  RaceContext.camera24C->parts18[1].rotation10.y=(s16)(u16)(0U-(u32)angle);
  s32 index=FindCurveSegment(FinishCamFollowTargets,first);s32 roll=InterpCurveLinear(FinishCamFollowTargets,index,first);
  RaceContext.camera24C->parts18[3].rotation10.x=(s16)roll;
  index=FindCurveSegment(ShowcaseCamDepthCurve,first);s32 z=InterpCurveLinear(ShowcaseCamDepthCurve,index,first);
  index=FindCurveSegment(FinishCamSplineWeights,first);s32 y=InterpCurveLinear(FinishCamSplineWeights,index,first);
  SceneCameraState160 *c=RaceContext.camera24C;
  c->parts18[3].position00.xyz.z=word((u32)((uint64_t)(u32)z-(uint64_t)(u32)c->extentA0*4U-16U));
  c->parts18[3].position00.xyz.y=add(y,12);c->parts18[2].position00.xyz.z=z;c->parts18[2].position00.xyz.y=y;
  c->offsetB0=add(z/4,word(0U-(u32)c->distance100));c->offsetAC=y/4;
  index=FindCurveSegment(IntroCamDollyKeyframes,first);s32 offset=InterpCurveLinear(IntroCamDollyKeyframes,index,first);
  s32 turn=word((u32)(UINT64_C(0)-(uint64_t)(u32)offset*32U));
  c=RaceContext.camera24C;c->offsetC8=offset;PlayerCars[0].wheelSteering17C=(s16)(turn%4096);
  PlayerCars[0].position20.words[2]=add(c->offsetB0,c->offsetC8);PlayerCars[0].position20.words[1]=add(c->offsetAC,c->offsetC4);
 }
}
static void initialize(unsigned number){
 scenario=number;kind=number&1;serial=0;u32 seed=number*0x9E3779B9U+0x55700U;
 FrozenSceneWorldInput frozen;u8 *p=(u8*)&frozen;
 for(unsigned i=0;i<sizeof(frozen);i++)p[i]=(u8)next(&seed);
 memset(&live,0xA5,sizeof(live));rr_scene_track_reset();
 memcpy(live.context.before,frozen.context.before,16);memcpy(live.context.after,frozen.context.after,16);
 rr_scene_context_prefix_load(&RaceContext,frozen.context.value.prefix);
 rr_scene_camera_load(&RaceContext.cameraEC,frozen.context.value.camera.bytes);
 rr_scene_context_tail_load(&RaceContext,frozen.context.value.tail);
 for(unsigned i=0;i<2;i++){
  memcpy(live.cameras[i].before,frozen.cameras[i].before,16);memcpy(live.cameras[i].after,frozen.cameras[i].after,16);
  rr_scene_camera_load(&live.cameras[i].value,frozen.cameras[i].value.bytes);
 }
 memcpy(&live.cars,&frozen.cars,sizeof(live.cars));memcpy(live.bounds,frozen.bounds,sizeof(live.bounds));
 memcpy(&live.table,&frozen.table,sizeof(live.table));memcpy(live.curves,frozen.curves,100);memcpy(live.masks,frozen.masks,16);
 live.index=frozen.index;live.selector=frozen.selector;live.draw=frozen.draw;
 rr_vehicle_wire_reset();for(unsigned i=0;i<2;i++){
  CarRecord *record=rr_vehicle_record_resolve(0x80200000U+i*4096U);
  live.cars.values[i].control.record18=record;live.cars.values[i].control.handling1C=&record->handling1C8;
 }
 RaceContext.camera24C=camera((number/2)%3);rootSeen[(number/2)%3]++;
 for(unsigned i=0;i<3;i++)live.table.values[i]=&live.bounds[i].value;
 live.index=(s16)((number/32)%4);live.selector=(s16)((number/128)%32);PlayerCars[0].halfSpan10E=(s16)(number*37U);
 live.masks[live.index]=word((number&8)?UINT32_MAX:0);memcpy(&initial,&live,sizeof(live));
}
static s32 argument0(unsigned n){return n%3?edges[(n/3)%12]:word(n*0x17171717U);}
static s32 argument1(unsigned n){return edges[(n/5)%12];}
unsigned exercise_native(void){
 _Static_assert(sizeof(CameraOwner)==sizeof(SceneCameraState160)+32&&sizeof(BoundsOwner)==44,"complete native camera/bounds owners");
 _Static_assert(offsetof(SceneRaceContext,reserved250)==offsetof(SceneRaceContext,camera24C)+sizeof(void*),"native context tail");
 _Static_assert(offsetof(SceneRaceContext,startGrid53C)-offsetof(SceneRaceContext,reserved250)==0x2F0,"native alignment before full grid pointer");
 for(unsigned n=0;n<32768;n++){
  initialize(n);assert((uintptr_t)RaceContext.camera24C>UINT32_MAX&&(uintptr_t)CarBoundsTable[0]>UINT32_MAX);
  capture=&expected;capture->count=0;reference(argument0(n),argument1(n));memcpy(&capture->final,&live,sizeof(live));
  memcpy(&live,&initial,sizeof(live));serial=0;capture=&actual;capture->count=0;
  if(kind)UpdateCarShowcaseCamera(argument0(n));else InitCameraState(argument0(n),argument1(n));
  memcpy(&capture->final,&live,sizeof(live));assert(actual.count==expected.count);
  for(unsigned i=0;i<actual.count;i++)assert(!memcmp(&actual.events[i],&expected.events[i],sizeof(Event)));
  assert(!memcmp(&actual.final,&expected.final,sizeof(World)));
  assert(rr_test_record_count==2&&rr_test_handling_count==0);rr_scene_tracks_unchanged();
 }
 for(unsigned i=1;i<6;i++)assert(serviceSeen[i]);for(unsigned i=0;i<3;i++)assert(rootSeen[i]);
 for(unsigned i=0;i<32;i++)assert(shiftSeen[i]);for(unsigned i=0;i<4;i++)assert(tableSeen[i]);
 for(unsigned i=0;i<12;i++)assert(edgeSeen[i]);assert(branchSeen[0]&&branchSeen[1]&&wrapSeen);return 32768;
}
unsigned prepare_trace(unsigned number){initialize(number);wire(&live,firstWire);capture=&expected;capture->count=0;recordWire=1;reference(argument0(number),argument1(number));wire(&live,lastWire);recordWire=0;return capture->count;}
const void *initial_wire(void){return firstWire;}const void *final_wire(void){return lastWire;}
const void *event_wire(unsigned index,unsigned after){assert(index<capture->count);return after?afterWire[index]:beforeWire[index];}
u32 event_value(unsigned index,unsigned field){assert(index<capture->count&&field<6);Event *e=&capture->events[index];return field==0?e->operation:field==5?e->result:e->args[field-1];}
u32 argument(unsigned number,unsigned index){return (u32)(index?argument1(number):argument0(number));}
unsigned wire_size(void){return WIRE_SIZE;}
'''


class SceneCameraControllerTest(unittest.TestCase):
    def test_complete_native_contexts_and_original_mips(self):
        owners = source_owners(ROOT/'src/main/PAL')
        body = '\n'.join(definition_source(owners[n].read_text(), n) for n in NAMES)
        body = body.replace('SetModelTexturePage(', 'traceCurrentModelTexturePage(')
        prefix = PREFIX.replace('@RACE@', hex(address_of('RaceContext'))+'U').replace('@CURVES@', hex(address_of('CarCameraHeightOffsets'))+'U')
        self.assertEqual(address_of('D_800AC006'), address_of('PlayerCars')+0x10E)
        for offset, name in [(12, 'FinishCamFollowTargets'), (24, 'ShowcaseCamDepthCurve'), (36, 'FinishCamSplineWeights'), (48, 'IntroCamDollyKeyframes')]:
            self.assertEqual(address_of(name), address_of('CarCameraHeightOffsets')+offset)
        with TemporaryDirectory() as temporary:
            source, library = Path(temporary)/'native.c', Path(temporary)/'native.dylib'
            source.write_text(prefix+body+REFERENCE)
            subprocess.run(['cc', '-std=c11', '-O2', '-shared', '-fPIC', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined,bounds', '-fno-sanitize-recover=all', '-I', str(ROOT/'include'),
                            '-I', str(ROOT/'tools/scripts'), str(source), '-o', str(library)], check=True)
            native = C.CDLL(str(library));native.exercise_native.restype = C.c_uint
            self.assertEqual(native.exercise_native(), 32768)
            native.prepare_trace.argtypes = [C.c_uint];native.prepare_trace.restype = C.c_uint
            native.initial_wire.restype = native.final_wire.restype = C.c_void_p
            native.event_wire.argtypes = [C.c_uint, C.c_uint];native.event_wire.restype = C.c_void_p
            native.event_value.argtypes = [C.c_uint, C.c_uint];native.event_value.restype = C.c_uint32
            native.argument.argtypes = [C.c_uint, C.c_uint];native.argument.restype = C.c_uint32
            native.wire_size.restype = C.c_uint
            # Freeze every original scenario, including complete service entry/exit
            # images. Native pointer fields may widen; the PAL oracle must not change.
            inputs, traces = hashlib.sha256(), hashlib.sha256()
            for number in range(32768):
                events = native.prepare_trace(number)
                header = struct.pack('<4I', number, events, native.argument(number, 0), native.argument(number, 1))
                initial = C.string_at(native.initial_wire(), 4112)
                inputs.update(header+initial);traces.update(header+initial)
                for event in range(events):
                    traces.update(struct.pack('<6I', *(native.event_value(event, field) for field in range(6))))
                    traces.update(C.string_at(native.event_wire(event, 0), 4112))
                    traces.update(C.string_at(native.event_wire(event, 1), 4112))
                traces.update(C.string_at(native.final_wire(), 4112))
            self.assertEqual(inputs.hexdigest(), 'b31ab5ff4e2aed1412cd7db95eaf65ab0b2fe83bbc52b880dcd1b0506a2d126d')
            self.assertEqual(traces.hexdigest(), 'c75b00d6ff550d1e42f4367cd6895be23ae82dec9b2ea29f9f9b485d485dae84')
            regions = [(address_of('RaceContext')-16, 0x564+16), (address_of('RaceContext')+0x57C,16), (0x80180000, 384), (0x80181000, 384),
                       (address_of('PlayerCars')-16, 1632), (0x801A0000, 44), (0x801A1000, 44), (0x801A2000, 44),
                       (address_of('CarBoundsTable')-16, 44), (address_of('CarCameraHeightOffsets'), 100),
                       (address_of('ReplayCamBankWeights'), 16), (address_of('SelectedMakerIndex'), 2),
                       (address_of('SelectedMakerCarIndex'), 2), (address_of('DrawSubOtFlag'), 4)]
            size = native.wire_size();self.assertEqual(sum(n for _, n in regions), size)
            intervals = sorted((a, a+n) for a, n in regions)
            self.assertTrue(all(a[1] <= b[0] for a, b in zip(intervals, intervals[1:])))
            memory = Uc(UC_ARCH_MIPS, UC_MODE_LITTLE_ENDIAN | UC_MODE_MIPS32);memory.mem_map(0, 0x400000)
            write = lambda a, b: memory.mem_write(a & 0x1FFFFFFF, bytes(b))
            read = lambda a, n: bytes(memory.mem_read(a & 0x1FFFFFFF, n))
            original = (ROOT/'assets/PAL/main.exe').read_bytes();base = struct.unpack_from('<I', original, 24)[0]
            write(base, original[0x800:])
            if os.getenv('RR_COMPILED_ELF'):
                with open(os.environ['RR_COMPILED_ELF'], 'rb') as stream:
                    elf = ELFFile(stream)
                    for name in NAMES:
                        symbol = elf.get_section_by_name('.symtab').get_symbol_by_name(name)[0]
                        section = elf.get_section(symbol['st_shndx']);offset = symbol['st_value']-section['sh_addr']
                        self.assertEqual(symbol['st_value'], address_of(name));write(symbol['st_value'], section.data()[offset:offset+symbol['st_size']])
            services = {address_of(name): i+1 for i, name in enumerate(SERVICES)}

            def store(cpu, access, address, length, value, user):
                address &= 0x1FFFFFFF
                self.assertTrue(any((a & 0x1FFFFFFF) <= address and address+length <= (a & 0x1FFFFFFF)+n
                                    for a, n in regions+[(STACK-64, 64)]), (hex(address), length))

            cpu = Cpu(memory, 0, 100, 101, 102, store)

            def write_state(data):
                write(address_of('RaceContext')+0x564,bytes(8)+b'\xA5'*0x10)
                offset = 0
                for address, length in regions:
                    write(address, data[offset:offset+length]);offset += length

            def state():
                self.assertEqual(read(address_of('RaceContext')+0x564,0x18),bytes(8)+b'\xA5'*0x10)
                return b''.join(read(a, n) for a, n in regions)

            for case in range(1024):
                number = (case*7919) & 32767;events = native.prepare_trace(number)
                write_state(C.string_at(native.initial_wire(), size));write(STACK-128, b'\xA5'*256)
                cpu.reset();cpu.r = [0]+[0xBEEF0000+r for r in range(1, 32)]
                cpu.r[4], cpu.r[5] = native.argument(number, 0), native.argument(number, 1)
                cpu.r[29], cpu.r[31], cpu.pc = STACK, STOP, address_of(NAMES[number & 1]);event = 0
                for instruction in range(2000):
                    if cpu.pc == STOP:break
                    if cpu.pc in services:
                        self.assertLess(event, events);operation = services[cpu.pc]
                        self.assertEqual(operation, native.event_value(event, 0), (number, event))
                        for index in range([2, 4, 1, 2, 3][operation-1]):
                            self.assertEqual(cpu.r[4+index], native.event_value(event, index+1), (number, event, index))
                        self.assertEqual(state(), C.string_at(native.event_wire(event, 0), size), (number, event, 'entry'))
                        write_state(C.string_at(native.event_wire(event, 1), size))
                        for register in (1, 2, 3, *range(4, 16), 24, 25):cpu.r[register] = (0xA1230000+event*257+register) & 0xFFFFFFFF
                        cpu.r[2] = native.event_value(event, 5);cpu.pc = cpu.r[31];event += 1
                    else:cpu.step()
                self.assertEqual(cpu.pc, STOP, number);self.assertEqual(event, events)
                self.assertEqual(state(), C.string_at(native.final_wire(), size), (number, 'final'))
                self.assertEqual(cpu.r[29], STACK);self.assertEqual(cpu.r[31], STOP)
                for register in (*range(16, 24), 28, 30):self.assertEqual(cpu.r[register], 0xBEEF0000+register)
                self.assertEqual(read(STACK-128, 64), b'\xA5'*64);self.assertEqual(read(STACK, 128), b'\xA5'*128)
            print('PASS 32768 native and 1024', 'compiled' if os.getenv('RR_COMPILED_ELF') else 'original',
                  'MIPS scene camera cases; complete contexts, live roots, bounds arrays, curve aliases and UBSan')


if __name__ == '__main__':
    unittest.main()
