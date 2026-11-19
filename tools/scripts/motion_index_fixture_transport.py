"""Bind the old presentation fixture's scalar index to the full audio owner.

The World/model/cases stay literal. Only the entry's storage binding changes;
the actual RestartMotionSequence definition is embedded without token edits.
"""
from c_functions import definition_source


def transport(source):
    name='RestartMotionSequence'
    body=definition_source(source,name).rstrip()
    assert source.count(body)==1
    support='''
static struct {u32 before[4];AudioState value;u32 after[4];} rr_fixture_sound,rr_fixture_sound_expected;
#define SoundState rr_fixture_sound.value
#define RestartMotionSequence rr_fixture_motion_restart
'''
    wrapper='''
#undef RestartMotionSequence
#undef SoundState
void RestartMotionSequence(void){
    memset(&rr_fixture_sound,0xA5,sizeof(rr_fixture_sound));
    rr_fixture_sound.value.seqTrack2868=MotionSequenceIndex;
    memcpy(&rr_fixture_sound_expected,&rr_fixture_sound,sizeof(rr_fixture_sound));
    assert((uintptr_t)&rr_fixture_sound.value>UINT32_MAX);
    rr_fixture_motion_restart();
    assert(!memcmp(&rr_fixture_sound,&rr_fixture_sound_expected,sizeof(rr_fixture_sound)));
}
'''
    result='#include "rr/audio_state_types.h"\n'+source.replace(body,support+body+wrapper)
    assert result.count(body)==1
    return result
