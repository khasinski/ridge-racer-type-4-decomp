#ifndef RR_GAME_DEMO_INTRO_H
#define RR_GAME_DEMO_INTRO_H

#include "rr/demo_fade_types.h"

extern DemoFadeCommand8 IntroTimeline[];

int IntroScriptStep(void);
void IntroInit(void);

#endif /* RR_GAME_DEMO_INTRO_H */
