"""Actual overlay standings source: stable minimum, sentinel, finish writes and guards."""
from overlay_units import read_c_source
from pathlib import Path
import re
import subprocess
from tempfile import TemporaryDirectory
import unittest

ROOT = Path(__file__).resolve().parents[2]


class Overlay659StandingsTest(unittest.TestCase):
    def test_standings_with_native_ubsan(self):
        body = read_c_source(ROOT / 'src/overlays/PAL/race_results/func_801158B4.c')
        body = re.sub(r'^#include[^\n]*\n', '', body, flags=re.M)
        body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
        source = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
/* Native stand-in for resident vehicle storage, with its real size/alignment. */
#include "rr/race_types.h"
RrRaceVehicle PlayerCars[2];
s32 LapTimes[10], TotalRaceTime;
u16 BestLapIndex;
''' + body + r'''
static u32 random_state = 0x3715A6B9U;
static u32 next(void) {
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}
static void check(const s32 input[6], s32 rank, s16 count) {
    s32 expected_times[10];
    u8 expected_vehicle[sizeof(PlayerCars)];
    memcpy(LapTimes, input, 6 * sizeof(*input));
    for (unsigned i = 6; i < 10; i++) LapTimes[i] = (s32)(0x5A000000U + i);
    memcpy(expected_times, LapTimes, sizeof(expected_times));
    memset(PlayerCars, 0xA5, sizeof(PlayerCars));
    memcpy((u8 *)PlayerCars + 0x2AA, &count, sizeof(count));
    memcpy(expected_vehicle, PlayerCars, sizeof(expected_vehicle));
    TotalRaceTime = rank; BestLapIndex = 0xBEEF;
    u16 selected = 0xBEEF;
    if (rank == -1) {
        s16 marker = -1;
        memcpy(expected_vehicle + 0x2A8, &marker, sizeof(marker));
        if (count > 0) expected_times[count-1] = -1;
    } else {
        selected = 0;
        /* Independent oracle: first minimum in the nonnegative prefix. */
        for (u16 i = 1; i < 6 && input[i] >= 0 && input[0] >= 0; i++)
            if (input[i] < input[selected]) selected = i;
    }
    race_results_rank_standings();
    assert(BestLapIndex == selected && TotalRaceTime == rank);
    assert(memcmp(expected_times, LapTimes, sizeof(expected_times)) == 0);
    assert(memcmp(expected_vehicle, PlayerCars, sizeof(expected_vehicle)) == 0);
}
int main(void) {
    s32 values[6];
    for (unsigned pattern = 0; pattern < 729; pattern++) {
        for (int prefix = 0; prefix <= 6; prefix++) {
            unsigned p = pattern;
            for (int i = 0; i < 6; i++) { values[i] = p % 3; p /= 3; }
            if (prefix < 6) values[prefix] = -1;
            check(values, 0, 6);
        }
    }
    for (int trial = 0; trial < 50000; trial++) {
        for (int i = 0; i < 6; i++) values[i] = (s32)(next() & 0x7FFFFFFFU);
        int prefix = next() % 7;
        if (prefix < 6) values[prefix] = (s32)(next() | 0x80000000U);
        check(values, (s32)(next() & 0x7FFFFFFFU), 6);
        check(values, -1, (s16)((int)(next() % 11) - 4));
    }
    return 0;
}
'''
        with TemporaryDirectory() as temporary:
            directory = Path(temporary)
            path = directory / 'standings.c'
            executable = directory / 'standings'
            path.write_text(source)
            result = subprocess.run(['cc', '-I'+str(ROOT/'include'), '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror','-Wno-error=deprecated-non-prototype',
                                     '-fno-strict-aliasing', '-fsanitize=undefined',
                                     str(path), '-o', str(executable)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stderr, '')


if __name__ == '__main__':
    unittest.main()
