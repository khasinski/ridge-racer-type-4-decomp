#include "rr/overlay662_input.h"

void garage_func_8011D730(u32 *output)
{
    u32 result=0;
    s32 axis;
    if (PlayerControllers[0].type01 == 0x73 && PlayerControllers[0].kind1C != 0 && PlayerControllers[0].flag40 == 0) {
        axis=PlayerControllers[0].axis17;
        if (axis < 32) {
            garage_D_80126C18++;
            if ((s32)garage_D_80126C18 >= 47) garage_D_80126C18=40;
            if (garage_D_80126C18 == 1 || garage_D_80126C18 == 40) result |= 1;
            *output=result;
        } else {
            garage_D_80126C18=0;
        }
        if (axis >= 225) {
            garage_D_80126C1C++;
            if ((s32)garage_D_80126C1C >= 47) garage_D_80126C1C=40;
            if (garage_D_80126C1C == 1 || garage_D_80126C1C == 40) result |= 2;
            *output=result;
        } else {
            garage_D_80126C1C=0;
        }
        axis=PlayerControllers[0].axis16;
        if (axis < 32) {
            garage_D_80126C20++;
            if ((s32)garage_D_80126C20 >= 47) garage_D_80126C20=40;
            if (garage_D_80126C20 == 1 || garage_D_80126C20 == 40) result |= 4;
            *output=result;
        } else {
            garage_D_80126C20=0;
        }
        if (axis >= 225) {
            garage_D_80126C24++;
            if ((s32)garage_D_80126C24 >= 47) garage_D_80126C24=40;
            if (garage_D_80126C24 == 1 || garage_D_80126C24 == 40) result |= 8;
            *output=result;
        } else {
            garage_D_80126C24=0;
        }
    }
}
