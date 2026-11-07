"""Native behavior of the nine overlay samples; actual C and full host pointers."""
from overlay_units import read_c_source
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class OverlaySamplesTest(unittest.TestCase):
    def test_native_samples(self):
        bodies = []
        from verify_overlay_samples import SAMPLES
        for index, address, _ in SAMPLES:
            source = ROOT / f'src/overlays/PAL/{index}/func_{address:08X}.c'
            body = read_c_source(source)
            if index == 665:
                body = body.replace('BadgeRows', 'sample665_badges')
            bodies.append(body)
        self.assertEqual(len(bodies), 9)
        harness = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
#include <stddef.h>
#include "rr/layout.h"
#undef RR_ASSERT_SIZE
#define RR_ASSERT_SIZE(t,n) _Static_assert(sizeof(void*)!=4 || sizeof(t)==n,"target size")
#undef RR_ASSERT_OFFSET
#define RR_ASSERT_OFFSET(t,f,n) _Static_assert(sizeof(void*)!=4 || offsetof(t,f)==n,"target offset")
#include "rr/overlay662_garage.h"
#include "rr/frame_storage_types.h"
#include "rr/race_types.h"
#include "rr/badge_row_types.h"
#include "rr/projection_state.h"
s16 ModeGroup, ModeId, D_800F4D8A, D_800AD50A;
s32 ModeFrame; RrRaceVehicle PlayerCars[2];
RrFrameStorage *CurrentDrawBuf, display_buffer;
RrBadgeRow1C first, second, *BadgeRows;
s16 *sample665_badges;
u8 ScreenFadeState;
RrGarageWorkingStorage ovl662_D_80127668;
s32 ovl667_D_80118EE0;
u32 *ovl668_D_80116E54;
RrProjectionScratchPrefix test_scratch;
#define RR_PROJECTION_SCRATCH (&test_scratch)
static int scenario, step;
void StopRaceAudioAndRumble(void) { assert(scenario == 666 && step++ == 0); }
void SetupRaceAndScene(void) {
    assert((scenario == 659 && step == 0) || (scenario == 666 && step == 1));
    step++;
    ModeGroup = -8; ModeId = -9; D_800F4D8A = -10;
    CurrentDrawBuf = &display_buffer;
}
void AddScreenFadeTiles(void *a, s32 b, s32 c) {
    assert(scenario == 666 && step++ == 2);
    assert(a == display_buffer.ordering.ordering70 && b == 255 && c == 69);
    assert(ModeGroup == 1 && ModeId == 82 && D_800F4D8A == 0);
}
void SetupRenderer(s32 a) {
    assert(scenario == 663 && step++ == 0 && a == 5 && ModeFrame == 0);
    memset(&test_scratch, 0xA5, sizeof(test_scratch));
}
void ResetCarState(void *p) {
    assert(scenario == 663 && step++ == 1 && p == PlayerCars);
    RrProjectionScratchPrefix expected;
    memset(&expected, 0xA5, sizeof(expected));
    expected.word18=120; expected.word08=-16; expected.word0C=-248;
    expected.word1C=0; expected.word20=0; expected.word10=-96;
    assert(!memcmp(&test_scratch,&expected,sizeof(expected)));
    memset(p, 0x5A, sizeof(PlayerCars));
}
void InitPreviewImageRows(void) {
    assert(scenario == 663 && step++ == 2);
    for (int i = 0; i < 0x320/4; i++) {
        s32 expected = 0x5A5A5A5A;
        if(i == 10) expected = 308;
        if(i == 9) expected = 10;
        if(i == 21) expected = 2560;
        s32 actual; memcpy(&actual,(u8 *)PlayerCars+i*4,4); assert(actual == expected);
    }
    BadgeRows = &second; sample665_badges=(s16 *)&second;
}
void ClearScreenOverlayState(void) {
    assert(scenario == 665 && step++ == 0);
    ScreenFadeState = 7;
}
void InitBadgeRow(s32 a,s32 b,s32 c,s32 d,s32 e,s32 f,s32 g) {
    assert(scenario == 665 && step++ == 1 && ScreenFadeState == 2);
    assert(a == 0 && b == 12 && c == 304 && d == 296 && e == 88 && f == 0 && g == 0);
    BadgeRows = &second; sample665_badges=(s16 *)&second;
}
void SetMenuFrameOptions(s32 a,s32 b,s32 c) {
    assert(scenario == 665 && step++ == 2);
    assert(a == 1 && b == 1 && c == 0 && second.half02 == 1);
}
''' + '\n'.join(bodies) + r'''
int main(void) {
    scenario = 659; step = 0;
    ovl659_exit_race();
    assert(step == 1 && ModeGroup == 1 && ModeId == 47 && D_800F4D8A == 0);
    ovl660_func_80114910(); assert(D_800AD50A == 0x131);
    ovl661_func_80117340(); assert(D_800AD50A == 0x137);
    for (unsigned i = 0; i < 65536; i++) {
        memset(&ovl662_D_80127668, 0xA5, sizeof(ovl662_D_80127668));
        s32 args[4] = {(s32)(0x12340000U|i), (s32)(0xFFFF0000U|i),
                       (s32)(0x80000000U|(i^0xFFFF)), (s32)(0x7FFF0000U|i)};
        ovl662_func_8011E7C0(args[0],args[1],args[2],args[3]);
        for (int j = 0; j < 32; j++) {
            u16 expected = j >= 24 && j < 28 ? (u16)args[j-24] : 0xA5A5;
            assert((u16)ovl662_D_80127668.halves[j] == expected);
        }
        u32 bits = (i << 16) | (i ^ 0xFFFF);
        ovl667_D_80118EE0 = (s32)bits;
        assert((u32)ovl667_func_80115048() == bits);
        u32 values[3] = {bits, ~bits, i};
        for (int j = 0; j < 3; j++) {
            ovl668_D_80116E54 = &values[j];
            assert(ovl668_func_8011688C() == values[j]);
        }
    }
    for (int module = 663; module <= 665; module += 2) {
        scenario = module; step = 0;
        memset(&first, 0xA5, sizeof(first)); memset(&second, 0x5A, sizeof(second));
        BadgeRows = &first; sample665_badges=(s16 *)&first; ModeFrame = 99;
        if (module == 663) ovl663_init_result_preview(); else ovl665_func_80115C8C();
        assert(step == 3);
        for (unsigned j = 0; j < sizeof(first); j++) {
            assert(((u8 *)&first)[j] == 0xA5);
            assert(((u8 *)&second)[j] == (j == 2 ? 1 : j == 3 ? 0 : 0x5A));
        }
    }
    scenario = 666; step = 0;
    ovl666_exit_race(); assert(step == 3);
    return 0;
}
'''
        with TemporaryDirectory() as temporary:
            directory = Path(temporary)
            source = directory / 'samples.c'
            executable = directory / 'samples'
            source.write_text(harness)
            result = subprocess.run(['cc', '-std=c99', '-O2', '-Wall', '-Wextra',
                                     '-Werror','-Wno-error=deprecated-non-prototype', '-fsanitize=undefined',
                                     '-I', str(ROOT / 'include'), str(source),
                                     '-o', str(executable)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stderr, '')


if __name__ == '__main__':
    unittest.main()
