#include "rr/overlay662_garage.h"
#include "rr/trig.h"
void garage_func_8011D208(void)
{
    garage_D_801276A8.shadeD0=rsin((s32)garage_D_801276A8.phaseCC)/64-65;
    garage_D_801276A8.phaseCC+=96U;
    garage_D_801276A8.phaseCC=(s32)garage_D_801276A8.phaseCC%4096;
}
