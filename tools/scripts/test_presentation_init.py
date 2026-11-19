from motion_index_fixture_transport import transport as transport_motion_index
"""Shared presentation owner: inline storage, root replacement and script dispatch."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

from c_functions import source_owners, definition_source

ROOT = Path(__file__).resolve().parents[2]


class PresentationInitTest(unittest.TestCase):
    def test_complete_native_owner_and_script_restarts(self):
        owners = source_owners(ROOT/'src/main/PAL')
        names = ('InitMotionController', 'RestartMotionSequence',
                 'SetPresentationDrawEnabled', 'SetPresentationFullViewport', 'SetPresentationStencilEnabled')
        bodies = '\n'.join(definition_source(owners[name].read_text(), name)
                           for name in names)
        harness = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "rr/presentation.h"
#include "game/camera/campath.h"
#include "game/camera/camrun.h"
#include "game/gfx/motion1.h"
#include "game/gfx/ring.h"

typedef struct {
    u32 before[8];
    PresentationContextPrefix context;
    PresentationController alternate[2];
    u32 after[8];
} World;
static World actual, expected;
static unsigned replacement, clears, dispatches;
static s32 expectedOperation, expectedArgument;
static PresentationController *expectedTarget;
CameraScriptEntry8 HudCircularGaugeVerts[4], sequences[4][4];
CameraScriptEntry8 *MotionSequenceTable[4] = {
    sequences[0], sequences[1], sequences[2], sequences[3]
};
int MotionSequenceIndex;
#define RaceContext actual.context

void PsyqBzero(void *buffer, s32 size) {
    assert(clears++ == 0);
    assert(buffer == &actual.context.presentation250);
    assert(size == sizeof(PresentationController));
    memset(buffer, 0, size);
    if (replacement) RaceContext.controller3D8 = &actual.alternate[replacement-1];
}
static void dispatched(s32 operation, s32 argument) {
    assert(dispatches++ == 0);
    assert(operation == expectedOperation && argument == expectedArgument);
    assert(RaceContext.controller3D8 == expectedTarget);
}
void InitCameraMotion(int argument) {dispatched(0,argument);}
void InitMotionPanel(s32 argument) {dispatched(1,argument);}
void InitScrollTrack(int argument) {dispatched(2,argument);}
void InitMotionTrack(int argument) {dispatched(3,argument);}
''' + bodies + r'''
static void lighting(PresentationController *p) {
    static const s16 directions[3][3] = {
        {449,-3576,-1946}, {-100,0,100}, {-2637,-1602,2694}
    };
    unsigned i;
    p->position.z=-5000;
    p->viewDistance48=440;
    for (i=0;i<3;i++) {
        PreviewColor4 *color=&p->lighting20.matrix.fields.colors[i];
        PreviewVector8 *direction=&p->lighting20.matrix.fields.vectors[i];
        color->red=color->green=color->blue=i==2?32:192;
        direction->x=directions[i][0];direction->y=directions[i][1];
        direction->z=directions[i][2];p->lighting20.ambient[i]=0;
    }
}
int main(void) {
    unsigned seed, mode, operation, i, j, cases=0;
    CameraScriptEntry8 savedDefault[4], savedSequences[4][4];
    const s16 operations[]={-1,0,1,2,3,4};
    assert(sizeof(void*)==8 && (uintptr_t)&actual>UINT32_MAX);
    assert(offsetof(PresentationContextPrefix,controller3D8)==
           offsetof(PresentationContextPrefix,presentation250)+
           sizeof(PresentationController));
    for (seed=0;seed<128;seed++) for (mode=0;mode<3;mode++)
    for (operation=0;operation<6;operation++) {
        memset(&actual,(u8)(seed*29+0xA5),sizeof actual);
        for (i=0;i<2;i++) actual.alternate[i].scriptIndex68=(seed+i)%4;
        actual.context.controller3D8=&actual.alternate[1];
        for (i=0;i<4;i++) {
            HudCircularGaugeVerts[i]=(CameraScriptEntry8){operations[operation],(s16)(seed+i),12345};
            for (j=0;j<4;j++) sequences[i][j]=HudCircularGaugeVerts[(i+j)%4];
        }
        memcpy(savedDefault,HudCircularGaugeVerts,sizeof savedDefault);
        memcpy(savedSequences,sequences,sizeof savedSequences);
        expected=actual;
        memset(&expected.context.presentation250,0,sizeof(PresentationController));
        expectedTarget=mode?&actual.alternate[mode-1]:&actual.context.presentation250;
        expected.context.controller3D8=expectedTarget;
        PresentationController *p=mode?&expected.alternate[mode-1]:&expected.context.presentation250;
        lighting(p);p->script64=HudCircularGaugeVerts;
        p->scriptOperation6C=HudCircularGaugeVerts[p->scriptIndex68].operation;
        p->scriptArgument70=HudCircularGaugeVerts[p->scriptIndex68].argument;
        expectedOperation=p->scriptOperation6C;expectedArgument=p->scriptArgument70;
        replacement=mode;clears=dispatches=0;
        InitMotionController();
        assert(clears==1 && dispatches==(expectedOperation>=0 && expectedOperation<4));
        assert(!memcmp(&actual,&expected,sizeof actual));

        /* Restart selects a fresh sequence and resets the index before reading it. */
        MotionSequenceIndex=seed%4;expected=actual;
        p=mode?&expected.alternate[mode-1]:&expected.context.presentation250;
        p->scriptIndex68=0;p->scriptTimer6A=0;p->flags60=0;
        p->script64=sequences[MotionSequenceIndex];
        p->scriptOperation6C=p->script64[0].operation;
        p->scriptArgument70=p->script64[0].argument;
        expectedOperation=p->scriptOperation6C;expectedArgument=p->scriptArgument70;
        clears=dispatches=0;
        RestartMotionSequence();
        assert(!clears && dispatches==(expectedOperation>=0 && expectedOperation<4));
        assert(!memcmp(&actual,&expected,sizeof actual));
        assert(!memcmp(HudCircularGaugeVerts,savedDefault,sizeof savedDefault));
        assert(!memcmp(sequences,savedSequences,sizeof savedSequences));

        /* The three setters address distinct members of the selected owner. */
        p=mode?&expected.alternate[mode-1]:&expected.context.presentation250;
        p->drawEnabled54=(s32)seed-64;p->fullViewport50=123;p->stencilEnabled58=-321;
        SetPresentationDrawEnabled((s32)seed-64);SetPresentationFullViewport(123);SetPresentationStencilEnabled(-321);
        assert(!memcmp(&actual,&expected,sizeof actual));cases++;
    }
    printf("%u native presentation cases: inline owner, root replacement, scripts and setters\n",cases);
    return 0;
}
'''
        with TemporaryDirectory() as directory:
            path = Path(directory)
            c, exe = path/'presentation.c', path/'presentation'
            c.write_text(transport_motion_index(harness))
            compiled = subprocess.run(
                ['cc', '-std=gnu11', '-O2', '-Wall', '-Wextra', '-Werror',
                 '-fsanitize=undefined,bounds', '-fno-sanitize-recover=all',
                 '-I', str(ROOT/'include'), str(c), '-o', str(exe)],
                capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            ran = subprocess.run([str(exe)], capture_output=True, text=True)
            self.assertEqual(ran.returncode, 0, ran.stderr)
            self.assertIn('2304 native presentation cases', ran.stdout)
