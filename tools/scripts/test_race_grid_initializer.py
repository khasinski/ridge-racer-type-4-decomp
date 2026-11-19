"""Literal grid/spec producers and InitComCars on complete native owners.

The independent model covers the full eleven-car clear, all modes, zero/seven
cars, both target slots, captured spec roots and live grid roots. Track/model
services are controlled boundaries; their algorithms are outside this test.
"""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest
from c_functions import definition_source, source_owners
ROOT=Path(__file__).resolve().parents[2]
PREFIX=r'''
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "rr/race.h"
#include "game/car/carlane.h"
#include "game/course/trkres2.h"
typedef struct {
 u32 before[4];SceneRaceContext scene;u32 afterScene[4];
 RaceVehicle cars[11];u32 afterCars[4];
 RaceStartGrid60 grids[2];u32 afterGrids[4];
 RaceSpecEntry30 specs[2][12];u32 after[4];
 s16 mode,maker,team;u16 draft;
} World;
static World world,expected;
static unsigned scenario,events,current;
#define RaceContext world.scene
#define ComCars world.cars
#define GameModeType world.mode
#define SelectedMakerIndex world.maker
#define SelectedTeamIndex world.team
#define CarDraftSlipstreamFactor world.draft
void PsyqBzero(void *p,s32 size){
 assert(events++==0&&p==world.cars&&size==sizeof(world.cars));memset(p,0,(size_t)size);
}
void ResetCarState(RaceVehicle *p){
 assert(events++==1+4*current&&p==&world.cars[current]);
 if(current==0){if(scenario&1)world.scene.carSpecs540=world.specs[1];if(scenario&2)world.scene.startGrid53C=&world.grids[1];}
 p->control.holdFlag48=77;
}
s32 FindTrackNode(s32 *position,s32 node){
 assert(events++==2+4*current&&position==world.cars[current].position20.words);
 unsigned k=0;while(world.scene.startOrder00[k]!=(s16)(current+1))k++;
 assert(position[0]==world.scene.startGrid53C->positions10[k].x&&position[1]==0&&position[2]==world.scene.startGrid53C->positions10[k].z);
 assert(node==world.scene.startGrid53C->trackNode00);return node+(s32)(scenario%7);
}
void AssignCarModelSlot(RaceVehicle *p){assert(events++==3+4*current&&p==&world.cars[current]);p->slotIndex1E6=(s16)(2*current);}
void UpdateCarTrackProjection(RaceVehicle *p){
 assert(events++==4+4*current&&p==&world.cars[current]);p->lateralOffset172=(s16)(-300+37*current+scenario%33);current++;
}
s32 DebugPrintStub(const char *text,...){
 va_list args;va_start(args,text);s32 count=va_arg(args,int);va_end(args);
 assert(!strcmp(text,"InitComCar OK!\n")&&count==world.scene.comCarCount10);
 assert(events==1+4*current);events++;return 0;
}
'''
TAIL=r'''
static void model(unsigned count){
 World *w=&expected;memset(w->cars,0,sizeof(w->cars));w->scene.comCarCount10=(s32)count;
 for(unsigned i=0;i<count;i++){
  const RaceSpecEntry30 *spec=&world.specs[(scenario&1)&&i!=0][i];
  if(i==0){if(scenario&1)w->scene.carSpecs540=world.specs[1];if(scenario&2)w->scene.startGrid53C=&world.grids[1];}
  const RaceStartGrid60 *grid=w->scene.startGrid53C;unsigned k=0;
  while(w->scene.startOrder00[k]!=(s16)(i+1))k++;
  RaceVehicle *car=&w->cars[i];RaceVehicleControl *c=&car->control;
  c->holdFlag48=77;car->flags1E4=(s16)(i+1);car->slotIndex1E6=(s16)(2*i);
  car->position20.vector.x=grid->positions10[k].x;car->position20.vector.z=grid->positions10[k].z;
  car->posScaled00[0]=(u32)grid->positions10[k].x*256U;car->posScaled00[2]=(u32)grid->positions10[k].z*256U;
  car->trackNodeF0=grid->trackNode00+(s32)(scenario%7);
  car->rot54=grid->headings50[k];car->heading1D4=(s16)car->rot54;
  car->lateralOffset172=(s16)(-300+37*i+scenario%33);c->laneTimer38=10;c->laneStart3A=car->lateralOffset172;
  c->maxSpeedF8=spec->baseSpeed00*256;c->gearCountFC=spec->gearCount04;
  for(unsigned j=0;j<6;j++){c->gearRatios100[j]=(s16)(u16)(spec->ratios08[j]*4U);c->shiftThresholds10C[j]=spec->thresholds14[j];}
  c->maximum118=spec->maximum20;c->span11A=spec->span22;c->divisor11C=spec->divisor24;
  c->boostTimer44=spec->divisor24<0?-spec->divisor24:spec->divisor24;
  c->factor11E=spec->factor26;c->altSpeed120=spec->altSpeed28;c->steeringEnabled124=spec->steering2C;c->laneCooldown40=(s16)((i*25+50)/2);
 }
 unsigned index;
 if(w->mode==6||w->mode==1){w->scene.playerCount14=1;index=(unsigned)(w->mode==6?w->maker:w->team)+7;w->scene.specTargets544[1]=NULL;}
 else if(w->mode>=2&&w->mode<=4){w->scene.playerCount14=2;index=7;w->scene.specTargets544[1]=&w->scene.carSpecs540[7];}
 else{w->scene.playerCount14=0;index=(unsigned)w->team+7;}
 w->scene.specTargets544[0]=&w->scene.carSpecs540[index];
 if(w->mode==1){
  const RaceSpecEntry30 *target=w->scene.specTargets544[0];
  for(unsigned i=0;i<count;i++){
   RaceVehicleControl *c=&w->cars[i].control;c->metric12A=target->divisor24;
   c->divisor11C=(s16)(((s32)c->divisor11C*target->steering2C)/1024);
   c->boostTimer44=c->divisor11C<0?-c->divisor11C:c->divisor11C;
  }
  w->draft=(u16)target->factor26;
 }else w->draft=0x320;
}
int main(void){
 assert(sizeof(void*)==8);assert((uintptr_t)&world.cars[10]>UINT32_MAX);
 for(scenario=0;scenario<1024;scenario++){
  memset(&world,0xA5,sizeof(world));events=current=0;
  unsigned count=(scenario/8)%8;world.mode=(s16)(scenario%8);world.maker=(s16)((scenario/64)%4);world.team=(s16)((scenario/128)%4);
  SceneRaceContext *p=&world.scene;
  p->standings=NULL;p->cameraEC.tracks138=NULL;p->camera24C=NULL;p->laneOffsets54C=NULL;
  p->cameraNodes550=NULL;p->workspace554=NULL;p->lightPresets558=NULL;p->workspace55C=NULL;p->soundPayload564=NULL;p->lookupTables=NULL;
  p->specTargets544[0]=&world.specs[1][10];p->specTargets544[1]=&world.specs[1][11];
  for(unsigned i=0;i<8;i++)p->startOrder00[(i+scenario/16)%8]=(s16)i;
  for(unsigned b=0;b<2;b++){
   world.grids[b].trackNode00=(s32)(100+10*b);
   for(unsigned i=0;i<8;i++){world.grids[b].positions10[i].x=-10000+(s32)(i*731+b*419);world.grids[b].positions10[i].z=9000-(s32)(i*359+b*411);world.grids[b].headings50[i]=(s16)(i*511+b);}
   for(unsigned i=0;i<12;i++){
    RaceSpecEntry30 *s=&world.specs[b][i];s->baseSpeed00=(s32)(100+31*b+i);s->gearCount04=(s32)(1+i%6);
    for(unsigned j=0;j<6;j++){s->ratios08[j]=(u16)(scenario*317U+i*519U+j*3271U);s->thresholds14[j]=(u16)(scenario*719U+i*317U+j*137U);}
    s->maximum20=(s16)(i*127-600);s->span22=(s16)(b*819-400);s->divisor24=(s16)(i%2?(-3000+(s32)i*97):(3000-(s32)i*71));s->factor26=(s16)(scenario*131U+i*71U);
    s->altSpeed28=(s32)(i*16384);s->steering2C=(s16)((s32)i*17-100);
   }
  }
  SetCarSpecTable(world.specs[0]);SetTrackStartNode(&world.grids[0]);
  memcpy(&expected,&world,sizeof(world));model(count);InitComCars((s32)count);
  assert(current==count&&events==2+4*count);assert(!memcmp(&world,&expected,sizeof(world)));
 }
 return 0;
}
'''
class RaceGridInitializerTest(unittest.TestCase):
 def test_complete_native_owners_captured_specs_live_grid_and_modes(self):
  owners=source_owners(ROOT/'src/main/PAL');names=('SetCarSpecTable','SetTrackStartNode','InitComCars')
  bodies=[definition_source(owners[n].read_text(),n)for n in names];source=PREFIX+'\n'.join(bodies)+TAIL
  self.assertTrue(all(source.count(body)==1 for body in bodies))
  with TemporaryDirectory()as temporary:
   path=Path(temporary)/'owner.c';path.write_text(source);exe=path.with_suffix('')
   build=subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined,bounds','-fno-sanitize-recover=all','-I',str(ROOT/'include'),str(path),'-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(build.returncode,0,build.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True);self.assertEqual(run.returncode,0,run.stdout+run.stderr)
