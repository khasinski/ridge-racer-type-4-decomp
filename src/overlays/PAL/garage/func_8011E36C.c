#include "rr/overlay662_action_loop.h"

void garage_func_8011E36C(s32 mode)
{
    s32 i;
    s16 selected;
    RrGarageActionRecord16 *a;
    RrGarageButton *p4, *p9, *p11, *p12, *p14;

    if (mode) {
        for (i = 0; i < 53; i++) {
            a = &garage_D_80126C38[i];
            garage_D_80127470[a->key] = &a->prefix;
        }
        for (i = 0; i < 46; i++) {
            garage_D_801273B8[garage_D_80126F88[i].key0C] = &garage_D_80126F88[i].rectangle;
        }
    } else {
        if (!(garage_D_80127668.words[0] & 2)) {
            if (!((garage_D_80127668.words[0] & 4) &&
                  (s32)garage_D_80127668.words[10] < 4096)) {
                if (garage_D_80127668.words[0] & 8) {
                    garage_D_80127470[garage_D_80127668.words[4]]->callback(0);
                } else {
                    for (i = 0; i < 53; i++) {
                        garage_D_80127470[i]->callback(0);
                        if (garage_D_80127668.words[0] & 8) {
                            break;
                        }
                    }
                }
            }
        }
        switch (garage_D_801276A8.config1E) {
        case 0: selected = 9; break;
        case 1: selected = 11; break;
        case 2: selected = 12; break;
        case 3: selected = 14; break;
        case 4: selected = 4; break;
        }
        p4 = (RrGarageButton *)garage_D_801273B8[4];
        p9 = (RrGarageButton *)garage_D_801273B8[9];
        p11 = (RrGarageButton *)garage_D_801273B8[11];
        p12 = (RrGarageButton *)garage_D_801273B8[12];
        p14 = (RrGarageButton *)garage_D_801273B8[14];
        p4->timer0E = 0;
        p14->timer0E = 0;
        p12->timer0E = 0;
        p11->timer0E = 0;
        p9->timer0E = 0;
        ((RrGarageButton *)garage_D_801273B8[selected])->timer0E = -1;
        if (!garage_D_801276A8.flagsD1) {
            for (i = 0; i < 53; i++) {
                /* Recover the complete owner from its initial prefix member. */
                a = (RrGarageActionRecord16 *)garage_D_80127470[i];
                a->render(a->argument);
            }
            garage_func_8011BF04();
        }
    }
}
