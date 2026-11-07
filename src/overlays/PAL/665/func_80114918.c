#include "rr/overlay665_visual.h"
#ifndef RR_PROJECTION_SCRATCH
#define RR_PROJECTION_SCRATCH ((RrProjectionScratchPrefix *)0x1F800000)
#endif
void ovl665_func_80114918(void)
{
    RrOverlay665VisualState *state;
    RrProjectionScratchPrefix *scratch;
    SetDispMask(0);
    SetupDrawDispEnvs(0,0,0,2);
    state=&ovl665_D_80119768;
    PsyqBzero(state,160);
    state->lighting4C.matrix.signedHalves[6]=4500;
    state->lighting4C.matrix.signedHalves[7]=-4096;
    state->lighting4C.matrix.signedHalves[8]=-8000;
    state->lighting4C.matrix.bytes[0]=192;state->lighting4C.matrix.bytes[1]=192;state->lighting4C.matrix.bytes[2]=192;
    state->lighting4C.matrix.signedHalves[10]=8000;state->lighting4C.matrix.signedHalves[11]=8000;state->lighting4C.matrix.signedHalves[12]=-8000;
    state->lighting4C.matrix.bytes[4]=128;state->lighting4C.matrix.bytes[5]=128;state->lighting4C.matrix.bytes[6]=128;
    state->lighting4C.matrix.signedHalves[14]=-2637;state->lighting4C.matrix.signedHalves[15]=1602;state->lighting4C.matrix.signedHalves[16]=2694;
    state->lighting4C.matrix.bytes[8]=0;state->lighting4C.matrix.bytes[9]=0;state->lighting4C.matrix.bytes[10]=0;
    state->lighting4C.ambient[0]=64;state->lighting4C.ambient[1]=64;state->lighting4C.ambient[2]=64;
    scratch=RR_PROJECTION_SCRATCH;
    scratch->word08=0;scratch->word0C=0;scratch->word10=0;
    scratch->word18=0;scratch->word1C=0;scratch->word20=0;
    ResetScreenCounters();
    DialogueContext=2;ModeGroup=2;ModeId=2;ModeSubState=0;
}
