#include "rr/garage_garage.h"
#include "rr/trig.h"
void garage_update_selection_pulse(void)
{
    garage_logo_state.shadeD0=rsin((s32)garage_logo_state.phaseCC)/64-65;
    garage_logo_state.phaseCC+=96U;
    garage_logo_state.phaseCC=(s32)garage_logo_state.phaseCC%4096;
}
