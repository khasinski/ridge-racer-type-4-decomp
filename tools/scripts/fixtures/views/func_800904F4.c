/* GCC_VERSION: 2.7.2 */

#include "rr/trig.h"

s32 rcos(s32 angle) {
    s32 a=angle;
    s32 result;
    if(a<0) a=(s32)(0U-(u32)a);
    a &= 0xFFF;
    if(a<0x801) {
        if(a<0x401) {result=SinTable[0x400-a];goto out;}
        result=SinTableBiasedBase400[a];result=-result;goto out;
    }
    if(a<0xC01) goto negative;
    result=SinTableBiasedBaseC00[a];goto out;
negative:
    result=-SinTable[0xC00-a];
out:
    return result;
}
